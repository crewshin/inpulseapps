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

#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 128
#define BALL_SIZE 5
#define ENEMY_SIZE 6
#define GAP_WIDTH 16

// Time required to be considered a hold in ms
#define BUTTON_HOLD_TIME 500
// Processor will go to sleep in 15 sec if no activity on the clock screen
#define TIME_BEFORE_SLEEP 10000

#include <pulse_os.h>
#include <pulse_types.h>
#include <app_resources.h>
#include <stdint.h>
#include <stddef.h>

uint32_t time_last_box_drawn_ms = 0;
uint32_t time_last_bullet_fired_ms = 0;
uint32_t time_last_enemies_moved_ms = 0;
int start_y = 0;
int score;
int highscore = 0;
int ball_x;
int ball_y;
int enemy_y[7];
int enemy_x[7];
bool enemy_alive[7];
int bullet_x[10];
int bullet_y[10];
bool bullet_fired[10];
int enemy_bullet_x[10];
int enemy_bullet_y[10];
bool enemy_bullet_fired[10];
int game_speed = 50;
int direction;
int i;
int j;
int bullet_number;
bool all_cleared;
bool fired;
bool press;
bool playing;
bool button_held = false;

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

int hold_timer_value;

// Current screen being displayed
Screen current_screen;

color24_t color;

color24_t green = { 0x44, 0x99, 0x11, 0x00 };//green
color24_t teal = { 0x00, 0xCC, 0xAA, 0x00 };//teal
color24_t blue = { 0x00, 0x40, 0xFF, 0x00 };//blue
color24_t purple = { 0x70, 0x00, 0x80, 0x00 };//purple
color24_t pink = { 0xFF, 0x66, 0xFF, 0x00 };//pink
color24_t orange = { 0xFF, 0x80, 0x00, 0x00 };//orange
color24_t red = { 0xFF, 0x00, 0x00, 0x00 };//red

color24_t clock_color = { 0x44, 0x99, 0x11, 0x00 };

uint32_t time_now_ms;

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
	//if (current_screen == SCREEN_CLOCK || current_screen == SCREEN_GAME_OVER)
	if ( current_screen == SCREEN_GAME_OVER)
		button_timer_id = pulse_register_timer(BUTTON_HOLD_TIME, &long_press, 0);
	else if (current_screen == SCREEN_GAMEPLAY)
		press = true;
}

void long_press() 
{
	button_held = true;

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
			reset_game();
			pulse_blank_canvas();
			current_screen = SCREEN_GAME_OVER;
			printf("\n\n\n\n\n Click to start");				
			prepare_to_sleep();				
		}
		else if (current_screen == SCREEN_GAME_OVER)
		{
			time_now_ms = pulse_get_millis();
			if (time_now_ms - time_last_box_drawn_ms >= 500) {
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

// Render clock text and draw to screen
void render_time(color24_t clock_color)
{
    // Initialize the text box parameters
    text_box.top = 0;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT-26;

    // Configure the text style
    // We want it to be centered both vertically and horizontally in the text box
    // and to truncate any text if it won't fit in the text box
    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER | PWTS_VERTICAL_CENTER);

    // Initialize the text widget
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_26, clock_color, style);

    // Print the time from the RTC (real time clock) in to the text buffer
    print_time_into_text_buffer();

    // Set font resource for clock
    text_widget.font.resource_id =  FONT_GOOD_TIMES_26;

    // Set the font color to be psuedo-random
    text_widget.font.color = clock_color;

    // Render the text in the text box area
    pulse_render_text(&text_box, &text_widget);
}

void return_to_clock_screen()
{
    pulse_blank_canvas();
    current_screen = SCREEN_CLOCK;
    render_time(clock_color);
    //render_date(clock_color);

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
	ball_x = 48;
	ball_y = SCREEN_HEIGHT-10;
	for (i = 0; i < 7; i++) {
		enemy_x[i] = (i+1)*12;
		enemy_alive[i] = true;
	}
	enemy_y[0] = 10;
	enemy_y[1] = 20;
	enemy_y[2] = 10;
	enemy_y[3] = 30;
	enemy_y[4] = 10;
	enemy_y[5] = 20;
	enemy_y[6] = 10;
	for (i = 0; i < 10; i++) {
		bullet_fired[i] = false;
		enemy_bullet_fired[i] = false;
	}
	game_speed = 20;
	press = false;
	playing = false;
	color = green;
	bullet_number = 0;
}

void draw_box(int x, int y, int width, int height, color24_t color) {
    pulse_set_draw_window(x, y, x + width - 1, y + height - 1);
    for (int i = 0; i < width * height; i++) {
        pulse_draw_point24(color);
    }
}

void draw_ball(int x, int y, color24_t color) {
	draw_box(x, y, BALL_SIZE, BALL_SIZE, color);
	
	draw_box(x, y, 1, 1, COLOR_BLACK24);
	draw_box(x+1, y, 1, 3, COLOR_BLACK24);
	draw_box(x+3, y, 1, 3, COLOR_BLACK24);
	draw_box(x+4, y, 1, 1, COLOR_BLACK24);
	draw_box(x, y+3, 1, 1, COLOR_BLACK24);
	draw_box(x+4, y+3, 1, 1, COLOR_BLACK24);
}

