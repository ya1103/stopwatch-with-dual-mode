/* Mini Project 2
 * Main.c
 *
 *  Created on: Aug 28, 2025
 *      Author: Yousef
 */



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define EN1 PA0
#define EN2 PA1
#define EN3 PA2
#define EN4 PA3
#define EN5 PA4
#define EN6 PA5

#define True 1
#define False 0
typedef unsigned char bool;

void Timer1_CTC_init(void){
	 TCCR1A |= (1<<FOC1A); // nonpwm
	 TCNT1 = 0; //start counting from 0
	 OCR1A = 62500; // one second
	 TIMSK |= (1<< OCIE1A); //enable MIE
	 sei(); // enable I-bit
	 TCCR1B |= (1<<WGM12) | (1<<CS12); //CTC mode and prescaler 256
}

void Segment7_init(){
	DDRA |= 0x3F; // first 6 pins to OUTPUT as enable for 7segments
	PORTA &= ~(0x3F); // turn off all 7 segments initially
	DDRC |= 0x0F; // first 4 pins of C to output
	PORTC &= ~(0x0F); // display 0 initially
}

void INT0_init(){
	DDRD &= ~(1<<PD2); //input
	PORTD |= (1<<PD2); //internal pull up ENABLE
	MCUCR |= (1<<ISC01); //falling edge
	GICR |= (1<<INT0); // MIE
	sei();
}

void INT1_init(){
	DDRD &= ~(1<<PD3); //input
	MCUCR |= (1<<ISC10) | (1<<ISC11); //rising edge
	GICR |= (1<<INT1); // MIE
	sei();
}

void INT2_init(){
	DDRB &= ~(1<<PB2); //input
	PORTB |= (1<<PB2); //internal pull up
	MCUCSR &= ~(1<<ISC2); //falling edge
	GICR |= (1<<INT2); //MIE
	sei();
}

void CountModeDisplay_init(void){
	DDRD |= 0x30; // pin PD4 PD5 as output pins
	PORTD &= ~(0x30); // initialize to be off
}

void PushButtons_init(void){
	//CountModeToggle PB7
	//PB0 Dec Hour
	//PB1 Inc Hour
	//PB2 INT2 found in INT2_init()
	//PB3 Dec Min
	//PB4 Inc Min
	//PB5 Dec Sec
	//PB6 Inc Sec

	DDRB = 0x00; // all pins input
	PORTB = 0xFF; // all pins internal pull up enable
}

// initialize all values to zero
// required to be signed to check on -ve values
signed char sec0 = 0, sec1 = 0, min0 = 0, min1 = 0, hour0 = 0, hour1 = 0;

void displaySegment(char); // declaring function

bool CountUpMode = True; // initially count mode is count up
bool flagMode = False; // initially PB7 is unflagged
bool flagHour = False, flagMin = False, flagSec = False; // initially unflagged
void PushButtonsCheck(void){

	//Count Mode LED Display
	if(CountUpMode == True){
		PORTD |= (1<<PD4); // turn CU led on
		PORTD &= ~(1<<PD5); // turn CD led off
	} else if(CountUpMode == False){
		PORTD |= (1<<PD5); // turn CD led on
		PORTD &= ~(1<<PD4); // turn CU led off
	}

	//Toggle count mode on PB7 enable and if flag is not enabled and timer is paused
	if(!(PINB & (1<<PB7)) && (flagMode == False) && !(TCCR1B & (1<<CS12)) ){
		flagMode = True;
		CountUpMode ^= 1; // Toggle
	}
	if((PINB & (1<<PB7)) && (flagMode == True)){
		flagMode = False; // if button released reset flag
	}

//	//Inc and Dec Hour on button press and flag is not enabled
	if(!(PINB & (1<<PB0)) && (flagHour == False)){
		flagHour = True;
		if((hour0 == 0) && (hour1 == 0)){
			hour0 = 9;
			hour1 = 9;
		} else{
			hour0--;
			if(hour0 == -1){
				hour0 = 9;
				hour1--;
			}
		}
	}

	if(!(PINB & (1<<PB1)) && (flagHour == False)){
		flagHour = True;
		if((hour0 == 9) && (hour1 == 9)){
			hour0 =0;
			hour1 =0;
		} else{
			hour0++;
			if(hour0 == 10){
				hour0 = 0;
				hour1++;
			}
		}
	}

	// reset flag on both buttons released and flag is enabled
	if((PINB & (1<<PB0)) && (PINB & (1<<PB1)) && (flagHour == True)){
		flagHour = False;
	}

//	//Inc and Dec Min on button press and flag is not enabled
	if(!(PINB & (1<<PB3)) && (flagMin == False)){
		flagMin = True;
		if((min0 == 0) && (min1 == 0)){
			min0 = 9;
			min1 = 5;
		} else{
			min0--;
			if(min0 == -1){
				min0 = 9;
				min1--;
			}
		}
	}

	if(!(PINB & (1<<PB4)) && (flagMin == False)){
		flagMin = True;
		if((min0 == 9) && (min1 == 5)){
			min0 = 0;
			min1 = 0;
		} else{
			min0++;
			if(min0 == 10){
				min0 = 0;
				min1++;
			}
		}
	}

	// reset flag on both buttons released and flag is enabled
	if((PINB & (1<<PB3)) && (PINB & (1<<PB4)) && (flagMin == True)){
		flagMin = False;
	}

//	//Inc and Dec Sec on button press and flag is not enabled
	if (!(PINB & (1 << PB5)) && (flagSec == False)) {
	    flagSec = True;
	    if ((sec0 == 0) && (sec1 == 0)) {
	        sec0 = 9;
	        sec1 = 5;
	    } else {
	        sec0--;
	        if (sec0 == -1) {
	            sec0 = 9;
	            sec1--;
	        }
	    }
	}

	if (!(PINB & (1 << PB6)) && (flagSec == False)) {
	    flagSec = True;
	    if ((sec0 == 9) && (sec1 == 5)) {
	        sec0 = 0;
	        sec1 = 0;
	    } else {
	        sec0++;
	        if (sec0 == 10) {
	            sec0 = 0;
	            sec1++;
	        }
	    }
	}

	// reset flag on both buttons released and flag is enabled
	if ((PINB & (1 << PB5)) && (PINB & (1 << PB6)) && (flagSec == True)) {
	    flagSec = False;
	}

}

