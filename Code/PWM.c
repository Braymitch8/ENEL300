/*
 * File:   PWM.c
 * Author: braydenmitchell
 *
 * Created on February 25, 2025, 2:53 PM
 */


#include <avr/io.h>

int main(void) {
    /* Replace with your application code */
    CCP = 0xd8;
    
    CLKCTRL.OSCHFCTRLA = 0x14;
    while(CLKCTRL.MCLKSTATUS & 0b00000001){
        
    }
    
    PORTMUX.TCAROUTEA = 0x2;
    TCA0.SINGLE.CTRLA = (0x01)<<3 | 0x01;
    TCA0.SINGLE.CTRLB = 0x03;
    TCA0.SINGLE.CTRLB |= 0x01 << 4;
    TCA0.SINGLE.PER = 500;
    TCA0.SINGLE.CMP0 = 500;
    
    PORTC.DIRSET = 0x01;
    
    while (1) {
    }
}
