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