void Buzzer_init(void){
	DDRD |= (1<<PD0); //PD0 output
	PORTD &= ~(1<<PD0); //initially off
}

int main(){
	Segment7_init();
	Timer1_CTC_init();
	INT0_init();
	INT1_init();
	INT2_init();
	CountModeDisplay_init();
	PushButtons_init();
	Buzzer_init();

	while(1){
		//check for any button pressed
		PushButtonsCheck();

		// Multiplexed 7Segment
		char i;
		for(i=1; i<=6; i++){
			displaySegment(i);
			_delay_us(5);
		}

	}
}

ISR(INT0_vect){
	sec0 = 0;
	sec1 = 0;
	min0 = 0;
	min1 = 0;
	hour0 = 0;
	hour1 = 0;
	TCNT1 = 0; //start counting from 0
	if(PORTD & (1<<PD0)){
		PORTD &= ~(1<<PD0); //turn off buzzer
		// if alarm is set, press resume stop watch for count up
		CountUpMode = True;
		TCCR1B &= ~(1<<CS12); //stop clock
	}
}

ISR(INT1_vect){
	 TCCR1B &= ~(1<<CS12); //stop clock
	 TCNT1 = 0;
}

ISR(INT2_vect){
	TCCR1B |= (1<<CS12); // resume clock
}

ISR(TIMER1_COMPA_vect){
	if(CountUpMode == True){
		sec0++;
		if(sec0 == 10){
			sec0 = 0;
			sec1++;
		}
		if(sec1 == 6){
			sec1 = 0;
			min0++;
		}
		if(min0 == 10){
			min0 = 0;
			min1++;
		}
		if(min1 == 6){
			min1 = 0;
			hour0++;
		}
		if(hour0 == 10){
			hour0 = 0;
			hour1++;
		}
		if(hour1 == 10){
			sec0 = 0;
			sec1 = 0;
			min0 = 0;
			min1 = 0;
			hour0 = 0;
			hour1 = 0;
		}
	} else if(CountUpMode == False){
		//first check if user didn't set an initial value to count down from
		if((hour1 == 0) && (hour0 == 0) && min1==0 && min0==0 && sec1==0 && sec0==0){
			// do nothing but stop the clock timer
			TCCR1B &= ~(1<<CS12);
		} else{
			sec0--;
			if(sec0 == -1){
				sec0 = 9;
				sec1--;
			}
			if(sec1 == -1){
				sec1 = 5;
				min0--;
			}
			if(min0 == -1){
				min0 = 9;
				min1--;
			}
			if(min1 == -1){
				min1 = 5;
				hour0--;
			}
			if(hour0 == -1){
				hour0 = 9;
				hour1--;
			}
			if(hour1 == -1){
				hour1 = 9;
			}
			if((hour1 == 0) && (hour0 == 0) && min1==0 && min0==0 && sec1==0 && sec0==0){
				// only enable buzzer if timer is ON, to avoid reset turning buzzer always
				if(TCCR1B & (1<<CS12)){
					PORTD |= (1<<PD0); // enable buzzer
				}
				TCCR1B &= ~(1<<CS12); //stop clock
			}
		}

	}



}

void displaySegment(char index){
	switch(index){
		case 1:
			PORTA &= ~(0x2F); // turn off all segments
			PORTC &= ~(0x0F); // clear to assign new value on segment
			PORTC |= sec0;
			PORTA |= (1<<EN1); // enable segment 1 only
			break;
		case 2:
			PORTA &= ~(0x3F); // turn off all segments
			PORTC &= ~(0x0F); // clear to assign new value on segment
			PORTC |= sec1;
			PORTA |= (1<<EN2); // enable segment 2 only
			break;
		case 3:
		    PORTA &= ~(0x3F); // turn off all segments
		    PORTC &= ~(0x0F); // clear to assign new value on segment
		    PORTC |= min0;
		    PORTA |= (1<<EN3); // enable segment 3 only
		    break;
		case 4:
		    PORTA &= ~(0x3F); // turn off all segments
		    PORTC &= ~(0x0F); // clear to assign new value on segment
		    PORTC |= min1;
		    PORTA |= (1<<EN4); // enable segment 4 only
		    break;
		case 5:
		    PORTA &= ~(0x3F); // turn off all segments
		    PORTC &= ~(0x0F); // clear to assign new value on segment
		    PORTC |= hour0;
		    PORTA |= (1<<EN5); // enable segment 5 only
		    break;
		case 6:
		    PORTA &= ~(0x3F); // turn off all segments
		    PORTC &= ~(0x0F); // clear to assign new value on segment
		    PORTC |= hour1;
		    PORTA |= (1<<EN6); // enable segment 6 only
		    break;
		default:
			break;
	}
}
