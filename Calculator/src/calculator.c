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
#define TIME_BEFORE_SLEEP 15000

// Screen dimensions
#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 128

// Text box
struct PWTextBox text_box;
struct PWidgetTextDynamic text_widget;
char text_buffer[26];

int32_t sleep_timer_id;
int32_t button_timer_id;
int32_t long_button_timer_id;
struct pulse_time_tm current_time;

// Types of screens
typedef enum Screen{
    SCREEN_CLOCK,
    SCREEN_CALCULATOR,
    SCREEN_ANSWER
} Screen;

// Current screen being displayed
Screen current_screen;

color24_t green = { 0x44, 0x99, 0x11, 0x00 };
color24_t grey = { 0x60, 0x60, 0x60, 0x00 };
	        
void return_to_clock_screen();
bool button_held = false;


struct PWTextBox buttons_textbox;
char buttons_text_buffer[46];
struct PWidgetTextDynamic buttons_text_widget;

int selector_size = 18;
int selector_row = 0;
int selector_column = 0;

// coordinates for the selector that scrolls through the keys
int selector_x[4] = { 7, 30, 53, 74 };
int selector_y[4] = { 40, 61, 82, 103 };

char display_text[9];

char *button_values[17] = {"1","2","3","+","4","5","6","-","7","8","9","C","0","x","/","="};

int num_1;
int num_2;
int operation = 0;

//bool displaying_answer = false;

void initialize_buttons();
void initialize_text_box();
void draw_keypad();
void print_to_display();
void draw_box();
void draw_calculator();
void draw_selector();
void return_selector();
void long_press();
void very_long_press();
void pulse_vib();
void append_to_display(char c);



// This function is called once after the watch has booted up
// and the OS has loaded
void main_app_init()
{
    pulse_get_time_date(&current_time);
	return_to_clock_screen();

    // Callback to handle button waking up processor
    pulse_register_callback(ACTION_WOKE_FROM_BUTTON, &return_to_clock_screen);
}

void calc_init()
{
	current_screen = SCREEN_CALCULATOR;
	reset_calc();
	draw_calculator();
	//prepare_to_sleep();
}

void reset_calc()
{
	num_1 = 0;
	num_2 = 0;
	operation = 0;
	selector_column = 0;
	selector_row = 0;
	*display_text='\0';	
}

void main_app_handle_button_down()
{
	cancel_sleep();
	
	if (current_screen == SCREEN_ANSWER)
   		button_timer_id = pulse_register_timer(BUTTON_HOLD_TIME, &long_press, 0);
	else if (current_screen == SCREEN_CALCULATOR)
	{
		button_timer_id = pulse_register_timer(BUTTON_HOLD_TIME, &long_press, 0);
   		long_button_timer_id = pulse_register_timer(BUTTON_HOLD_TIME*4, &very_long_press, 0);
	}
}

void main_app_handle_button_up()
{
	pulse_cancel_timer(&long_button_timer_id);
	if(!button_held)
   	{
		pulse_cancel_timer(&button_timer_id);

	   	if (current_screen == SCREEN_CALCULATOR)
	   	{
			draw_selector(COLOR_BLACK24);
			selector_column = (selector_column + 1)%4;
			if (selector_column == 0)
				selector_row = (selector_row + 1)%4;
			draw_selector(green); 
		}
		else if (current_screen == SCREEN_ANSWER || current_screen == SCREEN_CLOCK)
		{
			calc_init();
		}
	}
	else
		button_held = false;
}

void long_press() 
{
	button_held = true;
	
	if (current_screen == SCREEN_CALCULATOR)
	{
		//print_to_display(button_values[selector_row][selector_column] );
		int len = strlen(display_text);
	
		int selected_index = selector_row*4+selector_column;
		switch(selected_index) 
		{
		    case 3:// +
				if (operation == 0)
				{
					operation = 1;
					operation_selected();
				}
				else
					pulse_vib();
			    break;
		    case 7:// -
				if (operation == 0)
				{
					operation = 2;
					operation_selected();
				}
				else
					pulse_vib();
			    break;
		    case 11:// C
			    reset_calc();
			    *display_text='\0';	
			    print_to_display();
			    return_selector();
			    break;
		    case 13:// x
				if (operation == 0)
				{
					operation = 3;
					operation_selected();
				}
				else
					pulse_vib();
			    break;
		    case 14:// /
				if (operation == 0)
				{
					operation = 4;
					operation_selected();
				}
				else
					pulse_vib();
			    break;
		    case 15:// =
				if (operation != 0)
				{
					int answer;
					num_2 = atoi(display_text);
					if (operation != 4)
					{
						if (operation == 1)
							answer = num_1 + num_2;
						else if (operation == 2)
							answer = num_1 - num_2;
						else if (operation == 3)
							answer = num_1 * num_2;
						
						if (answer > 99999999 || answer < -9999999)
							sprintf(display_text,"OVERFLOW");
						else
							sprintf(display_text, "%d", answer);
					}
					else
					{
						if (!num_2)
							sprintf(display_text,"ERROR");
						else
							sprintf(display_text, "%d", answer);
					}
					current_screen = SCREEN_ANSWER;
					print_to_display();
					draw_selector(COLOR_BLACK24);
					prepare_to_sleep();
				}
				else
					pulse_vib();
			    break;
			default:
				if (len < 8)
				{
					append_to_display(*button_values[selected_index]);  
					print_to_display();
					return_selector();
				}        
		} 
	}
	else if (current_screen == SCREEN_ANSWER)
		return_to_clock_screen();
}

