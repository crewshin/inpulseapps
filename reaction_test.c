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
