/*
Computer Science 145: Embedded Systems
Project 2:
Ananya Karra
Sachita Rayapati
*/

#include <stdio.h>
#include <string.h>
#include "avr.h"
#include "lcd.h"

int NUM_RECORDINGS = 10;
int stopwatch[10] = {0};
	
// Frequencies for notes on Octave 3
#define A 220
#define As 233.08
#define B 246.94
#define C 130.81
#define C1 277.18
#define D 146.83
#define Ds 155.56
#define E 164.81
#define F 174.61
#define Fs 185
#define G 196
#define Gs 207.65

#define MAX_MONTH 12
#define MIN_MONTH 1
#define MAX_DAY   31
#define MIN_DAY   1
#define MAX_YEAR  2030
#define MIN_YEAR  2016
#define MAX_HOUR  23
#define MIN_HOUR  0
#define MAX_MIN   59
#define MIN_MIN   0
#define MAX_SECOND   59
#define MIN_SECOND   0
#define true 1
#define false 0

typedef struct {
	int year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} DateTime;

typedef struct{
	char date[17];
	char time[17];
} DateTimeAlarm;

DateTimeAlarm dta[10];

int monthsw30days[] = {4, 6, 9, 11}; // Array storing months with 30 days
int monthsw31days[] = {1, 3, 5, 7, 8, 10}; // Array storing months with 31 days
// Other would be directly accessed as {2}

/*
Default values on the display screen.
Values chosen test military time hour change.
*/
void init_dt(DateTime *dt) {
	dt->year = 2008;
	dt->month = 2;
	dt->day = 28;
	dt->hour = 23;
	dt->minute = 59;
	dt->second = 57;
}

const char keypad[4][4] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'},
};

// Checks if key is pressed on the Keypad
//----ALL 8 GPIOs TO N/C---
int is_pressed(int r, int c){

	DDRC = 0x00;
	PORTC = 0x00;
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	CLR_BIT(DDRC, c+4);
	SET_BIT(PORTC, c+4);
	avr_wait(10);
	if (GET_BIT(PINC, c+4)==0){
		return 1;
	}
	return 0;
}

// If key was pressed, return the key value from the keypad.
char get_key(){
	int i, j;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(is_pressed(i,j)){
				avr_wait(50);
				return keypad[i][j];
			}
		}
		avr_wait(10);
	}
	return '$';
}


// Initial output on display screen
void print_first() {
	char buf[17];
	char buff[17];
	lcd_pos(0, 0);
	sprintf(buf, "Project 5");
	
	lcd_puts2(buf);
	lcd_pos(1, 0);
	sprintf(buff, "Alarm Clock");
	lcd_puts2(buff);
}

typedef struct {
	double frequency;
	double duration;
} MusicNote;

#define Wait2 2.0
#define Wait1 1.0
#define Wait2_5 2.5
#define Wait4 4.0

