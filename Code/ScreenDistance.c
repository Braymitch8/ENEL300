/*
 * File:   ScreenDistance.c
 * Author: braydenmitchell
 *
 * Created on March 19, 2025, 2:20 PM
 */

// This needs to be equal to the CPU clock for the delay functions to work.
// It also needs to be defined before the include statements.
#define F_CPU 16000000UL // 16MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint16_t value[10];
uint16_t count = 0;
uint16_t duty = 4;
uint16_t avg;
uint32_t dist;


// The I2C chip is a PCF8574 with secondary address indicated in Fig 9 of the 
// data sheet.  The last 3 address bits are open jumpers on the back of the 
// device (look closely at it).
#define DISPLAY_ADDR 0x27

/* 
 * This function writes a single byte of data over the I2C bus.
 *   To follow along with this function, read the AVR data sheet sections
 *   29.2, 29.3.1, 29.3.2.1, 29.3.2.2.1 and the entries in 29.4 and 29.5
 *   that explain the SFRs I use below.
 */
void I2CWrite(uint8_t address, uint8_t data) {

  // Set secondary device address and trigger the bus start condition.
  TWI0.MADDR = address << 1;
    
  // Wait until the address has been transmitted out of the data register.
  while( !(TWI0.MSTATUS & 0b01000000) ){
    ;
  }
  
  // Write the data byte.
  TWI0.MDATA = data;
    
  // Wait until the address has been transmitted out of the data register.
  while( !(TWI0.MSTATUS & 0b01000000) ){
    ;
  }
  
  // Send stop condition
  TWI0.MCTRLB = 0x03;
  
  /*
   * Really not sure why I need this delay.  Without it, the display behaves
   * randomly, sometimes working sometimes not.  That *usually* means something
   * is being driven too fast but the data sheets say I'm easily within timing
   * requirements.  Possibly there's a more efficient way to write to the LCD
   * RAM that isn't as fussy but I couldn't find it. 
   */
  _delay_ms(1); // ???
}

/*
 * This function sends a single nibble of data to the LCD in 4 bit mode,
 * sets the RS, R/W lines and turns the display back light on/off.
 *   To follow this section, have a look at the SPLC780D data sheet sections
 *   5.2 to get an idea of some LCD commands and section 6.5.6 to see the
 *   timing requirements for the LCD lines.
 */
void LCDWrite(uint8_t nibble, uint8_t rs, uint8_t rw, uint8_t ledOn){
  
  // rs = 0 puts in command mode
  // rs = 0 puts in data mode
  // Prepare the 8 bits that will be sent over I2C.  The wiring diagram on
  // page 2 of the LCD1602 tutorial shows how these bits are mapped to the 
  // LCD control lines.
  uint8_t data = nibble << 4; // data bits are 4 MSB
  data |= ledOn << 3; // Third bit
  data |= rw << 1; // Second LSB
  data |= rs; // LSB
  
  // Set the data lines, RS and RW.
  I2CWrite(DISPLAY_ADDR,data); // Send 8-bits to I2C
  
  // Pulse E to trigger the SPLC780D to read the data nibble.
  data |= 0b00000100;
  I2CWrite(DISPLAY_ADDR,data);  
  data &= 0b11111011;
  I2CWrite(DISPLAY_ADDR,data);
  
}

// My Functions:
// Requires a number in char type.
// Sends that number to the LCD screen.
void CharWrite(char c){
    uint8_t highNibble = (c >> 4) & 0x0F;
    uint8_t lowNibble = c & 0x0F;
    
    // Send to LCD Screen:
    LCDWrite(highNibble,1,0,1);
    LCDWrite(lowNibble,1,0,1);
    
}

//Requires a pointer to a string.
// Prints that string to the LCD screen.
void StringWrite(char *str){
    for(int i = 0; str[i] != '\0'; i++) {
      CharWrite(str[i]);
    }
}

// Clear the LCD Screen (Only Top Line for Now)
void ClearScreen(void){
  // clear screen:
  LCDWrite(0b0000,0,0,1);
  LCDWrite(0b0001,0,0,1);
}

