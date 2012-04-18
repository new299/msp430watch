#define F_CPU 1000000UL

#include <io.h>
#include <signal.h>

#define LCD_PORT	P1OUT	//PORTB
#define LCD_DDR		P1DIR	//DDRB
#define LCD_CLK 	BIT1	//(1<<PB0)
#define LCD_SIO		BIT2	//(1<<PB1)
#define LCD_CS		BIT3	//(1<<PB2)
#define LCD_RST		BIT4	//(1<<PB3)

#define red	0b00000111
#define yellow	0b00111111
#define green	0b00111100
#define cyan	0b11111000
#define blue	0b11000000
#define magenta	0b11000111
#define white	0b11111111
#define black	0b00000000

#define page_size	97
#define row_size	67

#define color_bg	white
#define color_fg	black

#define ball_diameter	4

#define paddle_thickness	4
#define paddle_length		15

/**Device Information**
* MPS430 F2013        *
* Power=3.3v          *
**********************/

/**LCD Pinout*****************
* 1=Reset------>3.3v+        *
* 2=CS--------->Logic        *
* 3=GND-------->GND          *
* 4=SIO-------->Logic        *
* 5=SCL-------->Logic        *
* 6=VDigital--->3.3v+        *
* 7=VBoost----->3.3v+        *
* 8=VLCD--->0.22uf Cap-->GND *
*  (This cap may not be      *
*   optimal, other schematics*
*   have suggested 1uf)      *
*****************************/

//LCD_Out function comes from source code found here:
//http://hobbyelektronik.org/Elo/AVR/3510i/index.htm
//Unfortunately this is the only way I know to attribute
//this code to the writer.

static void __inline__ delay(register unsigned int n)
{
    __asm__ __volatile__ (
		"1: \n"
		" dec	%[n] \n"
		" jne	1b \n"
        : [n] "+r"(n));
}

void delay_ms(unsigned int n)
{
  while (n--)
  {
    delay(F_CPU/4000);
  }
}

void LCD_Out(unsigned char Data, unsigned char isCmd) {
    if(isCmd) LCD_PORT |= LCD_CS; 
    LCD_PORT &= ~(LCD_CLK|LCD_CS);  //Clock and CS low

    LCD_PORT |= LCD_SIO;        //SData High
    if(isCmd) LCD_PORT &= ~LCD_SIO; //If it is a command, SData Low

    LCD_PORT |= LCD_CLK;        //Clock High

    for(char x=0; x<8; x++)    {
        LCD_PORT &= ~(LCD_SIO|LCD_CLK);        //Clock and SData low
        if(Data & 128) LCD_PORT |= LCD_SIO;      // Mask MSB - SData high if it is a 1
        LCD_PORT |= LCD_CLK;            //Clock High
        Data=Data<<1;                //Shift bits 1 left (new MSB to be read)
    }
}

