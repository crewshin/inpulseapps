#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 128
#define BALL_SIZE 8
#define GAP_WIDTH 24

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
int score;
int highscore = 0;
int ball_x;
int ball_y;
int jump_ticks = 0;
int surface_y[8];
int surface_start_x[8];
int surface_length_x[8];
int game_speed = 50;
int jumps_available = 2;

bool ceiling;
bool press;
bool playing;
bool falling = false;
bool button_held;

void reset_game();
void long_press();
void return_to_clock_screen();

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

color24_t surface_color[8];
color24_t colors[8];

color24_t clock_color = { 0x44, 0x99, 0x11, 0x00 };
color24_t date_color = { 0x60, 0x60, 0x60, 0x00 };

uint32_t time_now_ms;

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
	else if ((current_screen == SCREEN_GAMEPLAY) && (jumps_available > 0)) {
		press = true;
		jump_ticks = 15;
		jumps_available--;
	}
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
			printf("Click to start.\n\n\nClick while \nyou're in the \nair to do a \ndouble jump!");				
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
void render_date(color24_t clock_color)
{
    // Initialize the text box parameters
    text_box.top = SCREEN_HEIGHT/2+4;
    text_box.right = SCREEN_WIDTH;
    text_box.left = 0;
    text_box.bottom = SCREEN_HEIGHT;

    // Configure the text style
    // We want it to be centered both vertically and horizontally in the text box
    // and to truncate any text if it won't fit in the text box
    enum PWTextStyle style = (PWTS_TRUNCATE | PWTS_CENTER );

    // Initialize the text widget
    pulse_init_dynamic_text_widget(&text_widget, text_buffer, FONT_GOOD_TIMES_10, date_color, style);

    // Print the time from the RTC (real time clock) in to the text buffer
    print_date_into_text_buffer();

    // Set font resource for clock
    text_widget.font.resource_id =  FONT_GOOD_TIMES_10;

    // Set the font color to be psuedo-random
    text_widget.font.color = date_color;

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

void reset_game() {
	score = 0;
	ball_x = 20;
	ball_y = SCREEN_HEIGHT-26-5;
	surface_y[0] = SCREEN_HEIGHT-16;
	surface_y[1] = SCREEN_HEIGHT-32;
	surface_y[2] = SCREEN_HEIGHT-48;
	surface_y[3] = SCREEN_HEIGHT-64;
	surface_y[4] = SCREEN_HEIGHT-80;
	surface_y[5] = SCREEN_HEIGHT-96;
	surface_y[6] = SCREEN_HEIGHT-112;
	surface_start_x[0] = 0;
	surface_start_x[1] = 20;
	surface_start_x[2] = 60;
	surface_start_x[3] = 80;
	surface_start_x[4] = 30;
	surface_start_x[5] = 10;
	surface_start_x[6] = 100;
	surface_length_x[0] = 50;
	surface_length_x[1] = 30;
	surface_length_x[2] = 30;
	surface_length_x[3] = 30;
	surface_length_x[4] = 30;
	surface_length_x[5] = 30;
	surface_length_x[6] = 30;
	surface_color[0] = red;
	surface_color[1] = blue;
	surface_color[2] = teal;
	surface_color[3] = orange;
	surface_color[4] = pink;
	surface_color[5] = purple;
	surface_color[6] = green;
	colors[0] = red;
	colors[1] = blue;
	colors[2] = teal;
	colors[3] = orange;
	colors[4] = pink;
	colors[5] = purple;
	colors[6] = green;
}

void draw_box(int x, int y, int width, int height, color24_t color) {
    if (x + width >= 96) {
		width = 96 - x;
	}
    pulse_set_draw_window(x, y, x + width - 1, y + height - 1);
    for (int i = 0; i < width * height; i++) {
        pulse_draw_point24(color);
    }
}

void draw_ball(int x, int y, color24_t color) {
	pulse_draw_image(IMAGE_SPRITE, x, y-15);
}

void clear_ball(int x, int y) {
	draw_box(x, y-15, 8, 15, COLOR_BLACK24);
}

void draw_surface(int start_x, int length_x, int y, color24_t color) {
	draw_box(start_x, y, length_x, 1, color);
}

int i;

void move_surfaces() {
	for (i = 0; i < 7; i++) {
		if (surface_start_x[i] < 96)
			draw_surface(surface_start_x[i], surface_length_x[i], surface_y[i], COLOR_BLACK24);
		if (surface_start_x[i] == 0) {
			surface_start_x[i] = surface_start_x[i] + 1;
			surface_length_x[i] = surface_length_x[i] - 1;
		}
		surface_start_x[i] = surface_start_x[i] - 1;
		if (surface_start_x[i] < 96)
			draw_surface(surface_start_x[i], surface_length_x[i], surface_y[i], surface_color[i]);
		if (surface_length_x[i] <= 0) {
			surface_length_x[i] = (rand()%10)+(3*(7-i));
			surface_start_x[i] = (rand()%30)+96;
			int rand_color = rand()%7;
			surface_color[i] = colors[rand_color];
		}
	}		
}

void game_over() {
	pulse_vibe_on();
	pulse_mdelay(60);
	pulse_vibe_off();
	//int vib_timer = pulse_register_timer(75, &pulse_vibe_off, 0);

	current_screen = SCREEN_GAME_OVER;
	pulse_blank_canvas();
	if (score > highscore)
		highscore = score;
	printf ("GAME OVER\n\nScore: %d\n\nHighscore: %d\n\nClick to restart",score,highscore);
	reset_game();
	prepare_to_sleep();
}

int counter;

void main_app_loop() {
	if (current_screen == SCREEN_GAMEPLAY) {
		time_now_ms = pulse_get_millis();
		if (time_now_ms - time_last_box_drawn_ms >= game_speed) {
			score++;
			move_surfaces();
			int this_height = ball_y;
			ceiling = false;
			if (jump_ticks > 0) {
				clear_ball(ball_x, ball_y);
				if (ball_y-15 >=2) {
					ball_y = ball_y - 2;
					jump_ticks--;
				} else {
					jump_ticks--;
					ceiling = true;
				}				
			} else if (ball_y < SCREEN_HEIGHT-112) {
				if ((ball_y+1 < SCREEN_HEIGHT-112) || ((surface_start_x[6]-4 > 20) || (surface_length_x[6]+surface_start_x[6] < 20))) {
					clear_ball(ball_x, ball_y);
					ball_y = ball_y + 2;
				}
			} else if (ball_y < SCREEN_HEIGHT-96) {
				if ((ball_y+1 < SCREEN_HEIGHT-96) || ((surface_start_x[5]-4 > 20) || (surface_length_x[5]+surface_start_x[5] < 20))) {
					clear_ball(ball_x, ball_y);
					ball_y = ball_y + 2;
				}
			} else if (ball_y < SCREEN_HEIGHT-80) {
				if ((ball_y+1 < SCREEN_HEIGHT-80) || ((surface_start_x[4]-4 > 20) || (surface_length_x[4]+surface_start_x[4] < 20))) {
					clear_ball(ball_x, ball_y);
					ball_y = ball_y + 2;
				}
			} else if (ball_y < SCREEN_HEIGHT-64) {
				if ((ball_y+1 < SCREEN_HEIGHT-64) || ((surface_start_x[3]-4 > 20) || (surface_length_x[3]+surface_start_x[3] < 20))) {
					clear_ball(ball_x, ball_y);
					ball_y = ball_y + 2;
				}
			} else if (ball_y < SCREEN_HEIGHT-48) {
				if ((ball_y+1 < SCREEN_HEIGHT-48) || ((surface_start_x[2]-4 > 20) || (surface_length_x[2]+surface_start_x[2] < 20))) {
					clear_ball(ball_x, ball_y);
					ball_y = ball_y + 2;
				}
			} else if (ball_y < SCREEN_HEIGHT-32) {
				if ((ball_y+1 < SCREEN_HEIGHT-32) || ((surface_start_x[1]-4 > 20) || (surface_length_x[1]+surface_start_x[1] < 20))) {
					clear_ball(ball_x, ball_y);
					ball_y = ball_y + 2;
				}
			} else if (ball_y < SCREEN_HEIGHT-16) {
				if ((ball_y+1 < SCREEN_HEIGHT-16) || ((surface_start_x[0]-4 > 20) || (surface_length_x[0]+surface_start_x[0] < 20))) {
					clear_ball(ball_x, ball_y);
					ball_y = ball_y + 2;
				}
			} else {
				clear_ball(ball_x, ball_y);
				ball_y = ball_y + 2;
			}
			draw_ball(ball_x, ball_y, green);
			if ((this_height == ball_y) && (!ceiling)) {
				jumps_available = 2;
			}
			if (ball_y >= SCREEN_HEIGHT-1) {
				game_over();
			}
			time_last_box_drawn_ms = time_now_ms;
		} else if (current_screen == SCREEN_CLOCK) {
			pulse_get_time_date(&current_time);
		}
	}
}

void main_app_handle_doz() {
}

void main_app_handle_hardware_update( enum PulseHardwareEvent event) {
}
