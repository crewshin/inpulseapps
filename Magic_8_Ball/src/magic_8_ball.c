/*
Copyright (C) 2012 Allerta
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
	provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions 
	and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
	and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED 
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
	PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.
*/

#include <pulse_os.h>
#include <pulse_types.h>
#include <app_resources.h>
#include <stdint.h>

// Time required to be considered a hold in ms
#define BUTTON_HOLD_TIME 500
// Processor will go to sleep in 15 sec if no activity on the clock screen
#define TIME_BEFORE_SLEEP 10000

// Screen dimensions
#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 128

// Text box
struct PWTextBox text_box;
struct PWidgetTextDynamic text_widget;
char text_buffer[26];

int32_t sleep_timer_id;
int32_t button_timer_id;
uint8_t current_min;
struct pulse_time_tm current_time;

// Types of screens
typedef enum Screen{
    SCREEN_CLOCK,
    SCREEN_8_BALL
} Screen;

// Current screen being displayed
Screen current_screen;

int hold_timer_value;
uint32_t time_last_box_drawn_ms = 0;
uint32_t time_now_ms;

color24_t clock_color = { 0x17, 0x38, 0xA3, 0x00 };
color24_t date_color = { 0x60, 0x60, 0x60, 0x00 };
	        
void long_press();
void return_to_clock_screen();
bool button_held = false;



// This function is called once after the watch has booted up and the OS has loaded
void main_app_init()
{	    
    pulse_get_time_date(&current_time);
	return_to_clock_screen();

    // Callback to handle button waking up processor
    pulse_register_callback(ACTION_WOKE_FROM_BUTTON, &return_to_clock_screen);
}


void eight_ball_init()
{	    
    current_screen = SCREEN_8_BALL;
    
    // seed the random number function with the current time
    struct pulse_time_tm current_time;
    pulse_get_time_date(&current_time);
    uint32_t sec = current_time.tm_sec;
    srand(sec);
    
    // draw the 8 ball
    pulse_blank_canvas();
	pulse_draw_image(IMAGE_8BALL2,SCREEN_WIDTH/2-30,SCREEN_HEIGHT/2-30);
}


void print_answer_into_text_buffer()
{      
    int randNum = rand()%20;

	switch( randNum ) 
	{
	    case 0:	
		    sprintf(text_buffer,"It is certain");
		    break;
	    case 1: 
		    sprintf(text_buffer,"It is decidedly so");
		    break;
	    case 2: 
		    sprintf(text_buffer,"Without a doubt");
		    break;
	    case 3: 
		    sprintf(text_buffer,"Yes – definitely");
		    break;
	    case 4: 
		    sprintf(text_buffer,"You may rely on it");
		    break;
	    case 5: 
		    sprintf(text_buffer,"As I see it, yes");
		    break;    
	    case 6: 
		    sprintf(text_buffer,"Most likely");
		    break;
	    case 7: 
		    sprintf(text_buffer,"Outlook good");
		    break;
	    case 8: 
		    sprintf(text_buffer,"Signs point to yes");
		    break;
	    case 9: 
		    sprintf(text_buffer,"Yes");
		    break;
	    case 10: 
		    sprintf(text_buffer,"Reply hazy, try again");
		    break;
	    case 11: 
		    sprintf(text_buffer,"Ask again later");
		    break;
	    case 12: 
		    sprintf(text_buffer,"Better not tell you now");
		    break;
	    case 13: 
		    sprintf(text_buffer,"Cannot predict now");
		    break;
	    case 14: 
		    sprintf(text_buffer,"Concentrate and ask again");
		    break;
	    case 15: 
		    sprintf(text_buffer,"Don't count on it");    
		    break;
	    case 16: 
		    sprintf(text_buffer,"My reply is no");
		    break;
	    case 17: 
		    sprintf(text_buffer,"My sources say no");
		    break;
	    case 18: 
		    sprintf(text_buffer,"Outlook not so good");
		    break;
		case 19: 
			sprintf(text_buffer,"Don't count on it"); 
			break;
		default: 
			sprintf(text_buffer,"error");
	} 
}

void print_answer()
{
	// Initialize the text box parameters
    text_box.top = 30;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = 60;//SCREEN_HEIGHT;

	enum PWTextStyle style = (PWTS_WRAP_AT_SPACE  | PWTS_CENTER);
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_CANDC_12, COLOR_WHITE24, style);
    
    //pulse_blank_canvas();
	pulse_draw_image(IMAGE_ANSWER,0,15);
	
    print_answer_into_text_buffer();
    pulse_render_text(&text_box, &text_widget);
    
    prepare_to_sleep();
}