void LCD_init(void)
{
  LCD_DDR |= (LCD_CLK | LCD_SIO | LCD_CS | LCD_RST);

  //Hardware Reset
  LCD_PORT &= ~LCD_RST;
  LCD_PORT |= LCD_RST;
  delay_ms(50);

  LCD_PORT |= (LCD_CLK | LCD_SIO | LCD_CS);

  //Software Reset
  LCD_Out(0x01, 1);
  delay_ms(50);


// ORIG COMMENT
  //Refresh set
//  LCD_Out(0xB9, 1);
//  LCD_Out(0x00, 0);


  //Display Control
  LCD_Out(0xB6, 0);
  LCD_Out(128, 0);
  LCD_Out(128, 0);
  LCD_Out(129, 0);
  LCD_Out(84, 0);
  LCD_Out(69, 0);
  LCD_Out(82, 0);
  LCD_Out(67, 0);

/*
  //Temperature gradient set
  LCD_Out(0xB7, 1);
  for(char i=0; i<14; i++)  LCD_Out(0, 0);
*/

  //Booster Voltage On
  LCD_Out(0x03, 1);
  delay_ms(50);  //NOTE: At least 40ms must pass between voltage on and display on.
          //Other operations may be carried out as long as the display is off
          //for this length of time.


/*
  //Test Mode
  LCD_Out(0x04, 1);
*/

//ORIGCOMMENT
  // Power Control
//  LCD_Out(0xBE, 1);
//  LCD_Out(4, 0);


  //Sleep Out
  LCD_Out(0x11, 1);

  //Display mode Normal
  LCD_Out(0x13, 1);

  //Display On
  LCD_Out(0x29, 1);

  //Set Color Lookup Table
  LCD_Out(0x2D, 1);        //Red and Green (3 bits each)
  char x, y;
  
  for(y = 0; y < 2; y++) {
      for(x = 0; x <= 14; x+=2) {
          LCD_Out(x, 0);
      }
  }
  //Set Color Lookup Table    //Blue (2 bits)
  LCD_Out(0, 0);
  LCD_Out(4, 0);
  LCD_Out(9, 0);
  LCD_Out(14, 0);

  //Set Pixel format to 8-bit color codes
  LCD_Out(0x3A, 1);
  LCD_Out(0b00000010, 0);

//***************************************
//Initialization sequence from datasheet:

//Power to chip
//RES pin=low
//RES pin=high -- 5ms pause
//Software Reset
//5ms Pause
//INIESC
  //<Display Setup 1>
    //REFSET
    //Display Control
    //Gray Scale position set
    //Gamma Curve Set
    //Common Driver Output Select
  //<Power Supply Setup>
    //Power Control
    //Sleep Out
    //Voltage Control
    //Write Contrast
    //Temperature Gradient
    //Boost Voltage On
  //<Display Setup 2>
    //Inversion On
    //Partial Area
    //Vertical Scroll Definition
    //Vertical Scroll Start Address
  //<Display Setup 3>
    //Interface Pixel Format
    //Colour Set
    //Memory access control
    //Page Address Set
    //Column Address Set
    //Memory Write
  //Display On

//****************************************

}


