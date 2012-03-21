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
#include <stddef.h>

// Time required to be considered a hold in ms
#define BUTTON_HOLD_TIME 500
// Processor will go to sleep in 15 sec if no activity on the clock screen
#define TIME_BEFORE_SLEEP 10000

// Screen dimensions
#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 128

// Game Constants
#define BALL_SIZE 4
#define GAP_WIDTH 16
#define NUM_WALLS 3

// This is a text box widget that will define the bounds of where to render the text
struct PWTextBox text_box;
// Text widget that stores required information about rendering the text
// It must be initialized before it can be used
struct PWidgetTextDynamic text_widget;
// Text buffer to store text to be rendered
char text_buffer[16];
// Timer used to put processor to sleep
int32_t sleep_timer_id;
// Timer used to determine button holds
int32_t button_timer_id;
// Current system minute
uint8_t current_min;
// Current system time
struct pulse_time_tm current_time;

// Types of screens
typedef enum Screen
{
    SCREEN_CLOCK,
    SCREEN_GAMEPLAY,
    SCREEN_GAME_OVER
} Screen;

// Current screen being displayed
Screen current_screen;

int hold_timer_value;
uint32_t time_last_box_drawn_ms = 0;
uint32_t time_now_ms;

int start_y = 0;
int score;
int highscore = 0;
int ball_x;
int ball_y;
int wall_y [NUM_WALLS];
int wall_gap_x [NUM_WALLS];
int game_speed;
bool press;
bool button_held = false;

color24_t green = { 0x44, 0x99, 0x11, 0x00 };
color24_t teal = { 0x00, 0xCC, 0xAA, 0x00 };
color24_t blue = { 0x00, 0x40, 0xFF, 0x00 };
color24_t purple = { 0x70, 0x00, 0x80, 0x00 };
color24_t pink = { 0xFF, 0x66, 0xFF, 0x00 };
color24_t orange = { 0xFF, 0x80, 0x00, 0x00 };
color24_t red = { 0xFF, 0x00, 0x00, 0x00 };

color24_t wall_color = { 0x44, 0x99, 0x11, 0x00 };
color24_t clock_color = { 0x44, 0x99, 0x11, 0x00 };
color24_t date_color = { 0x60, 0x60, 0x60, 0x00 };
	        
void reset_game();
void long_press();
void return_to_clock_screen();

void main_app_init()
{  
    pulse_get_time_date(&current_time);
	return_to_clock_screen();

    // Callback to handle button waking up processor
    pulse_register_callback(ACTION_WOKE_FROM_BUTTON, &return_to_clock_screen);
}

void main_app_handle_button_down()
{
	if (current_screen == SCREEN_GAME_OVER)// || current_screen == SCREEN_CLOCK)
		button_timer_id = pulse_register_timer(BUTTON_HOLD_TIME, &long_press, 0);
	else if (current_screen == SCREEN_GAMEPLAY)
		press = true;
}

void long_press() 
{
	button_held = true;

	/*if (current_screen == SCREEN_CLOCK)
		pulse_update_power_down_timer(0);	
	else*/
	if (current_screen == SCREEN_GAME_OVER)
		return_to_clock_screen();
}

void main_app_handle_button_up()
{
	if (!button_held)
	{
		pulse_cancel_timer(&button_timer_id);
		
		if (current_screen == SCREEN_CLOCK)
		{
			cancel_sleep();
			pulse_blank_canvas();
			reset_game();
			current_screen = SCREEN_GAME_OVER;
			printf("\n\n\n\n\n Click to start");
			prepare_to_sleep();				
		}
		else if (current_screen == SCREEN_GAME_OVER)
		{
			time_now_ms = pulse_get_millis();
		    // Check if it's been at least 500 ms since the game ended to prevent accidental clicks
		    if (time_now_ms - time_last_box_drawn_ms > 500) 
		    {
				cancel_sleep();
				pulse_blank_canvas();
				current_screen = SCREEN_GAMEPLAY;
			}
		}
		else if (current_screen == SCREEN_GAMEPLAY)
			press = false;
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
    // Initialize the text box parameters
    text_box.top = 0;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT-26;

    // Configure the text style
    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER | PWTS_VERTICAL_CENTER);
    // Initialize the text widget
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_26, clock_color, style);
    // Print the time from the RTC (real time clock) in to the text buffer
    print_time_into_text_buffer();

    // Render the text in the text box area
    pulse_render_text(&text_box, &text_widget);
}


// Render clock text and draw to screen
void render_date(color24_t clock_color)
{
    // Initialize the text box parameters
    text_box.top = SCREEN_HEIGHT/2+4;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT;

    // Configure the text style
    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER );

    // Initialize the text widget
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_10, date_color, style);

    // Print the time from the RTC (real time clock) in to the text buffer
    print_date_into_text_buffer();

    // Render the text in the text box area
    pulse_render_text(&text_box, &text_widget);
}

void return_to_clock_screen()
{
    pulse_blank_canvas();
    current_screen = SCREEN_CLOCK;
    render_time(clock_color);
    render_date(clock_color);

    prepare_to_sleep();
}

// Will put processor to sleep in predetermined number of ms
void prepare_to_sleep()
{
    pulse_cancel_timer(&sleep_timer_id);
	sleep_timer_id = pulse_register_timer(TIME_BEFORE_SLEEP, &pulse_update_power_down_timer, 0);
}

