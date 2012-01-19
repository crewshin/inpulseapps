#include <pulse_os.h>
#include <pulse_types.h>

#include <stdint.h>

struct pulse_time_tm current_time;
int hold_timer_value;
bool held = false;
bool counting = true;

void main_app_init() {
	print_time_to_screen();
}

void print_time_to_screen() {
	if (counting) {
		pulse_blank_canvas();
		printf("%02d:%02d:%02d\n", current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
		current_time.tm_sec++;
		if (current_time.tm_sec == 60) {
			current_time.tm_min++;
			current_time.tm_sec = 0;
		}
		if (current_time.tm_min == 60) {
			current_time.tm_hour++;
			current_time.tm_min = 0;
		}
	}
	pulse_register_timer(1000, &print_time_to_screen, 0);
}

void click_hold() {
	held = true;
}

void main_app_handle_button_down() {
	hold_timer_value = pulse_register_timer(1000, &click_hold, 0);
}

void main_app_handle_button_up() {
	if(!held) {
		pulse_cancel_timer(&hold_timer_value);
		if (counting) {
			counting = false;
		}
		else {
			counting = true;
		}
	}
	else {
		current_time.tm_hour = 0;
		current_time.tm_min = 0;
		current_time.tm_sec = 0;
	}
	held = false;
}

void main_app_loop() {
}

void main_app_handle_doz() {
}

void main_app_handle_hardware_update(enum PulseHardwareEvent event) {
}