void fillScreen(unsigned char color)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(0, 0);
  LCD_Out(97, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(0, 0);
  LCD_Out(66, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (int i=0; i<6566; i++) LCD_Out(color, 0);
}


void draw_char(unsigned char c,unsigned char x,unsigned char y,unsigned char size);
void draw_number(unsigned char number, unsigned char x,unsigned char y,unsigned char size) {

  unsigned char t = number/100;

  draw_char(t,x-(8*size),y,size);
  
  t = (number-(t*100))/10;
  draw_char(t,x-(4*size),y,size);

  t = number%10;
  draw_char(t,x,y,size);

}

void draw_number_binary(unsigned char number, unsigned char x,unsigned char y) {

  for(unsigned char pos=0;pos<8;pos++) {
    if(number & (1<<pos)) {
      draw_char(1,x-(pos*4),y,1);
    } else {
      draw_char(0,x-(pos*4),y,1);
    }
  }
}

void draw_char(unsigned char c,unsigned char x,unsigned char y,unsigned char size) {
  unsigned char bitfont[19] = { 0b11110110 ,0b11011111 ,0b10010010 ,0b01011111 ,0b10011111 ,0b00111111 ,0b00111100 ,0b11111001 ,0b00110111 ,0b01011110 ,0b01110011 ,0b11110100 ,0b11110111 ,0b11110010 ,0b01001001 ,0b11110111 ,0b11011111 ,0b11101111 ,0b00100100 };

/* ARRAY OK
unsigned char bitfont[60] = { 0b11110110 ,0b11011111 ,0b10010010 ,0b01011111 ,0b10011111 ,0b00111111 ,0b00111100 ,0b11111001 ,0b00110111 ,0b01011110 ,0b01110011 ,0b11110100 ,0b11110111 ,0b11110010 ,0b01001001 ,0b11110111 ,0b11011111 ,0b11101111 ,0b00100111 ,0b11011111 ,0b01101110 ,0b10111010 ,0b11101111 ,0b00100100 ,0b11111010 ,0b11011011 ,0b10111100 ,0b11110011 ,0b11111001 ,0b11100100 ,0b11110010 ,0b01011111 ,0b01101111 ,0b10110111 ,0b10100100 ,0b10111111 ,0b01001001 ,0b01101011 ,0b10100110 ,0b10110010 ,0b01001001 ,0b11111111 ,0b10110110 ,0b11011111 ,0b11111101 ,0b11110110 ,0b11011111 ,0b11101111 ,0b10010000 ,0b01111011 ,0b11001101 ,0b10110110 ,0b11111111 ,0b01110101 ,0b10111110 ,0b01110011 ,0b11111010 ,0b01001001 ,0b01011011 ,0b01101111};
*/
/*
unsigned char bitfont[73] = { 0b11110110 ,0b11011111 ,0b10010010 ,0b01011111 ,0b10011111 ,0b00111111 ,0b00111100 ,0b11111001 ,0b00110111 ,0b01011110 ,0b01110011 ,0b11110100 ,0b11110111 ,0b11110010 ,0b01001001 ,0b11110111 ,0b11011111 ,0b11101111 ,0b00100111 ,0b11011111 ,0b01101110 ,0b10111010 ,0b11101111 ,0b00100100 ,0b11111010 ,0b11011011 ,0b10111100 ,0b11110011 ,0b11111001 ,0b11100100 ,0b11110010 ,0b01011111 ,0b01101111 ,0b10110111 ,0b10100100 ,0b10111111 ,0b01001001 ,0b01101011 ,0b10100110 ,0b10110010 ,0b01001001 ,0b11111111 ,0b10110110 ,0b11011111 ,0b11111101 ,0b11110110 ,0b11011111 ,0b11101111 ,0b10010000 ,0b01111011 ,0b11001101 ,0b10110110 ,0b11111111 ,0b01110101 ,0b10111110 ,0b01110011 ,0b11111010 ,0b01001001 ,0b01011011 ,0b01101111 ,0b10110110 ,0b11010101 ,0b01101101 ,0b11101010 ,0b11010101 ,0b01101101 ,0b10101001 ,0b00101110 ,0b01010100 ,0b11100000 };
*/
  if(c>36) c=0;

  unsigned char byte_pos = (15*c)/8;
  unsigned char bit_pos  = (15*c)%8;

  LCD_Out(0x2A, 1);
  LCD_Out(x, 0);
  LCD_Out(x+(3*size)-1, 0);

  LCD_Out(0x2B, 1);
  LCD_Out(y, 0);
  LCD_Out(y+(5*size)-1, 0);

  LCD_Out(0x2C, 1);

  unsigned char line_start_bit_pos = bit_pos;
  unsigned char line_start_byte_pos = byte_pos;
 
  unsigned char s = 0;
  for(unsigned char c=0;c<(15*size);c++) {

      if((s==1) && (size==2) && ((c%3) == 0)) {
        unsigned char t_bit_pos = bit_pos;
        unsigned char t_byte_pos = byte_pos;
        bit_pos=line_start_bit_pos;
        byte_pos=line_start_byte_pos;
        line_start_bit_pos = t_bit_pos;
        line_start_byte_pos = t_byte_pos;
      }

      if(bitfont[byte_pos] & (1<<(7-bit_pos))) {
	LCD_Out(0xFF, 0); //no - draw background in white
        if(size == 2) LCD_Out(0xFF, 0);
      } else {
	LCD_Out(0x00, 0); //yes - draw it in black
        if(size == 2) LCD_Out(0x00, 0); 
      }

      bit_pos++;
      if(bit_pos==8) {bit_pos=0; byte_pos++;}

      if(((c%3) == 2) && (c>0)) s++;
      if(s==size) s = 0;
  }
}

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
  
  unsigned char c = 0;
  while(1) {
    draw_number(hours,20,20,2);
    draw_number(mins ,50,20,2);
    draw_number(secs ,80,20,2);

    draw_number(mode1 ,10,10,1);
    draw_number(mode2 ,10, 0,1);

    draw_number_binary(hours ,60,40);
    draw_number_binary(mins  ,60,46);
    draw_number_binary(secs  ,60,52);

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
#define F_CPU 1000000UL

#include <io.h>
#include <signal.h>

#define LCD_PORT        P1OUT   //PORTB
#define LCD_DDR         P1DIR   //DDRB
#define LCD_CLK         BIT1    //(1<<PB0)
#define LCD_SIO         BIT2    //(1<<PB1)
#define LCD_CS          BIT3    //(1<<PB2)
#define LCD_RST         BIT5    //(1<<PB3)
#define LCD_EN          BIT4    //(1<<PB4)

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
#define GNDPORT P1OUT
#define CARDPORT P1OUT
#define CARD  BIT6
#define GND	 BIT6
#define SCE  BIT5 // LCD pin 5 (enable)
#define SDIN BIT3 // LCD pin 3 (data)
#define DC   BIT4 // LCD pin 4 (Data enabled)
#define SCK  BIT1 // LCD pin 2 (Data clock)
#define RES  BIT2 // LCD pin 8 (reset)

void LCDSend(unsigned char,unsigned char);
void LCDClear(void);
void LCDInit(void);
void LCDBlack(void);
void updateDisplayString(void);
void LCDChar(unsigned char);
void LCDCurs(unsigned char,unsigned char);
void LCDString(unsigned char*,unsigned int, unsigned int, unsigned int);
void updateTime(void);
void LCDDot(void);
void LCDHour(void);

short second, minute, hour, day, subsec = 0;

unsigned char displayString[14] = {
	0x00,0x00,0x0a,0x00,0x00,0x0a,0x00,0x00,0x0a,0x00,0x00,0x0a,0x00,0x00
};

const unsigned char ofYouth[][41]={
{
  0xFC,0xFF,0xFF,0x07,0x03,0x03,0x06,0x1E,0xFC,0xF0,0x80,
  0x00,0x00,0x60,0x30,0x30,0x30,0x30,0xE0,0xE0,0x80,0x00,
  0x70,0xF0,0xF0,0x00,0x00,0x00,0x00,0xF0,0xF0,0x00,0x00,
  0xC0,0xE0,0x20,0x30,0x30,0x70,0x60
},
{
  0x7F,0xFF,0xFF,0xE0,0xC0,0xC0,0xE0,0x70,0x7F,0x3F,0x0F,
  0x00,0x30,0x7C,0x6E,0xC6,0xC2,0xC2,0xE3,0xFF,0xFF,0x00,
  0xE0,0xE1,0x67,0x7F,0x3C,0x1C,0x0F,0x07,0x00,0x00,0x00,
  0x70,0x63,0xC3,0xC7,0xE6,0x7E,0x7C,0x18
}
};

const unsigned char nums[][2][10] ={
 {// 0
   {0xE0,0xFC,0xFE,0x06,0x03,0x03,0x06,0xFE,0xFC,0xE0},
   {0x07,0x3F,0x7F,0x70,0xE0,0xE0,0x70,0x7F,0x3F,0x07}
 },
 {// 1
   {0x00,0x00,0x00,0x04,0x06,0xFE,0xFF,0xFF,0x00,0x00},
   {0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0x00,0x00}
 },
 {// 2
   {0x00,0x00,0x1C,0x0E,0x07,0x83,0x87,0xFE,0xFE,0x38},
   {0x00,0x60,0xFC,0xFE,0xEF,0xE7,0xE3,0xE1,0x60,0x30}
 },
 {// 3
   {0x00,0x00,0x0E,0x0E,0xC6,0xE6,0xF6,0xBE,0x1E,0x06},
   {0x00,0x60,0x70,0xF0,0xE0,0xE0,0xF1,0x7F,0x3F,0x0E}
 },
 {// 4
   {0x00,0xC0,0xE0,0x78,0x1C,0x0E,0x86,0xE2,0x10,0x00},
   {0x06,0x07,0x07,0x06,0x06,0xE6,0xFF,0xFF,0x0E,0x0E}
 },
 {// 5
   {0x00,0x70,0xF8,0xFE,0xCE,0xC6,0xC6,0x86,0x86,0x00},
   {0x00,0x30,0x71,0x71,0xE0,0xE0,0xF0,0x7F,0x3F,0x0E}
 },
 {// 6
   {0xC0,0xF8,0xFC,0x8E,0x86,0xC7,0xC3,0x87,0x82,0x00},
   {0x0F,0x3F,0x7F,0x71,0xE0,0xE0,0x61,0x7B,0x3F,0x0E}
 },
 {// 7
   {0x00,0x00,0x0E,0x0E,0x06,0x86,0xE6,0x7E,0x1E,0x0E},
   {0x00,0x00,0x00,0x38,0x7F,0xFF,0xE1,0x40,0x00,0x00}
 },
 {// 8
   {0x18,0x7E,0xE6,0xC3,0xC3,0xC3,0xE6,0xBE,0x18,0x00},
   {0x18,0x7E,0x7F,0xE1,0xE1,0xE1,0xF3,0x7F,0x7F,0x1C}
 },
 {// 9
   {0xFC,0xCE,0x06,0x03,0x03,0x07,0x86,0xFE,0xFC,0xE0},
   {0x41,0xE3,0xE3,0xE3,0x63,0x73,0x79,0x3F,0x1F,0x03}
 }
};

void updateTime( void)
{
  second = (second -1);
  if ( second == -1 )
  { // new minute
  	second = 59;
    minute = (minute - 1);
    if ( minute == -1)
    { // new hour
      minute = 59;
      hour = (hour -1);
      if ( hour == -1)
      { // new day
      	hour = 23;
      	day = (day -1);
      	displayString[0] = day / 10;
      	displayString[1] = day % 10;
      }
      displayString[3] = hour   / 10;
      displayString[4] = hour   % 10;
    }
    displayString[6] = minute / 10;
    displayString[7] = minute % 10; 
  }
  displayString[9] = second / 10;
  displayString[10] = second % 10;
}

void LCDString(unsigned char *character, unsigned int start, unsigned int layer, unsigned int stop)
{
  int i;
  for(i=start;i<(stop+1);i++)
  {
    int j;
    for (j = 0; j < 10; j++)
    {
      LCDSend(nums[character[i]][layer][j], SEND_CHR);
    }
  }
}

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

lcdcontrast(char c) {
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

#pragma vector=TIMERA0_VECTOR
void Timer_A (void)
{
  subsec = (subsec + 1) % 32;
  if (subsec == 0)
  { // New second
  	updateTime();
  }
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;	// Stop WDT
  P1DIR = 0xFF;
  
  BCSCTL1 = CALBC1_1MHZ;                   // Set range
  DCOCTL = CALDCO_1MHZ;                    // Set DCO step + modulation
  
  BCSCTL2 = 0xF8;
  BCSCTL3 = LFXT1S_0 + XCAP_3;
  
  subsec=0x00;
  second = 0x05;
  minute = 0x00;
  hour = 0x16;
  day = 0x03;
  CCTL0 = CCIE;
  CCR0 = 0;
  TACCR0 = 0x3FF;
  TACTL = 0x0211;
  
  LCDInit();
  
  _BIS_SR(GIE);

  int lm; int lm2;
  for(lm=0;lm<2;lm++)
  {
  	for(lm2=0;lm2<20;lm2++)
  	{
      LCDSend(ofYouth[lm][lm2],SEND_CHR);
  	}
  	LCDCurs(34,4);
  }
for(;;);
}


