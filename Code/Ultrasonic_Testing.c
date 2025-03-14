/*
 * File:   Ultrasonic_Testing.c
 * Author: braydenmitchell
 *
 * Created on March 7, 2025, 10:28 PM
 */


#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t value[5];
uint16_t count = 0;
uint16_t duty = 4;
uint16_t avg;

int main(void) {
 
    CCP = 0xd8;                                         // unlocks special registers
    CLKCTRL.OSCHFCTRLA = 0x14;                          // setting the clock to 8MHz (8x10^6)
   
    while( CLKCTRL.MCLKSTATUS & 0b00000001 ){           // Makes sure clock isn't still changing\

    }
    
    PORTMUX.TCAROUTEA = 0x02;
    
    // generating a PWM waveform using TCA0 and pin PD5, 
    // frequency should be 100kHz (100,000Hz)
    
    TCA0.SINGLE.CTRLA = (0x01)<<3 | 0x01;               // 500kHz, 2uS/clock
    TCA0.SINGLE.CTRLB = 0x03;                           // single sloped PWM
    TCA0.SINGLE.CTRLB |= 0x01 << 4;
    TCA0.SINGLE.PER = 5-1 ;
    TCA0.SINGLE.CMP0 = duty;                               // 50% duty
    
    PORTA.DIRSET = 0x01;
    PORTC.DIRSET = 0x01; // PC0 output
   
    EVSYS.CHANNEL0 = 0x41; //PA1 input
    EVSYS.USERTCB2CAPT = 0x01;
    TCB2.INTCTRL = 0x01;                                // capture interrupt enabled
    TCB2.EVCTRL = 0x01;                                 // enable input capture event
    TCB2.CTRLB = 0x04;                                  // input capture PW Measurement mode - T = value / 8000000
    //TCB2.CTRLB = 0x03;                                  // input capture frequency Measurement mode - T = value / 8000000
    TCB2.CTRLA = 0x01;                                  // no clock div, TCB enabled
    
    sei();                                              // enable global interrupt
    
    
    while (1) {
    }
    
}

ISR(TCB2_INT_vect)
{
    TCB2.INTFLAGS = 0x01;                               // clear flag
    TCA0.SINGLE.CMP0 = duty;                            // 50% duty
    value[count] = TCB2.CCMP;                           // read counter value
    count++;
    
    if(count >= 5)
    {
        count = 0;
    }
}



// TCA0 for PWM of ultrasonic sensor
// using Trig Pin PD5, Echo Pin PC3