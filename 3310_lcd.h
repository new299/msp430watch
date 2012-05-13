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

void fillScreen(unsigned char color); // implemented
void draw_char(unsigned char c,unsigned char x,unsigned char y,unsigned char size);
void draw_number(unsigned char number, unsigned char x,unsigned char y,unsigned char size);
void draw_number_binary(unsigned char number, unsigned char x,unsigned char y);

void LCD_send(unsigned char,unsigned char);
void LCDClear(void);
void LCD_init(void);
void LCD_move(unsigned char,unsigned char);

void LCD_out(unsigned char data, unsigned char cd) {

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

void fillScreen(unsigned char color) {
  int i,j;
      
  LCD_out(0x80, SEND_CMD );
  LCD_out(0x40, SEND_CMD );
  
  for (i=0;i<6;i++)  // number of rows
    for (j=0;j<LCD_X_RES;j++) {  // number of columns
      if(color == 0) LCD_out(0x00, SEND_CHR);
      if(color != 0) LCD_out(0xFF, SEND_CHR);
    }
}

void LCD_init(void)
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
  LCD_out( 0x21, SEND_CMD );  // LCD Extended Commands.
  LCD_out( 0xC8, SEND_CMD );  // Set LCD Vop (Contrast).
  LCD_out( 0x06, SEND_CMD );  // Set Temp coefficent to 2.
  LCD_out( 0x13, SEND_CMD );  // LCD bias mode 1:100.
  LCD_out( 0x20, SEND_CMD );  // LCD Standard Commands, Horizontal addressing mode.
  LCD_out( 0x08, SEND_CMD );  // LCD blank
  LCD_out( 0x0C, SEND_CMD );  // LCD in inverse mode.
 
  fillScreen(0); 
}

void lcdcontrast(char c) {
  LCD_out( 0x21, SEND_CMD );  // LCD Extended Commands.
  LCD_out( c, SEND_CMD );  // Set LCD Vop (Contrast).
  LCD_out( 0x20, SEND_CMD );  // LCD Standard Commands, Horizontal addressing mode.
}

void LCD_move(unsigned char x, unsigned char y)
{
	LCD_out(0x80|x,SEND_CMD);
	LCD_out(0x40|y,SEND_CMD);
}

/*
void LCDDot()
{
  int lm;
  LCD_out(0x00,SEND_CHR);
  LCD_out(0x00,SEND_CHR);
  LCD_out(0x00,SEND_CHR);
  for(lm=0;lm<3;lm++)
    LCD_out(0xFF,SEND_CHR);
}
*/

void draw_number(unsigned char number, unsigned char x,unsigned char y,unsigned char size) {

  unsigned char t = number/100;

  draw_char(t,x-(16*size),y,size);
  
  t = (number-(t*100))/10;
  draw_char(t,x-(8*size),y,size);

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

  LCD_move(x,y);
//  LCD_out(0,SEND_CHR);

  unsigned char line_start_bit_pos = bit_pos;
  unsigned char line_start_byte_pos = byte_pos;
 
  unsigned char s = 0;
  unsigned char outline=0;
  unsigned char outline_size=0;
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
        outline = outline << 1;
        outline += 1;
      } else {
        outline = outline << 1;
      }

      outline_size++;
      if(outline_size == 3) {
        LCD_out(outline,SEND_CHR);
        outline=0;
        outline_size=0;
      }

      bit_pos++;
      if(bit_pos==8) {bit_pos=0; byte_pos++;}

      if(((c%3) == 2) && (c>0)) s++;
      if(s==size) s = 0;
  }
//  LCD_out(bitfont[byte_pos],SEND_CHR);
  //LCDDot();
  //LCD_out(0,SEND_CHR);
}
