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
#include <stdio.h>

struct pulse_amount {
	int hundreds, tens, ones, ten_cents, one_cents;
};
struct pulse_percent {
	int tens, ones;
};
struct pulse_people {
	int tens, ones;
};

char* string1 = "        ";
char* string2 = "        ";
char* string3 = "        ";
char* string4 = "         ";

struct pulse_amount tip_amount;
struct pulse_percent tip_percent;
struct pulse_people tip_people;

int tip_to_pay_dollars = 0;
int tip_to_pay_cents = 0;
int total_to_pay_dollars = 0;
int total_to_pay_cents = 0;
int total_per_person_dollars = 0;
int total_per_person_cents = 0;
int total_cents;

int location = 0;
int hold_timer_value;
int hold_long_timer_value;
bool held = false;

void main_app_init() {
	print_display_to_screen(location);
}

void print_display_to_screen(int edit_location) {
	
	switch (edit_location)
	{
		case 0:
			string1 = "     ^  \n";
			string2 = "\n";
			string3 = "\n";
			string4 = "\n";
			break;
		case 1:
			string1 = "      ^ \n";
			string2 = "\n";
			string3 = "\n";
			string4 = "\n";
			break;
		case 2:
			string1 = "       ^\n";
			string2 = "\n";
			string3 = "\n";
			string4 = "\n";
			break;
		case 3:
			string1 = "         ^\n";
			string2 = "\n";
			string3 = "\n";
			string4 = "\n";
			break;
		case 4:
			string1 = "          ^\n";
			string2 = "\n";
			string3 = "\n";
			string4 = "\n";
			break;
		case 5:
			string1 = "\n";
			string2 = "         ^ \n";
			string3 = "\n";
			string4 = "\n";
			break;
		case 6:
			string1 = "\n";
			string2 = "          ^\n";
			string3 = "\n";
			string4 = "\n";
			break;
		case 7:
			string1 = "\n";
			string2 = "\n";
			string3 = "         ^ \n";
			string4 = "\n";
			break;
		case 8:
			string1 = "\n";
			string2 = "\n";
			string3 = "          ^\n";
			string4 = "\n";
			break;
		case 9:
			string1 = "\n";
			string2 = "\n";
			string3 = "\n";
			string4 = "  ^    \n";
			break;
		case 10:
			string1 = "\n";
			string2 = "\n";
			string3 = "\n";
			string4 = "           ^\n";
			break;
		default:
			string1 = "     ^  \n";
			string2 = "\n";
			string3 = "\n";
			string4 = "\n";
			break;
	}
	
	printf("Amt: %01d%01d%01d.%01d%01d\n", tip_amount.hundreds, tip_amount.tens, tip_amount.ones, tip_amount.ten_cents, tip_amount.one_cents);
	printf(string1);
	printf("Percent: %01d%01d\n", tip_percent.tens, tip_percent.ones);
	printf(string2);
	printf("#People: %01d%01d\n", tip_people.tens, tip_people.ones);
	printf(string3);
	printf("Calc     Clear\n");
	printf(string4);
	printf("Tip to pay\n");
	printf("$%03d.%02d", tip_to_pay_dollars, tip_to_pay_cents/100);
	printf("\n\nTotal amount\n");
	printf("$%03d.%02d", total_to_pay_dollars, total_to_pay_cents/100);
	printf("\n\nTotal per person");
	printf("$%03d.%02d", total_per_person_dollars, total_per_person_cents);
}

void click_hold() {
	held = true;
	location++;
	if (location == 11) {
		location = 0;
	}
	pulse_blank_canvas();
	print_display_to_screen(location);
	hold_long_timer_value = pulse_register_timer(500, &click_hold, 0);
}

void main_app_handle_button_down() {
	hold_timer_value = pulse_register_timer(500, &click_hold, 0);
}

