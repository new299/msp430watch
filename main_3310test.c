#define F_CPU 1000000UL

#include <io.h>
#include <signal.h>

#define SEND_CMD                   0
#define SEND_CHR                   1

#define LCD_X_RES                  84
#define LCD_Y_RES                  48

#define COUNTDOWN				   1

// defines for 4250 pin connections to Nokia 3310
#define SCEPORT P1OUT 
#define SDINPORT P1OUT
#define DCPORT P1OUT
#define SCKPORT P1OUT
#define RESPORT P1OUT
#define SCE  BIT5 // LCD pin 5 (enable)
#define SDIN BIT3 // LCD pin 3 (data)
#define DC   BIT4 // LCD pin 4 (Data enabled)
#define SCK  BIT1 // LCD pin 2 (Data clock)
#define RES  BIT2 // LCD pin 8 (reset)

void LCDSend(unsigned char,unsigned char);
void LCDClear(void);
void LCDInit(void);
void LCDBlack(void);
void LCDCurs(unsigned char,unsigned char);

void LCDSend(unsigned char data, unsigned char cd) {

  volatile unsigned char bits;
  unsigned short cnt=8;
  // assume clk is hi
  // Enable display controller (active low).
  SCEPORT &= ~SCE;  //RESET SCE

  // command or data
  if(cd == SEND_CHR) {
    DCPORT |= DC;  //set to send data
  }
  else {  // reset to send command
    DCPORT &= ~DC;
  }

  ///// SEND SPI /////
  bits=0x80; // bits is mask to select bit to send. select bit msb first
 
  //send data
  while (0<cnt--)
  {
    // put bit on line
    // cycle clock
    SCKPORT &= ~SCK;
    if ((data & bits)>0) SDINPORT |= SDIN; else SDINPORT &= ~SDIN;
    //Delay(1);
    SCKPORT |= SCK;
    //Delay(2);
    // SHIFT BIT MASK 1 right
    bits >>= 1;
  }
   
  // Disable display controller.
  SCEPORT |= SCE;

}

void LCDClear(void) {
  int i,j;
      
  LCDSend(0x80, SEND_CMD );
  LCDSend(0x40, SEND_CMD );
  
  for (i=0;i<6;i++)  // number of rows
    for (j=0;j<LCD_X_RES;j++)  // number of columns
      LCDSend(0x00, SEND_CHR);
}

void LCDInit(void)
{ // assume ports set up and initialized to output

  // Reset LCD
  SCEPORT &= ~SCE;          // RESET SCE to enable 
  // toggle RES
  RESPORT |= RES;           // Set RES
  char l;
  for(l=0;l<100;l++)
    l=l;
  RESPORT &= ~RES;          // reset RES
  for(l=0;l<100;l++)
    l=l;
  RESPORT |= RES;           // Set RES
  
  // Cycle Clock
  SCKPORT &= ~SCK;
  SCKPORT |= SCK;
 
 // Disable display controller.
  SCEPORT |= SCE;           // bring high to disable 
  
  for(l=0;l<100;l++)
    l=l;

  // Send sequence of command
  LCDSend( 0x21, SEND_CMD );  // LCD Extended Commands.
  LCDSend( 0xC8, SEND_CMD );  // Set LCD Vop (Contrast).
  LCDSend( 0x06, SEND_CMD );  // Set Temp coefficent to 2.
  LCDSend( 0x13, SEND_CMD );  // LCD bias mode 1:100.
  LCDSend( 0x20, SEND_CMD );  // LCD Standard Commands, Horizontal addressing mode.
  LCDSend( 0x08, SEND_CMD );  // LCD blank
  LCDSend( 0x0C, SEND_CMD );  // LCD in inverse mode.
  
  LCDClear();

}

void lcdcontrast(char c) {
  LCDSend( 0x21, SEND_CMD );  // LCD Extended Commands.
  LCDSend( c, SEND_CMD );  // Set LCD Vop (Contrast).
  LCDSend( 0x20, SEND_CMD );  // LCD Standard Commands, Horizontal addressing mode.
}

void LCDCurs(unsigned char x, unsigned char y)
{
	LCDSend(0x80|x,SEND_CMD);
	LCDSend(0x40|y,SEND_CMD);
}

void LCDDot()
{
  int lm;
  LCDSend(0x00,SEND_CHR);
  LCDSend(0x00,SEND_CHR);
  LCDSend(0x00,SEND_CHR);
  for(lm=0;lm<3;lm++)
    LCDSend(0xFF,SEND_CHR);
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;	// Stop WDT
  P1DIR = 0xFF;
  
  BCSCTL1 = CALBC1_1MHZ;                   // Set range
  DCOCTL = CALDCO_1MHZ;                    // Set DCO step + modulation
  
  BCSCTL2 = 0xF8;
  BCSCTL3 = LFXT1S_0 + XCAP_3;
  
  CCTL0 = CCIE;
  CCR0 = 0;
  TACCR0 = 0x3FF;
  TACTL = 0x0211;
  
  LCDInit();
  
  _BIS_SR(GIE);

  // draw cross pattern
  char v = 0xAA;
  for(int x=0;x<((84*48)/8);x++) {
 //   if(x%48==0) {
 //   }

    LCDSend(v,SEND_CHR);
    v = v ^ 0xFF;
  }
  for(;;);
}