void NumToString(int num, char *str) {
    char temp[10];  // Temporary buffer to store digits
    int i = 0, j = 0;
    
    if (num == 0) {  // Special case for zero
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {  // Handle negative numbers
        str[j++] = '-';
        num = -num;
    }
    
    // Extract digits from least significant to most significant
    while (num > 0) {
        temp[i++] = (num % 10) + '0';  // Convert digit to character
        num /= 10;
    }

    // Reverse the digits into `str`
    while (i > 0) {
        str[j++] = temp[--i];
    }
    
    str[j] = '\0';  // Null-terminate the string
}


int main(void) {
  
  
  // -- Initialization --

  // Set internal clock frequency, fClk = 16e6 (Hz)
  CCP = 0xd8;
  CLKCTRL.OSCHFCTRLA = 0b00011100;
  while( CLKCTRL.MCLKSTATUS & 0b00000001 ){
      ;
  }
  
  // From the PCF8574 data sheet:
  //   Serial Clock Freq, fSCL = 100e3 (Hz)
  //   Bus Rise Time, Tr = 1e-6 (sec)
  // From the AVR data sheet:
  //   BAUD = fClk/(fSCL*2) - (5+fCLK*Tr/2)
  //        = 67
  TWI0.MBAUD = 67; // essentially a counter counting to 67 ticks of clock
          
  // Enable TWI main.
  TWI0.MCTRLA |= 0x01; // AVR is the main device
  
  // Force bus state machine to idle.
  TWI0.MSTATUS = 0x01; // SEE AVR DATASHEET FOR DEFINITIONS

  
  /*
   * The following init sequence comes from page 16 of the LCD1602
   * data sheet.  Basically, you can't know for sure if the LCD controller
   * will boot up in 8 or 4 bit mode.  So, these three commands are meant to
   * switch it over to 8 bit mode first.  The delays are from the data sheet
   * (with some extra buffer time added).
   */
  
  LCDWrite(0b0011,0,0,1);
  _delay_ms(5);
  LCDWrite(0b0011,0,0,1);
  _delay_ms(5);
  LCDWrite(0b0011,0,0,1);
  _delay_ms(5);
 
  /*
   * The following init sequence is from section 5.5 from the SPLC780D data
   * sheet with an extra clear screen command thrown in.  Check out the 
   * earlier sections of that same data sheet to understand each command
   * in detail.
   */
  
  // Switch from 8 bit to 4 bit mode.
  LCDWrite(0b0010,0,0,1);
    
  // Set to 4 bit operation with 1 display line. 
  LCDWrite(0b0010,0,0,1);
  LCDWrite(0b0000,0,0,1);
  
  // Display on, cursor on, cursor flash off.
  LCDWrite(0b0000,0,0,1);
  LCDWrite(0b1110,0,0,1);
  
  // Cursor increment when write on, display shift off.
  LCDWrite(0b0000,0,0,1);
  LCDWrite(0b0110,0,0,1);
  
  // clear screen
  LCDWrite(0b0000,0,0,1);
  LCDWrite(0b0001,0,0,1);
  
  CCP = 0xd8;                                         // unlocks special registers
  CLKCTRL.OSCHFCTRLA = 0x14;                          // setting the clock to 8MHz (8x10^6)
  while( CLKCTRL.MCLKSTATUS & 0b00000001 ){           // Makes sure clock isn't still changing
      ;
  }
  PORTMUX.TCAROUTEA = 0x02; // generating a PWM waveform using TCA0 and pin PD5, frequency should be 100kHz (100,000Hz)
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
  
  // Main loop
  while (1) {
    ;
  }

  return 0;
}

ISR(TCB2_INT_vect)
{
    TCB2.INTFLAGS = 0x01;  // Clear flag
    TCA0.SINGLE.CMP0 = duty;
    
    value[count] = TCB2.CCMP;  // Store captured value
    count++;
    
    if(count >= 10)
    {
        count = 0;

        // Compute average
        uint32_t sum = 0;
        for(int i = 0; i < 10; i++) {
            sum += value[i];
        }
        avg = sum / 10;

        // Distance formula: (average * 343) / (2 * 8000000)
        uint32_t distance = (avg * 34300) / 16000000;

        // Convert to string
        char distanceStr[10];  // Enough for integer characters
        NumToString(distance, distanceStr);

        // Display on LCD
        ClearScreen();
        StringWrite("Distance: ");
        StringWrite(distanceStr);
        StringWrite(" cm");
        _delay_ms(500);
    }
}
