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
