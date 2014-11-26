#include <msp430x20x3.h>

void i2c_init() {

  DCOCTL = 0;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  // P1OUT = 0xC0; // enable pullups on I2C lines
  // P1REN |= 0xC0;
  P1DIR = 0xFF;
  P2OUT = 0;

  P2DIR = 0xFF;

  USICTL0 = USIPE6+USIPE7+USIMST+USISWRST; // Port & USI mode setup
  USICTL1 = USII2C+USIIE;                  // Enable I2C mode, USI interrupt
  USICKCTL = USIDIV_3+USISSEL_2+USICKPL;   // Setup USI clocks
  USICNT |= USIIFGCC;                      // disable auto-clear
  USICTL0 &= ~USISWRST;                    // enable usi
  USICTL1 &= ~USIIFG;                      // clear pending flag

  _BIS_SR(LPM4_bits+GIE); // enable interrupts
}

void i2c_send(unsigned char a) {

  USISRL = a;

  USICTL1 |= USIIFG;

              P1OUT |= 0x01;           // LED on: sequence start
              USISRL = 0x00;           // Generate Start Condition...
              USICTL0 |= USIGE+USIOE;
              USICTL0 &= ~USIGE;       
              USISRL = 0x11;       // ... and transmit address, R/W = 0
              USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, TX Address

              USISRL = 0x0FF;          // USISRL = 1 to release SDA
              USICTL0 |= USIGE;        // Transparent latch enabled
              USICTL0 &= ~(USIGE+USIOE);// Latch/SDA output disabled



//  USICTL0 |= USIOE;  // set SDA as output
//  USICNT = (USICNT & 0xE0) + 8;
}

int main (void) {
  WDTCTL = WDTPW | WDTHOLD;

  // boot delay
  __delay_cycles(250000);

  P1DIR = 0xFF;
  i2c_init();


  for(;;) {

     i2c_send(0x38);
    __delay_cycles(250000);
  }
  return 0;
}
