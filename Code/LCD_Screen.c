// This needs to be equal to the CPU clock for the delay functions to work.
// It also needs to be defined before the include statements.
#define F_CPU 16000000UL // 16MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


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
// rs = 0 puts in command mode
// rs = 0 puts in data mode
void LCDWrite(uint8_t nibble, uint8_t rs, uint8_t rw, uint8_t ledOn){
  
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

// Requires a integer number
// Stores that number in a given string
void NumToString(int num, char *str){} // TO BE COMPLETED


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
  
  
  // Write 'W'.
  //LCDWrite(0b0101,1,0,1); // Send the high byte
  //LCDWrite(0b0111,1,0,1); // Send the low byte
  
  //LCDWrite(0b0000,0,0,1);
  //LCDWrite(0b0110,0,0,1);
  
  // PRINTING "ALEX"
  // In order to print a character, you must send the high byte of the ascii encoding, followed by the low byte.
//  LCDWrite(0x04,1,0,1);
//  LCDWrite(0x01,1,0,1);
//  
//  LCDWrite(0b0000,0,0,1);
//  LCDWrite(0b0110,0,0,1);
//  
//  LCDWrite(0x04,1,0,1);
//  LCDWrite(0x0c,1,0,1);
//  
//  
//  LCDWrite(0b0000,0,0,1);
//  LCDWrite(0b0110,0,0,1);
//  
//  LCDWrite(0x04,1,0,1);
//  LCDWrite(0x05,1,0,1);
//
//  LCDWrite(0b0000,0,0,1);
//  LCDWrite(0b0110,0,0,1);
//  
//  LCDWrite(0x05,1,0,1);
//  LCDWrite(0x08,1,0,1);
  
  // TESTING NumWrite FUNCTION
  CharWrite('A');
  CharWrite('3');
  CharWrite('42');
  
  char str[] = "Bro";
  
  for(int i = 0; str[i] != '\0'; i++) {
      CharWrite(str[i]);
  }
  
  StringWrite("HEYHEY");
  ClearScreen();
  StringWrite("Distance is: ");

  
  // Main loop
  while (1) {
    ;
  }

  return 0;
}