void main_app_handle_button_up() {
	if(!held) {
		pulse_cancel_timer(&hold_timer_value);
		pulse_cancel_timer(&hold_long_timer_value);
		switch (location)
		{
			case 0:
				tip_amount.hundreds++;
				if (tip_amount.hundreds == 10)
					tip_amount.hundreds = 0;
				break;
			case 1:
				tip_amount.tens++;
				if (tip_amount.tens == 10)
					tip_amount.tens = 0;
				break;
			case 2:
				tip_amount.ones++;
				if (tip_amount.ones == 10)
					tip_amount.ones = 0;
				break;
			case 3:
				tip_amount.ten_cents++;
				if (tip_amount.ten_cents == 10)
					tip_amount.ten_cents = 0;
				break;
			case 4:
				tip_amount.one_cents++;
				if (tip_amount.one_cents == 10)
					tip_amount.one_cents = 0;
				break;
			case 5:
				tip_percent.tens++;
				if (tip_percent.tens == 10)
					tip_percent.tens = 0;
				break;
			case 6:
				tip_percent.ones++;
				if (tip_percent.ones == 10)
					tip_percent.ones = 0;
				break;
			case 7:
				tip_people.tens++;
				if (tip_people.tens == 10)
					tip_people.tens = 0;
				break;
			case 8:
				tip_people.ones++;
				if (tip_people.ones == 10)
					tip_people.ones = 0;
				break;
			case 9:
				//tip_to_pay = (tip_amount.hundreds*100 + tip_amount.tens*10 + tip_amount.ones) * ((tip_percent.tens*10 + tip_percent.ones)/100);
				//total_to_pay = (tip_amount.hundreds*100 + tip_amount.tens*10 + tip_amount.ones) + tip_to_pay;
				//total_per_person = total_to_pay/(tip_people.tens*10 + tip_people.ones);
				total_cents = (tip_amount.hundreds*100 + tip_amount.tens*10 + tip_amount.ones)*100 + tip_amount.ten_cents*10 + tip_amount.one_cents;
				tip_to_pay_dollars = (total_cents * ((tip_percent.tens*10 + tip_percent.ones)))/10000;
				tip_to_pay_cents = (total_cents * ((tip_percent.tens*10 + tip_percent.ones))) % 10000;
				total_to_pay_dollars = tip_to_pay_dollars + (tip_amount.hundreds*100 + tip_amount.tens*10 + tip_amount.ones);
				total_to_pay_cents = tip_to_pay_cents + tip_amount.ten_cents*10 + tip_amount.one_cents;
				total_cents = total_to_pay_dollars*100 + total_to_pay_cents/100;
				total_per_person_dollars = (total_cents/(tip_people.tens*10 + tip_people.ones))/100;
				total_per_person_cents = (total_cents/(tip_people.tens*10 + tip_people.ones))%100;
				break;
			case 10:
				tip_to_pay_dollars = 0;
				tip_to_pay_cents = 0;
				total_to_pay_dollars = 0;
				total_to_pay_cents = 0;
				total_per_person_dollars = 0;
				total_per_person_cents = 0;
				tip_amount.hundreds = 0;
				tip_amount.one_cents = 0;
				tip_amount.ones = 0;
				tip_amount.ten_cents = 0;
				tip_amount.tens = 0;
				tip_people.ones = 0;
				tip_people.tens = 0;
				tip_percent.ones = 0;
				tip_percent.tens = 0;
				break;
			default:
				tip_amount.hundreds++;
				if (tip_amount.hundreds == 10)
					tip_amount.hundreds = 0;
		}
		pulse_blank_canvas();
		print_display_to_screen(location);
	}
	pulse_cancel_timer(&hold_timer_value);
	pulse_cancel_timer(&hold_long_timer_value);
	held = false;
}

void main_app_loop() {
}

void main_app_handle_doz() {
}

void main_app_handle_hardware_update(enum PulseHardwareEvent event) {
}
