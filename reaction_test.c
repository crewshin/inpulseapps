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

void get_ready();
void go();

// 0-waiting 1-ready 2-running
int state = 0;

int hold_timer_value;

int start_time;

int best_time = 999999;

// This function is called once after the watch has booted up
// and the OS has loaded
void main_app_init()
{
	struct pulse_time_tm current_time;
    pulse_get_time_date(&current_time);
    uint32_t sec = current_time.tm_sec;
    
    srand(sec);
	//get_ready();
	
	printf("Click to start\n");
}

void get_ready()
{
	pulse_blank_canvas();

	state = 1;
	printf("Wait for it...\n\n");
	

    int wait_time = 1000 + rand()%5000;
    
	hold_timer_value = pulse_register_timer(wait_time, &go, wait_time);
}

void go(int t)
{
		state = 2;
		printf("GO",t);
		start_time = (int)pulse_get_millis();
}

void main_app_handle_button_down()
{
    pulse_blank_canvas();
    
    if (state == 0)
		get_ready();
	else if (state == 1)
	{
		pulse_cancel_timer(&hold_timer_value);
		state = 0;
		printf("XXX Too soon XXX\n\n");
		printf("click to try \nagain");
		printf("\n\n\n\n\n\n\n\n\n\n\nBest: %dms",best_time);
	}
	else if (state == 2)
	{
		state = 0;
		
		int reaction_time = (int)pulse_get_millis() - start_time;
		printf("You reacted in \n\n%d millisecs\n\n",reaction_time);
		printf("Click to try \nagain");
		
		if (reaction_time < best_time)
			best_time = reaction_time;
		
		printf("\n\n\n\n\n\n\n\n\n\nBest: %dms",best_time);
	}		
}

void main_app_handle_button_up()
{
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
