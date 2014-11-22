#include <msp430x20x3.h>
int main (void) {
  WDTCTL = WDTPW | WDTHOLD;

  P1DIR = 0xFF;
  for(;;) {
    P1OUT=0;
    __delay_cycles(2500000);
    P1OUT=1;
    __delay_cycles(2500000);
  }
return 0;
}
