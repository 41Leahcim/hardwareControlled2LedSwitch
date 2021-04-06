/*
 * hardwareControlled2LedSwitch.c
 *
 * Created: 6-4-2021 13:25:42
 * Author : Michael Scholten
 * microcontroller: AVR ATmega 328P XPLAINED mini
 * other hardware used: 2 LED's, 1 button, 1 potmeter, 2 220 ohm resistors, 6 jumperwires and 2 breadboards
 */

// import io variables from avr/io.h and interrupt variables and functions from avr/interrupt.h
#include <avr/io.h>
#include <avr/interrupt.h>

// stores whether the button has been pressed (1) and whether the button timer has overflowed.
uint8_t buttonFlags = 0;

void initExternalInterruptPD2(){
	// generate an interrupt at falling edge at INT0 (PD2)
	EICRA |= (1 << ISC01);
	EIMSK |= (1 << INT0);
}

void initADC(){
	// set the referance voltage to AVcc, the channel to 5 and activate ADLAR so we only need to get 8 bits
	ADMUX |= (1 << REFS0) | 5 | (1 << ADLAR);
	
	// enable the converter, start a conversion, enable the ADC interrupt, prescale by 1024
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (7 << ADPS2);
}

void initPWMTimer(){
	// set OC0A (PD6) and OC0B (PD5) at bottom, clear on compare match, enable PWM and don't prescale
	TCCR0A = (1 << COM0A1) | (3 << WGM00) | (1 << COM0B1);
	TCCR0B = (1 << CS00);
}

void initButtonTimer(){
	// set prescaler to 1024
	TCCR0B |= (5 << CS10);
	
	// set an overflow interrupt for this timer
	TIMSK0 |= (1 << TOIE0);
}

ISR(INT0_vect){
	// if the time since start/end of last press has overflowed (more than 256 / (16000000 / 1024) = ~16 milliseconds have passed)
	if((buttonFlags & 2) == 2){
		// if the button was not pressed
		if((buttonFlags & 1) == 0){
			// toggle PORTD5 and PORTD6 to change which of those is on
			DDRD ^= (3 << PORTD5);
		}
		// reset the timer
		TCNT1 = 0;
		
		// Store in the buttonFlags variable, whether the button has been pressed or released.
		// We set the timer overflow flag to 0, so we know when the timer overflows again
		buttonFlags = (~buttonFlags) & 1;
	}
}

ISR(ADC_vect){
	// set the Output Compare Registers of OC0A (PD5) and OC0B (PD6) to the value in the ADC
	OCR0A = ADCH;
	OCR0B = ADCH;
	
	// start new conversion
	ADCSRA |= (1 << ADSC);
}

ISR(TIMER0_OVF_vect){
	// set the timer overflow flag in the buttonFlags variable
	buttonFlags |= 2;
}

int main(void){
	// set PORTD2 as input and PORTD3 as output
	DDRD = (DDRD & ~(1 << PORTD2)) | (1 << PORTD5);
	
	// enable the pull up resistor at PD2
	PORTD |= (1 << PORTD2);
	
	// initialize the external interrupt at PD2 (INT1)
	initExternalInterruptPD2();
	
	// initialize and start the ADC with interrupt
	initADC();
	
	// initialize the PWM timer (timer0)
	initPWMTimer();
	
	// initialize the button timer (timer1)
	initButtonTimer();
	
	// start all interrupts
	sei();
	
    while (1);
}

