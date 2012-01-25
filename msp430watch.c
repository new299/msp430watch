#define F_CPU 1000000UL

// select the LCD header file depending on which LCD you want to use.

#include "3310_lcd.h"
//#include "3530_lcd.h"

unsigned char hours=1;
unsigned char mins=1;
unsigned char secs=1;

unsigned char mode1=0;
unsigned char mode2=0;

void check_input() {

  if((P1IN & 0x40) == 0) mode1++;
  if((P1IN & 0x80) == 0) mode2 = 1;
                    else mode2 = 0;

  if(mode1 == 4) mode1 = 0;

  if((mode1 == 1 ) && mode2) { mins++;}
  if((mode1 == 2 ) && mode2) {hours++;}
  if(hours  == 24) hours=0;
  if(mins   == 60) {mins=0;}
}

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

  LCD_init();
  
  fillScreen(0x00);

// BCSCTL1 = CALBC1_1MHZ;
// DCOCTL = CALDCO_1MHZ;

  CCTL0 = CCIE;
  CCR0 = 32767;
  TACTL = TASSEL_1 + MC_1;// + ID_1 + ID_2;
  // Watch crystal is attached
  
  _BIS_SR(GIE);
 

//3310
  P1DIR = 0xFF;
  
  LCD_init();
  
  _BIS_SR(GIE);
 
  unsigned char c = 0;
  while(1) {
// 3310
    draw_number(hours,40,1,1);
    draw_number(mins ,40,2,1);
    draw_number(secs ,40,3,1);

//    draw_number(mode1 ,10,1,1);
//    draw_number(mode2 ,10, 0,1);

//    draw_number_binary(hours ,60,4);
//    draw_number_binary(mins  ,60,5);
//    draw_number_binary(secs  ,60,6);

/* 3530
    draw_number(hours,20,20,2);
    draw_number(mins ,50,20,2);
    draw_number(secs ,80,20,2);

    draw_number(mode1 ,10,10,1);
    draw_number(mode2 ,10, 0,1);

    draw_number_binary(hours ,60,40);
    draw_number_binary(mins  ,60,46);
    draw_number_binary(secs  ,60,52);
*/
    check_input();

   // draw_char(c,60,10,1);
   // if(c==36) c=0; else c++;
  }
}

interrupt(TIMERA0_VECTOR) TimerA_procedure(void) {
  secs++;
  if(secs  ==60)  {secs=0;mins++;}
  if(mins  ==60)  {mins=0;hours++;}
  if(hours ==24)  {hours=0;}
}