// Cancel call to put processor to sleep
void cancel_sleep()
{
    pulse_cancel_timer(&sleep_timer_id);
}


void reset_game()
{
	score = 0;
	ball_x = 46;
	ball_y = 80;
	game_speed = 20;
	press = false;
	wall_color = green;
	
	for (int i = 0; i < NUM_WALLS; i++)
	{
		wall_y[i] = (SCREEN_HEIGHT-13 + i*SCREEN_HEIGHT/3) % (SCREEN_HEIGHT - 1);
		wall_gap_x[i] = (SCREEN_WIDTH-GAP_WIDTH)/2;
	}
	draw_game_elements();
}

void draw_box(int x,int y,int width, int height, color24_t color)
{
    // Set the canvas dimensions to be a 4 x 4 box
    pulse_set_draw_window(x, y, x + width - 1, y + height - 1);

    // Draw pixels onto that canvas
    for (int i = 0; i < width * height; i++) 
        pulse_draw_point24(color);
}

void draw_ball(int x,int y, color24_t color)
{
	draw_box(x,y,BALL_SIZE,BALL_SIZE,color);
}

void draw_wall(int y,int gap_x, color24_t color)
{
	draw_box(0,y,gap_x,1,color);
	draw_box(gap_x + GAP_WIDTH,y,SCREEN_WIDTH - gap_x - GAP_WIDTH,1,color);
}

void draw_game_elements()
{
	draw_ball(ball_x,ball_y, green);
	for (int i = 0; i < NUM_WALLS; i++)
		draw_wall(wall_y[i],wall_gap_x[i], wall_color);
}

void move_wall(int i){
	// Incrememnt the y coordinate and wrap around if we're hitting the top of the screen
	wall_y[i] = (wall_y[i] - 1) % (SCREEN_HEIGHT -1 - 1);
	if (wall_y[i] <= start_y){
		wall_y[i] = SCREEN_HEIGHT-2;
		wall_gap_x[i] = rand()%(SCREEN_WIDTH-GAP_WIDTH);
	}
}

void game_over()
{
	clock_color = wall_color;
	pulse_vibe_on();
	pulse_mdelay(70);
	pulse_vibe_off();
	//int vib_timer = pulse_register_timer(75, &pulse_vibe_off, 0);

	current_screen = SCREEN_GAME_OVER;
	pulse_blank_canvas();
	if (score > highscore)
		highscore = score;
	printf ("\n   GAME OVER\n\n\n\nScore: %d\n\nHighscore: %d",score,highscore);

	reset_game();
	prepare_to_sleep();
}

// Main loop. 
void main_app_loop()
{
    if (current_screen == SCREEN_GAMEPLAY)
	{
	    // Find out how long the processor has been active (since boot or since sleep)
	    time_now_ms = pulse_get_millis();
	
	    // Check if it's been at least 10 ms since the box was last drawn
	    if (time_now_ms - time_last_box_drawn_ms >= game_speed) 
	    {	
		    score++;
		    
		    // Change the color and speed up the walls when certain scores are reached
		    switch(score) 
			{
			    case 800:	
					wall_color = teal;
					game_speed-=2;
				    break;
			    case 1600:	
					wall_color = blue;
					game_speed-=2;
				    break;
				case 2400:	
					wall_color = purple;
					game_speed-=2;
				    break;
				case 3200:	
					wall_color = pink;
					game_speed-=2;
				    break;
				case 4000:	
					wall_color = orange;
					game_speed-=2;
				    break; 
				case 4800:	
					wall_color = red;
					game_speed-=2;
				    break;            
			} 
		    
	        // Erase the old ball and walls by drawing over them
	        draw_ball(ball_x,ball_y, COLOR_BLACK24);
			for (int i = 0; i < NUM_WALLS; i++)
				draw_wall(wall_y[i],wall_gap_x[i], COLOR_BLACK24);
	
			// Move the ball based on user input
			if(press && ball_x < SCREEN_WIDTH - BALL_SIZE)
				ball_x++;
			else if (!press && ball_x > 0)
				ball_x--;
	        
	        // Move the walls up
			for (int i = 0; i < NUM_WALLS; i++)
				move_wall(i);
			
			// Check for collisions between the ball and the wall
			bool collision = false;
			for (int i = 0; i < NUM_WALLS; i++)
			{
				if ((ball_y == wall_y[i] - BALL_SIZE + 1 || ball_y == wall_y[i] - BALL_SIZE)&& (ball_x < wall_gap_x[i] || ball_x >  wall_gap_x[i] + GAP_WIDTH - BALL_SIZE))
				{
					ball_y = wall_y[i] - BALL_SIZE;
					collision = true;
				}
			}

			// check if ball reached the top 
			if (ball_y == start_y)
					game_over();
			else 
			{
				if (!collision && ball_y < SCREEN_HEIGHT-BALL_SIZE-1)
					ball_y = (ball_y + 1);
				
		        // Draw the new box and walls
				draw_game_elements();
	
				// update the time_last_box_draw_ms variable
		        time_last_box_drawn_ms = time_now_ms;
			}
	    }
	}
	else if (current_screen == SCREEN_CLOCK)
		pulse_get_time_date(&current_time);
}

// This function is called whenever the processor is about to sleep (to conserve power)
// The sleep functionality is scheduled with pulse_update_power_down_timer(uint32_t)
void main_app_handle_doz()
{

}

void main_app_handle_hardware_update(enum PulseHardwareEvent event)
{

}
