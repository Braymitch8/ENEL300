/*
 * File:   RF_Testing.c
 * Author: braydenmitchell
 *
 * Created on March 5, 2025, 9:00 AM
 */

#include <avr/io.h>

void setup() {
    // Set PA0 as input with internal pull-up
    PORTA.DIRCLR = (1 << 0); // Configure PA0 as input
    PORTA.PIN0CTRL = PORT_PULLUPEN_bm; // Enable internal pull-up resistor on PA0
    
    // Set PD1 as output
    PORTD.DIRSET = (1 << 1); // Configure PD1 as output
    PORTD.OUTSET = (1 << 1); // Set PD1 high by default
}

void loop() {
    // Check if button is pressed (PA0 low)
    if ((PORTA.IN & (1 << 0)) == 0) { // If PA0 is low (button pressed)
        PORTD.OUTCLR = (1 << 1); // Set PD1 low
    } else {
        PORTD.OUTSET = (1 << 1); // Set PD1 high when button is released
    }
}

int main() {
    setup();
    
    while (1) {
        loop();
    }
}