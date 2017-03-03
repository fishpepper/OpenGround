/*
    Copyright 2016 fishpepper <AT> gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    author: fishpepper <AT> gmail.com
*/

#include "telemetry.h"
#include "debug.h"
#include "fifo.h"

// telemetry fifo size, has to be a power of 2 !
#define TELEMETRY_BUFFER_LENGTH 64
static volatile uint8_t telemetry_buffer[TELEMETRY_BUFFER_LENGTH];
fifo_buffer_t telemetry_fifo_buffer;

static telemetry_state_t telemetry_state;
static uint8_t telemetry_data_id;
static uint8_t telemetry_high_byte;
static uint8_t telemetry_low_byte;
static uint16_t telemetry_last_value;
static uint8_t  telemetry_last_id;

static uint16_t telemetry_decoded_data_voltage;
static uint16_t telemetry_decoded_data_current;
static uint16_t telemetry_decoded_data_mah;

// internal functions
static void telemetry_parse_stream(uint8_t byte);
static void telemetry_process_hub_packet(uint8_t id, uint16_t value);

void telemetry_init(void) {
    debug("telemetry: init\n"); debug_flush();

    telemetry_state = TELEMETRY_IDLE;
    telemetry_decoded_data_voltage = 0;
    telemetry_decoded_data_current = 0;
    telemetry_last_value = 0;
    telemetry_data_id = 0;
    telemetry_last_id = 0;
    telemetry_high_byte = 0;
    telemetry_low_byte = 0;

    // init isr safe fifo
    fifo_init(&telemetry_fifo_buffer, telemetry_buffer, TELEMETRY_BUFFER_LENGTH);
}

void telemetry_enqueue(uint8_t byte) {
    // debug("telemetry: enq 0x"); debug_put_hex8(byte); debug_put_newline(); debug_flush();
    // insert into fifo
    if (!fifo_put(&telemetry_fifo_buffer, byte)) {
        // debug("telemetry: fifo full\n");
    }
}

void telemetry_process(void) {
    // handle telemetry packets
    if (!fifo_empty(&telemetry_fifo_buffer)) {
        uint8_t byte = fifo_get(&telemetry_fifo_buffer);
        // process incoming telemetry
        telemetry_parse_stream(byte);
    }
}

static void telemetry_parse_stream(uint8_t byte) {
    if (telemetry_state == TELEMETRY_DATA_END) {
        if (byte == 0x5e) {
            telemetry_process_hub_packet(telemetry_data_id,
                                         (telemetry_high_byte << 8) + telemetry_low_byte);
            telemetry_state = TELEMETRY_DATA_ID;
        } else {
            telemetry_state = TELEMETRY_IDLE;
        }
        return;
    }
    if (byte == 0x5e) {
        telemetry_state = TELEMETRY_DATA_ID;
        return;
    }
    if (telemetry_state == TELEMETRY_IDLE) {
        return;
    }
    if (telemetry_state & TELEMETRY_XOR) {
        byte = byte ^ 0x60;
        telemetry_state = (telemetry_state_t)(telemetry_state - TELEMETRY_XOR);
    } else if (byte == 0x5d) {
        telemetry_state = (telemetry_state_t)(telemetry_state | TELEMETRY_XOR);
        return;
    }
    if (telemetry_state == TELEMETRY_DATA_ID) {
        if (byte > 0x3f) {
            telemetry_state = TELEMETRY_IDLE;
        } else {
            telemetry_data_id = byte;
            telemetry_state = TELEMETRY_DATA_LOW;
        }
        return;
    }
    if (telemetry_state == TELEMETRY_DATA_LOW) {
        telemetry_low_byte = byte;
        telemetry_state = TELEMETRY_DATA_HIGH;
        return;
    }
    if (telemetry_state == TELEMETRY_DATA_HIGH) {
        telemetry_high_byte = byte;
        telemetry_state = TELEMETRY_DATA_END;
    }
}

static void telemetry_process_hub_packet(uint8_t id, uint16_t value) {
    // process hub data
    switch (id) {
        // defined in protocol_sensor_hub.pdf
        default:
        case 0x01:  // GPS_ALT (whole number & sign) -500m-9000m (.01m/count)
        case 0x09:  // GPS_ALT (fraction)  TODO opentx throws away the ap value and only uses bp
        case 0x02:  // TEMP1 -30C-250C (1C/ count)
        case 0x03:  // RPM   0-60000
        case 0x05:  // TEMP2 -30C-250C (1C/ count)
        case 0x06:  // Battery voltages - CELL# and VOLT
        case 0x10:  // ALT (whole number & sign) -500m-9000m (.01m/count)
        case 0x21:  // ALT (fraction)  (.01m/count)
        case 0x11:  // GPS Speed (whole number and sign) in Knots
        case 0x19:  // GPS Speed (fraction)
        case 0x12:  // GPS Longitude (whole number) dddmm.mmmm
        case 0x1A:  // GPS Longitude (fraction)
        case 0x22:  // GPS Longitude E/W
        case 0x13:  // GPS Latitude (whole number) ddmm.mmmm
        case 0x1B:  // GPS Latitude (fraction)
        case 0x23:  // GPS Latitude N/S
        case 0x14:  // GPS Compass (whole number) (0-259.99) (.01degree/count)
        case 0x1C:  // GPS Compass (fraction)
        case 0x15:  // GPS Date/Month
        case 0x16:  // GPS Year
        case 0x17:  // GPS Hour/Minute
        case 0x18:  // GPS Second
        case 0x24:  // Accel X
        case 0x25:  // Accel Y
        case 0x26:  // Accel Z
        case 0x30:  // VARIO
            // we will ignore this data
            break;

        case 0x04:  // Fuel  0, 25, 50, 75, 100
            // betaflight sends capacity in mah (default)
            telemetry_decoded_data_mah =  value;
            break;


        case 0x28:  // Current 0A-100A (0.1A/count)
            telemetry_decoded_data_current = value * 10;
            /*set_telemetry(TELEM_FRSKY_CURRENT, value);
            if (discharge_time == 0) discharge_time = CLOCK_getms();
            discharge_dAms += (u32)value * (CLOCK_getms() - discharge_time);
            discharge_time = CLOCK_getms();
            set_telemetry(TELEM_FRSKY_DISCHARGE, discharge_dAms / 36000);*/
            break;

        case 0x39:  // VFAS_ID
            telemetry_decoded_data_voltage = value * 10;
            break;

        case 0x3A:  // Ampere sensor voltage (whole number) (measured as V) 0V-48V (0.5V/count)
            telemetry_last_id    = id;
            telemetry_last_value = value;
            break;
        case 0x3B:  // Ampere sensor voltage (fractional part)
            if (telemetry_last_id == 0x3A) {
                telemetry_decoded_data_voltage =
                        ((telemetry_last_value * 100 + value * 10) * 210) / 110;
            }
            break;
    }
/*
    if (value >= VFAS_D_HIPREC_OFFSET) {
        // incoming value has a resolution of 0.01V and added offset of VFAS_D_HIPREC_OFFSET
        value -= VFAS_D_HIPREC_OFFSET;
    } else {
        // incoming value has a resolution of 0.1V
        value *= 10;
    }
    set_telemetry(TELEM_FRSKY_VOLT3, value);
            break;
*/
}


uint16_t telemetry_get_voltage(void) {
    return telemetry_decoded_data_voltage;
}

uint16_t telemetry_get_current(void) {
    return telemetry_decoded_data_current;
}

uint16_t telemetry_get_mah(void) {
    return telemetry_decoded_data_mah;
}