// Draw 4 black rectangles around the new 8ball to ensure that the old 8ball is drawn over
void clear_old_8ball(int x, int y)
{
    //top
    pulse_set_draw_window(8, 24, 88, y);
    for (int i = 0; i < 80 * (y-24); i++) 
        pulse_draw_point24(COLOR_BLACK24);
    //left
    pulse_set_draw_window(8, y, x, y+60);
    for (int i = 0; i < (x-8) * (60); i++) 
        pulse_draw_point24(COLOR_BLACK24);
    //right
    pulse_set_draw_window(x+60, y, 88, y+60);
    for (int i = 0; i < (28-x) * (60); i++) 
        pulse_draw_point24(COLOR_BLACK24);
	//bottom
    pulse_set_draw_window(8, y+60, 88, 104);
    for (int i = 0; i < 80 * (44-y); i++) 
        pulse_draw_point24(COLOR_BLACK24);
}


void main_app_handle_button_down()
{  
	cancel_sleep();
	if ( current_screen == SCREEN_8_BALL) //||current_screen == SCREEN_CLOCK)
		button_timer_id = pulse_register_timer(BUTTON_HOLD_TIME, &long_press, 0);
}

void long_press() 
{
	button_held = true;

	/*if (current_screen == SCREEN_CLOCK)
		pulse_update_power_down_timer(0);	
	else */
	
	if (current_screen == SCREEN_8_BALL)
		return_to_clock_screen();
}

void main_app_handle_button_up()
{
	if (!button_held)
	{
		pulse_cancel_timer(&button_timer_id);
	
		if (current_screen == SCREEN_CLOCK)
			eight_ball_init();
		else if (current_screen == SCREEN_8_BALL)
		{
			pulse_blank_canvas();
		
			int x;
			int y;
			int count = 0;
			    
			// Animation of the 8 ball shaking
			while (count < 20)
			{	
				x = SCREEN_WIDTH/2-40+rand()%20;
				y = SCREEN_HEIGHT/2-40+rand()%20;
				
				clear_old_8ball(x,y);
				pulse_draw_image(IMAGE_8BALL2,x,y);
			
				count++;
				pulse_mdelay(20);
			}
			print_answer();
		}
	}
	else
		button_held = false;
}

void print_time_into_text_buffer()
{
    uint32_t hour = current_time.tm_hour;
    if(hour > 12)
        hour -= 12;
    else if(hour == 0)
        hour = 12;
    sprintf(text_buffer, "%d:%02d", hour, current_time.tm_min);
}

void print_date_into_text_buffer()
{
    uint32_t month = current_time.tm_mon;
    char *month_str;
    switch(month) 
	{
	    case 0:	
			month_str = "Jan.";
		    break;
		case 1:	
			month_str = "Feb.";
		    break;
		case 2:	
			month_str = "Mar.";
		    break;
		case 3:	
			month_str = "Apr.";
		    break;
		case 4:	
			month_str = "May";
		    break;
		case 5:	
			month_str = "June";
		    break;
		case 6:	
			month_str = "July";
		    break;
		case 7:	
			month_str = "Aug.";
		    break;
		case 8:	
			month_str = "Sep.";
		    break;
		case 9:	
			month_str = "Oct.";
		    break;
		case 10:	
			month_str = "Nov.";
		    break;
		case 11:	
			month_str = "Dec.";
		    break;
	} 
    sprintf(text_buffer, "%s %d, %ld", month_str, current_time.tm_mday, current_time.tm_year+1900);
}


// Render clock text and draw to screen
void render_time(color24_t clock_color)
{
    text_box.top = 0;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT-26;

    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER | PWTS_VERTICAL_CENTER);
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_26, clock_color, style);
    print_time_into_text_buffer();
    pulse_render_text(&text_box, &text_widget);
}


// Render clock text and draw to screen
void render_date(color24_t clock_color)
{
    text_box.top = SCREEN_HEIGHT/2+4;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT;

    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER );
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_10, date_color, style);
    print_date_into_text_buffer();
    pulse_render_text(&text_box, &text_widget);
}

void return_to_clock_screen()
{
    pulse_get_time_date(&current_time);
	pulse_blank_canvas();
    current_screen = SCREEN_CLOCK;
    render_time(clock_color);
    render_date(clock_color);

    prepare_to_sleep();
}

// Will put processor to sleep in predetermined number of ms
void prepare_to_sleep()
{
	sleep_timer_id = pulse_register_timer(TIME_BEFORE_SLEEP, &pulse_update_power_down_timer, 0);
}

// Cancel call to put processor to sleep
void cancel_sleep()
{
    pulse_cancel_timer(&sleep_timer_id);
}


// Main loop. This function is called frequently.
void main_app_loop()
{
}

// This function is called whenever the processor is about to sleep (to conserve power)
void main_app_handle_doz()
{
}

void main_app_handle_hardware_update(enum PulseHardwareEvent event)
{
}