// Notes for Jurassic Theme {note_frequency, duration}
MusicNote Jurassic[] = {
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {A,Wait2_5}, {As,Wait2}, {F,Wait2}, {As,Wait2}, {Gs,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2},
	{As,Wait2}, {A,Wait2}, {As,Wait2}, {Ds,Wait2}, {D,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {A,Wait2}, {As,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {A,Wait2_5}, {As,Wait2}, {F,Wait2}, {As,Wait2}, {Gs,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2},
	{As,Wait2}, {A,Wait2}, {As,Wait2}, {Ds,Wait2}, {D,Wait2}, {Ds,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {A,Wait2}, {As,Wait2_5}
};


/*
From AVR_C with updated frequency wait period
for better and precise sound quality.
*/
void avr_wait_main(unsigned short msec) {
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.00001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}


// Default setting of volume to 1 (Switched ON)
int volume_button = 1;

int playing_note(const MusicNote* play_note) {
	if(volume_button){
		double freq = play_note->frequency;
		double time = play_note->duration;
		double load = 1/freq;
		int comp = (load/2) * 10000; // convert to ms
		int note = time / load;
		for(int i = 0; i < note; i++){
			// Sets third bit of PORTB to 1
			SET_BIT(PORTB, 3); // Switch Speaker ON
			avr_wait_main(comp); // Time is High
			// Clears third bit of PORTB to 0
			CLR_BIT(PORTB, 3); // Switch Speaker OFF
			avr_wait_main(comp); // Time if Low
		}
	}
	return 1; // continue alarm
}


/*
Loops through every note in the song's array.
If key A, B, C, or D are selected, the song
changes immediately to the chosen one.
*/
void advance_dt();
int song(DateTime *dt, MusicNote music[], int notes) {
	int flag = 0;
	if(volume_button) {
		for(int i = 0; i < notes; i++) {
			char key = get_key();
			if(playing_note(&music[i])){
				if(key == '8' || i == 5){
					flag++;
					for(int i = 0; i < 5; i++){
						avr_wait(1000);
						advance_dt(dt);
					}
					advance_dt(dt);
				}
				else if(key == '9'){
					return 0;
				}
			}
		}
	}
	return flag;
}

void print_song() {
	char buf1[17];
	lcd_pos(0,0);
	sprintf(buf1, "Alarm Ringing");
	lcd_puts2(buf1);
	
	char buf2[17];
	lcd_pos(1,0);
	sprintf(buf2, "Peek-A-Boo!");
	lcd_puts2(buf2);
}

/*
The function checks if the month is in the
30 or 30 days array (monthsw30days[] or monthsw31days[])
*/
int contains_month(int month, int *months, int len_months) {
	for (int i = 0; i < len_months; i++) {
		if (month == months[i]) {
			return 1;
		}
	}
	return 0;
}

/*
@brief: Advances the date and time in
nested logic, ensuring that when the second
is updated, it runs a series of checks that ensure
that the new seconds value is valid, and subsequent
minute, hour, day, month, and year values correspond
to the user's settings.
*/
void advance_dt(DateTime *dt, int isMilitary)
{
	++dt->second;
	if (dt->second == 60)
	{
		dt->minute++;
		dt->second = 0;
		
		if ((dt->minute == 60))
		{
			dt->hour++;
			dt->minute = 0;
			if(dt->hour == 24){
				dt->hour = 0;
				dt->day++;
				if (dt->year % 4 == 0 && dt->day > 29)
				{
					if (dt->month == 2){
						dt->day = 1;
						dt->month++;
					}
				}
				else if (dt->year % 4 != 0 && dt->day > 28){
					if(dt->month == 2){
						dt->day = 1;
						dt->month++;
					}
				}
				else if((dt->day == 30) && (contains_month(dt->month, monthsw30days, 4))){
					dt->month++;
					dt->day = 1;
				}
				else if((dt->day == 31) && (contains_month(dt->month, monthsw31days, 4))){
					dt->month++;
					dt->day = 1;
				}
				if(dt->month > 12){
					dt->year++;
					dt->month = 1;
				}
				
			}
		}
		
	}
	
	if ((isMilitary) && (dt->hour >= 24)) {
		dt->day++;
		dt->hour = 0;
	}
	if((isMilitary) && (dt->hour == 12)){
		dt->hour = 0;
	}
	else if ((!isMilitary) && (dt->hour > 12)) {
		if(dt->minute == 0 && dt->hour == 0){
			dt->day++;
		}
		dt->hour = 1;
	}
	else if ((!isMilitary) && dt->hour == 0){
		dt->hour = 12;
	}
}

// Incremental updates of value on the editing mode
int increment_value(int value, int max_limit, int min_limit) {
	return (value == max_limit) ? min_limit : value + 1;
}

// Decremental updates of value on the editing mode
int decrement_value(int value, int min_limit, int max_limit) {
	return (value == min_limit) ? max_limit : value - 1;
}

/*
Function updates changes in editing mode.
If function is changed in the editing mode,
depending on the case, the switch case changed
the value on the display accordingly
*/
void update_dt(DateTime *dt1, DateTime *dt, char isNoted, int isMilitary) {
	char name;
	advance_dt(dt1, 1);
	switch(isNoted){
		// Date: Row 1
		case '1':
		name = increment_value(dt->month, MAX_MONTH-1, MIN_MONTH);
		char isNoted = name;
		dt->month = isNoted;
		break;
		
		case '2':
		dt->month = decrement_value(dt->month, MIN_MONTH, MAX_MONTH);
		break;
		
		case '3':
		if(contains_month(dt->month, monthsw30days, 4)){
			dt->day = increment_value(dt->day, MAX_DAY-1, MIN_DAY);
		}
		else if(contains_month(dt->month, monthsw31days, 6)){
			dt->day = increment_value(dt->day, MAX_DAY, MIN_DAY);
		}
		else if(dt->month == 2){
			if(dt->year % 400 == 0){
				dt->day = increment_value(dt->day, MAX_DAY-2, MIN_DAY);
			}
			else {
				dt->day = increment_value(dt->day, MAX_DAY-3, MIN_DAY);
			}
		}
		break;
		
		case '4':
		dt->day = decrement_value(dt->day, MIN_DAY, MAX_DAY);
		break;
		
		case '5':
		dt->year = increment_value(dt->year, MAX_YEAR, MIN_YEAR);
		break;
		
		case '6':
		dt->year = decrement_value(dt->year, MIN_YEAR, MAX_YEAR);
		break;
		
		// Time Row: 2
		case '7':
		if(isMilitary){
			dt->hour = increment_value(dt->hour, MAX_HOUR, MIN_HOUR);
		}
		else{
			dt->hour = increment_value(dt->hour, MAX_HOUR-11, MIN_HOUR);
		}
		break;
		
		case '8':
		if(isMilitary){
			dt->hour = decrement_value(dt->hour, MIN_HOUR, MAX_HOUR);
		}
		else{
			dt->hour = decrement_value(dt->hour, MIN_HOUR, MAX_HOUR-11);
		}
		break;

		case '9':
		dt->minute = increment_value(dt->minute, MAX_MIN, MIN_MIN);
		break;
		
		case 'A':
		dt->minute = decrement_value(dt->minute, MIN_MIN, MAX_MIN);
		break;
		
		case '0':
		dt->second = increment_value(dt->second, MAX_SECOND, MIN_SECOND);
		break;
		
		case 'C':
		dt->second = decrement_value(dt->second, MIN_SECOND, MAX_SECOND);
		break;
	}
}

//checking if alarm is set at a particular time
int is_alarm_set(const char *date, const char *time){
	for(int i = 0; i < 10; i++){
		if(strcmp(dta[i].date, date) == 0 && strcmp(dta->time, time) == 0){
			return 1;
		}
	}
	return 0;
}

/*
Prints date and time on the LCD.
The format is based on the military_time setting
(24 hour clock or A.M./P.M.).
*/
void print_dt(DateTime *dt, int isMilitary)
{
	char buf[17];
	lcd_pos(0,0);
	sprintf(buf, "%04d-%02d-%02d", dt->year, dt->month, dt->day);
	lcd_puts2(buf);

	char buf2[17];
	lcd_pos(1,0);
	if (isMilitary) {
		sprintf(buf2, "%02d:%02d:%02d", dt->hour, dt->minute, dt->second);
		lcd_puts2(buf2);
		} else {
		if (dt->hour < 13) {
			sprintf(buf2, "%02d:%02d:%02d AM", dt->hour, dt->minute, dt->second);
			lcd_puts2(buf2);
			} else {
			sprintf(buf2, "%02d:%02d:%02d PM", dt->hour, dt->minute, dt->second);
			lcd_puts2(buf2);
		}
	}
	
	// if alarm matches with current time, song plays
	if(is_alarm_set(buf, buf2)){
		int HB_Length = sizeof(Jurassic)/sizeof(Jurassic[0]);
		print_song();
		song(dt, Jurassic, HB_Length);
		lcd_clr();
		avr_wait(1000);
	}
}

// Storing MAX 10 Alarms.
DateTimeAlarm store_data(const DateTime *dt1, int militarytime){
	DateTimeAlarm temp;
	char storeDate[17];
	char storeTime[17];
	sprintf(storeTime, "%04d-%02d-%02d", dt1->year, dt1->month, dt1->day);
	if(militarytime) {
		sprintf(storeDate, "%04d-%02d-%02d", dt1->year, dt1->month, dt1->day);
	}
	else{
		if (dt1->hour < 13) {
			sprintf(storeDate, "%02d:%02d:%02d AM", dt1->hour, dt1->minute, dt1->second);
			} else {
			sprintf(storeDate, "%02d:%02d:%02d AM", dt1->hour, dt1->minute, dt1->second);
		}
	}
	strcpy(temp.date, storeDate);
	strcpy(temp.time, storeTime);
	return temp;
}

void initialize_lcd() {
	/* initial output on display screen */
	char buf[17];
	lcd_pos(0, 0);
	sprintf(buf, "Project 5");
	lcd_puts2(buf);
	
	char buff[17];
	lcd_pos(1, 0);
	sprintf(buff, "Alarm and Stopwatch");
	lcd_puts2(buff);
}

void decouple_readings(int *hour, int *min, int *sec, int *total_seconds) {
	// stopwatch backwards modulo operations with hour min and second times /* series of modulo operations */
	*hour = *total_seconds / 3600;
	*total_seconds %= 3600;
	*min = *total_seconds / 60;
	*sec = *total_seconds % 60;
}

void print_stopwatch_reading(int counter) {
    /* print stopwatch lapse number and reading
    limit preset = 10 lapses */
    int hour = 0;
	int min = 0;
	int sec = 0;
    decouple_readings(&hour, &min, &sec, &counter);
    char lapse_buf[17];
	lcd_pos(0, 0);
	sprintf(lapse_buf, "%02d:%02d:%02d", hour, min, sec); /* needs a reformat */
	lcd_puts2(lapse_buf);
}


/* Iterates through the readings and prints
all the lapse recondings under hour, minute,
and second with corresponding lapse number. */
void print_all_readings() {
	for (size_t i=0; i<NUM_RECORDINGS; i++) {
		/* initialize the values to  be read*/
		int hour = 0;
		int min = 0;
		int sec = 0;
		/* decouple readings takes the counter value
		and calculates the hour, minute, second values
		of stopwatch */
		decouple_readings(&hour, &min, &sec, &stopwatch[i]);

		/* displays the lapse number */
		char lapse_number[17];
		lcd_pos(0, 0);
		sprintf(lapse_number, "LAPSE: %01d", i); /* needs a reformat */
		lcd_puts2(lapse_number);

		/* displays the hour, minute, and second ticking for the corresponding
		lapse. There is a delay of 1 second between each display */
		char lapse_buf[17];
		lcd_pos(1, 0);
		sprintf(lapse_buf, "%02d:%02d:%02d", hour, min, sec);
		lcd_puts2(lapse_buf);
		avr_wait(1000);
		lcd_clr();
	}
}

void initializations() {
	SET_BIT(DDRB, 4);
	SET_BIT(PORTB, 4);
	avr_init();
	lcd_init();
	initialize_lcd();
}

/* stopwatch model uses keys
to set (start lapses / continue them)
and exit the mode. the stop watcher
counter gets reset after previous
recording gets stored in an array of
stopwatch values. */
void stopwatchMode() {
	// initialize stopwatch values to 0
	int stop_watch_counter = 0;
	int stop_watch_lapse_number = 0;
	while(1) {
		char key = get_key();
		// add in the lapses
		if (key == 'C') {
			stopwatch[stop_watch_lapse_number] = stop_watch_counter;
			stop_watch_counter = 0;
			} else if (key == 'D') { /* terminate */
			stopwatch[stop_watch_lapse_number] = stop_watch_counter;
			stop_watch_counter = 0;
			break;
			} else if (key == 'B') {
			print_all_readings();
		}
		print_stopwatch_reading(stop_watch_counter);
		avr_wait(1000);
		lcd_clr();
		stop_watch_counter += 1;
	}
}

int main(){
	avr_init();
	DateTime dt;
	DateTime dt1; // Making another instance so clock can from dt can keep ticking in the background
	lcd_init();
	init_dt(&dt);
	init_dt(&dt1);
	SET_BIT(DDRB, 3);
	int military_time = true;
	print_first(); // Prints Project 5 message
	avr_wait(1000);
	lcd_clr();
	print_dt(&dt, military_time); // Prints time from the init values
	avr_wait(1000);
	while(1){
		char key = get_key();
		int count = 0;
		if(key == '4'){
			lcd_clr();
			lcd_pos(0,0);
			lcd_puts2("Entering Mode:");
			lcd_pos(1,0);
			lcd_puts2("Alarm!!");
			stopwatchMode();
			avr_wait(2000);
			lcd_clr();
			// Exit from alarm state
			while(key != '#') {
				key = get_key();
				// * records the current set time as an alarm
				if(key != '*'){
					update_dt(&dt, &dt1, key, military_time);
					print_dt(&dt1, military_time);
				}
				if(key == '*'){
					dta[count++] = store_data(&dt1, military_time);
					// If exceeds 10, overwrites from 0;
					if(count == 10) {count = 0;}
				}
			}
		}
		else if (key == 'C') { /* start */  /* if already started then terminate current lapse and start another one */
			lcd_clr();
			lcd_pos(0,0);
			lcd_puts2("Entering Mode:");
			lcd_pos(1,0);
			lcd_puts2("Stopwatch!!");
			stopwatchMode();
		}
		lcd_clr();
		advance_dt(&dt, military_time);
		print_dt(&dt, military_time);
		avr_wait(1000);
	}
	return 0;
}