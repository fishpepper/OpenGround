/*
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
#include "console.h"
#include "screen.h"
#include "font.h"
#include "delay.h"

static uint8_t console_buffer[CONSOLE_BUFFER_SIZE_Y][CONSOLE_BUFFER_SIZE_X+1];
static uint8_t console_write_x;
static uint8_t console_write_y;

void console_init(void) {
    uint32_t i;
    
    //initialise console
    console_write_x = 0;
    console_write_y = 0;
    for(i=0; i<CONSOLE_BUFFER_SIZE_Y; i++){
        console_buffer[i][0] = 0;
        console_buffer[i][CONSOLE_BUFFER_SIZE_X] = 0;
    }

    //testing:
    uint8_t c = 'A';
    while(1){
        console_putc(c++);
        console_render();
        if (c >= 'z') c = 'A';
        delay_us(100*1000);
    }
}


static void console_render_str(uint8_t line, uint8_t color, uint8_t *str){
    const uint8_t *font = font_system5x7;
    screen_set_font(font);
    
    //write string to screen at position x,y
    uint8_t height = font[FONT_HEIGHT];
    uint8_t y = (height+1) * line;
    
    //render to screen buffer
    screen_puts_xy(1, y, color, str);
}


void console_puts(uint8_t *str){
    while(*str){
        console_putc(*str);
    }
}

void console_putc(uint8_t c){
    //print one char to our screen, this function 
    //handles the screen layout etc
    
    if (c == '\r'){
        //reset x pointer
        console_write_x = 0;
        return;
    }
    
    if(c == '\n'){
        //newline, skip to next line
    }else{
        //output char:
        console_buffer[console_write_y][console_write_x] = c;
    }
    
    //keep track of lines
    console_write_x++;
    if (console_write_x >= (CONSOLE_BUFFER_SIZE_X)){
        //switch and clear next line
        console_write_y = (console_write_y + 1) % CONSOLE_BUFFER_SIZE_Y;
        console_buffer[console_write_y][0] = 0;
        console_write_x = 0;
    }
}

void console_render(void) {
    uint32_t i;
    uint8_t  color = CONSOLE_TEXTCOLOR;
    
    //fill screen with inverse of color
    screen_fill(1-color);
    
    //update screen memory to show the current line buffer
    //we want the current line to be at the bottom, calculate first line to render:
    uint8_t line_now = console_write_y;
    if (console_write_x == 0){
        //no chars on next line, use previous line!
        line_now--;
    }
    
    //calculate first line to print:
    uint8_t line = (line_now + 1) % CONSOLE_BUFFER_SIZE_Y;
    for(i=0; i<CONSOLE_BUFFER_SIZE_Y; i++){
        //print current line
        console_render_str(i, color, console_buffer[line]);
        //fetch next line id
        line = (line + 1) % CONSOLE_BUFFER_SIZE_Y;
    } 
    
    screen_update();
}


