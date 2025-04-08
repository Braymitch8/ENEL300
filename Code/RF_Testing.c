/*
 * File:   RFReciever.c
 * Author: braydenmitchell
 *
 * Created on March 23, 2025, 3:29 PM
 */


//(UP) turns on RC PD3 using RF1
//(DOWN) turns on RC PD4 using RF1
//(LEFT) turns on RC PD2 using RF1
//(RIGHT) turns on RC PD1 using RF1


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
    // sets clock frequency of timer/counter
    // fTCA = f_clock per / 16 = 500kHz therefore every 2us clock counts up
    TCA0.SINGLE.CTRLA = 0b00001001;
    
    // We will manually check the timer and reset the timer
    // so set the period to its max value to avoid an automatic
    // reset.
    TCA0.SINGLE.PER = 0xffff;

    // set timer to 1000
    unsigned int timermax = 10000;
    //440 RIGHT 45
    //640 MIDDLE
    //840 LEFT 45
   
    PORTA.DIRSET = 0b00100000; // set PA5 as output
    
    
    while (1) {
        
        if (PORTD.IN & (1 << 1)) {  // PD1 is pressed (high) TURN RIGHT
            PORTA.OUT |= 0b00100000;
            while( TCA0.SINGLE.CNT <= 440) ; 
            PORTA.OUT &= 0b11011111;
            while( TCA0.SINGLE.CNT <= timermax) ; 
            TCA0.SINGLE.CNT = 0;
        }
        else if (PORTD.IN & (1 << 2)) {  // PD2 is pressed (high) TURN LEFT
            PORTA.OUT |= 0b00100000;
            while( TCA0.SINGLE.CNT <= 840) ; 
            PORTA.OUT &= 0b11011111;
            while( TCA0.SINGLE.CNT <= timermax) ; 
            TCA0.SINGLE.CNT = 0;
        }
        else{
            PORTA.OUT |= 0b00100000;
            while( TCA0.SINGLE.CNT <= 640) ; 
            PORTA.OUT &= 0b11011111;
            while( TCA0.SINGLE.CNT <= timermax) ; 
            TCA0.SINGLE.CNT = 0;
        }
        
    }

}