void draw_enemy(int x, int y, color24_t color) {
	draw_box(x, y, ENEMY_SIZE, ENEMY_SIZE, color);
	
	draw_box(x+2, y, 2, 1, COLOR_BLACK24);
	draw_box(x, y+1, 1, 1, COLOR_BLACK24);
	draw_box(x+5, y+1, 1, 1, COLOR_BLACK24);
	draw_box(x, y+3, 1, 3, COLOR_BLACK24);
	draw_box(x+5, y+3, 1, 3, COLOR_BLACK24);
	draw_box(x+1, y+4, 1, 2, COLOR_BLACK24);
	draw_box(x+4, y+4, 1, 2, COLOR_BLACK24);
}

void move_enemies() {
	for (i = 0; i < 8; i++) {
		draw_enemy(enemy_x[i], enemy_y[i], COLOR_BLACK24);
		if (enemy_alive[i]) {
			direction = rand()%2;
			if (direction == 0) {
				direction = -1;
			} else {
				direction = 1;
			}
			if ((enemy_x[i] > 0) && (enemy_x[i] < SCREEN_WIDTH-ENEMY_SIZE-1)) {
				enemy_x[i] = enemy_x[i] + direction;
			}
			draw_enemy(enemy_x[i], enemy_y[i], red);
		}
	}
}

void draw_bullet(int x, int y, color24_t color) {
	draw_box(x, y, 1, 1, color);
}

void move_bullets() {
	for (i = 0; i < 11; i++) {
		draw_bullet(bullet_x[i], bullet_y[i], COLOR_BLACK24);
		if (bullet_y[i] == 0) {
			bullet_fired[i] = false;
		}
		if (bullet_fired[i]) {
			bullet_y[i]--;
			draw_bullet(bullet_x[i], bullet_y[i], blue);
			for (j = 0; j < 8; j++) {
				if ((bullet_x[i] > enemy_x[j]) && (bullet_x[i] < enemy_x[j]+ENEMY_SIZE) && (bullet_y[i] > enemy_y[j]) && (bullet_y[i] < enemy_y[j]+ENEMY_SIZE) && (enemy_alive[j])) {
					bullet_fired[i] = false;
					enemy_alive[j] = false;
					score = score + 10;
				}
			}
		}
	}
}

void move_enemy_bullets() {
	for (i = 0; i < 11; i++) {
		draw_bullet(enemy_bullet_x[i], enemy_bullet_y[i], COLOR_BLACK24);
		if (enemy_bullet_y[i] == SCREEN_HEIGHT-1) {
			enemy_bullet_fired[i] = false;
		}
		if (enemy_bullet_fired[i]) {
			enemy_bullet_y[i]++;
			draw_bullet(enemy_bullet_x[i], enemy_bullet_y[i], red);
			if ((enemy_bullet_x[i] > ball_x) && (enemy_bullet_x[i] < ball_x+BALL_SIZE) && (enemy_bullet_y[i] > ball_y) && (enemy_bullet_y[i] < ball_y+BALL_SIZE)) {
				game_over();
			}
		}
	}
}

void fire_bullet(int x) {
	for (i = 0; i < 11; i++) {
		if (!bullet_fired[i]) {
			bullet_fired[i] = true;
			bullet_x[i] = x+2;
			bullet_y[i] = SCREEN_HEIGHT - 11;
			break;
		}
	}
}

void fire_enemy_bullet() {
	fired = false;
	bullet_number++;
	if (bullet_number == 7) {
		fired = true;
		bullet_number = 0;
	}
	while (!fired) {
		for (i = 0; i < 11; i++) {
			if (!enemy_bullet_fired[i]) {
				j = rand()%7;
				if (enemy_alive[j]) {
					enemy_bullet_fired[i] = true;
					fired = true;
				}
				enemy_bullet_x[i] = enemy_x[j]+3;
				enemy_bullet_y[i] = enemy_y[j]+ENEMY_SIZE+1;
				break;
			}
		}
	}
}

void game_over() {
	pulse_vibe_on();
	pulse_mdelay(60);
	pulse_vibe_off();current_screen = SCREEN_GAME_OVER;
	pulse_blank_canvas();
	if (score > highscore)
		highscore = score;
	printf ("GAME OVER\n\nScore: %d\n\nHighscore: %d\n\nClick to restart",score,highscore);
	reset_game();
	prepare_to_sleep();
}

void main_app_loop() {
	if (current_screen == SCREEN_GAMEPLAY) {
		time_now_ms = pulse_get_millis();
		if (time_now_ms - time_last_box_drawn_ms >= game_speed) {
	        draw_ball(ball_x,ball_y, COLOR_BLACK24);
			if(press && ball_x < SCREEN_WIDTH - BALL_SIZE)
				ball_x++;
			else if (!press && ball_x > 0)
				ball_x--;
			if (time_now_ms - time_last_bullet_fired_ms >= 400) {
				fire_bullet(ball_x);
				fire_enemy_bullet();
				time_last_bullet_fired_ms = time_now_ms;
			}
			if (time_now_ms - time_last_enemies_moved_ms >= 1000) {
				move_enemies();
				time_last_enemies_moved_ms = time_now_ms;
			}			
			move_bullets();
			move_enemy_bullets();
			draw_ball(ball_x, ball_y, green);
			time_last_box_drawn_ms = time_now_ms;
			all_cleared = true;
			for (i = 0; i < 8; i++) {
				if (enemy_alive[i]) {
					all_cleared = false;
				}
			}
			if (all_cleared) {
				for (i = 0; i < 7; i++) {
					enemy_alive[i] = true;
				}
			}
		}
	} else if (current_screen == SCREEN_CLOCK) {
		pulse_get_time_date(&current_time);
	}
}

void main_app_handle_doz() {
}

void main_app_handle_hardware_update( enum PulseHardwareEvent event) {
}
