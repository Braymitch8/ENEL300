/*
 * File:   servo.c
 * Author: braydenmitchell
 *
 * Created on March 6, 2025, 3:36 PM
 */
#include <avr/io.h>

int main(void) {
  
    // unlocking special registers
    CCP = 0xd8;
    
    // setting the clock to 8MHz
    CLKCTRL.OSCHFCTRLA = 0b00010100;
    
    // Makes sure clock isn't still changing
    while( CLKCTRL.MCLKSTATUS & 0b00000001 ){
        ;
    }
    // enabled. 
    // sets clock freq of timer/counter
    // fTCA = f_clkper / 16 = 500kHz therefore every 2us clock counts up
    TCA0.SINGLE.CTRLA = 0b00001001;
    
    // We will manually check the timer and reset the timer
    // so set the period to its max value to avoid an automatic
    // reset.
    TCA0.SINGLE.PER = 0xffff;

    // set int timer to 1000
    unsigned int timerThreshold = 10000;
    unsigned int small_int = 1000;
    //500 RIGHT 45
    //750 MIDDLE
    //1000 LEFT 45
   
    PORTA.DIRSET = 0b00010000; // set PA4 as output
    while (1) {
        // turns PA4 on
        PORTA.OUT |= 0b00010000;
        
        // while count is less than timer don't do anything
        while( TCA0.SINGLE.CNT <= small_int) ;   
    
        //turns PA4 off
        PORTA.OUT &= 0b11101111;
        
        // while count is less than timer don't do anything
        while( TCA0.SINGLE.CNT <= timerThreshold) ; 
        //Resets timer
        TCA0.SINGLE.CNT = 0;
    }
}