void very_long_press() 
{
	return_to_clock_screen();
}

void operation_selected()
{
	num_1 = atoi(display_text);
	*display_text='\0';	
	print_to_display();
	return_selector();
}

void return_selector()
{
	draw_selector(COLOR_BLACK24);
	selector_column = 0;
	selector_row = 0;
	if (current_screen != SCREEN_ANSWER)
		draw_selector(green);
}

void pulse_vib()
{
	pulse_vibe_on();
	pulse_mdelay(75);
	pulse_vibe_off();
}
	

void draw_calculator()
{
    pulse_blank_canvas();
	draw_keypad();
	
	// display
	draw_box(3,8,SCREEN_WIDTH-6,24,grey);
}

void draw_keypad()
{
    //setting text box for the text
    buttons_textbox.top = 34;
    buttons_textbox.left = 0;
    buttons_textbox.right = SCREEN_WIDTH;
    buttons_textbox.bottom = SCREEN_HEIGHT-1;
    
    enum PWTextStyle buttons_style = (PWTS_WRAP_AT_SPACE | PWTS_VERTICAL_CENTER | PWTS_CENTER);
    pulse_init_dynamic_text_widget(&buttons_text_widget, buttons_text_buffer, FONT_DREAMSPEAK, COLOR_WHITE24, buttons_style);
    buttons_text_widget.font.resource_id = FONT_DREAMSPEAK;
    buttons_text_widget.font.color = COLOR_WHITE24;
    
    sprintf(buttons_text_buffer, "1  2  3  +  4  5  6  -  7  8  9  c  0  x  /  =");
    pulse_render_text(&buttons_textbox, &buttons_text_widget);
   	
   	draw_selector(green);
}

void draw_selector(color24_t color)
{
	draw_box(selector_x[selector_column], selector_y[selector_row], selector_size, 1, color);
	draw_box(selector_x[selector_column], selector_y[selector_row] + 1, 1, selector_size - 2, color);
	draw_box(selector_x[selector_column] + selector_size - 1, selector_y[selector_row] + 1, 1, selector_size - 2, color);
	draw_box(selector_x[selector_column], selector_y[selector_row] + selector_size - 1, selector_size, 1, color);	
}


void print_to_display()
{
    //setting text box for the text
    text_box.top = 4;
    text_box.left = 0;
    text_box.right = SCREEN_WIDTH;
    text_box.bottom = 28;
    
    enum PWTextStyle display_style = (PWTS_VERTICAL_CENTER | PWTS_CENTER);
	pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_LCD_DIGI_DS, COLOR_BLACK24, display_style);
    text_widget.font.resource_id = FONT_LCD_DIGI_DS;
    text_widget.font.color = COLOR_BLACK24;
    
    //Prints the text to the screen
    draw_calculator();

	sprintf(text_buffer, display_text);
	pulse_render_text(&text_box, &text_widget);
	
	if (operation == 1)
		printf("%d+",num_1);
	else if (operation == 2)
		printf("%d-",num_1);
	else if (operation == 3)
		printf("%dx",num_1);
	else if (operation == 4)
		printf("%d/",num_1);
	if (current_screen == SCREEN_ANSWER)
		printf("%d=",num_2);
}

void append_to_display(char c)
{
	int len = strlen(display_text);
	display_text[len] = c;
	display_text[len+1] = '\0';
}


void draw_box(int x,int y,int width, int height, color24_t color)
{
    // Set the canvas dimensions to be a 4 x 4 box
    pulse_set_draw_window(x, y, x + width - 1, y + height - 1);

    // Draw pixels onto that canvas
    for (int i = 0; i < width * height; i++) 
        pulse_draw_point24(color);
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


// Render clock text and draw to screen
void render_time()
{
    text_box.top = 0;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT-26;

    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER | PWTS_VERTICAL_CENTER);
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_26, green, style);
	print_time_into_text_buffer();
    pulse_render_text(&text_box, &text_widget);
}


// Render clock text and draw to screen
void render_date()
{
    text_box.top = SCREEN_HEIGHT/2+4;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT;

    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER );
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_10, grey, style);
    sprintf(text_buffer, "%d/%d/%ld", current_time.tm_mon+1, current_time.tm_mday, current_time.tm_year+1900);
    pulse_render_text(&text_box, &text_widget);
}

void return_to_clock_screen()
{
    pulse_get_time_date(&current_time);
	pulse_blank_canvas();
    current_screen = SCREEN_CLOCK;
    render_time();
    render_date();

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
