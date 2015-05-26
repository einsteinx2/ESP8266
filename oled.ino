#include <Wire.h>
#include "oled_data.h"

// OLED I2C bus address
#define OLED_address  0x3c

// Resets display depending on the actual mode.
static void reset_display(void)
{
    displayOff();
    clear_display();
    
    displayOn();
}

// Turns display on.
void displayOn(void)
{
    sendcommand(0xaf);
}

// Turns display off.
void displayOff(void)
{
    sendcommand(0xae);
}

// Clears the display by sendind 0 to all the screen map.
static void clear_display(void)
{
    unsigned char i,k;
    for(k = 0; k < 8; k++)          //8
    { 
        setXY(k, 0);
        
        for(i = 0; i < 128; i++)    //was 128
        {
            SendChar(0);            //clear all COL
        }
    }
}

// Actually this sends a byte, not a char to draw in the display. 
// Display's chars uses 8 byte font the small ones and 96 bytes
// for the big number font.
static void SendChar(unsigned char data) 
{ 
    Wire.beginTransmission(OLED_address);

    // Set data mode
    Wire.write(0x40);

    // Write data
    Wire.write(data);

    Wire.endTransmission();
}

// Prints a display char (not just a byte) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15) 
// and 8 ROWS (0-7).
static void sendCharXY(unsigned char data, int X, int Y)
{
    setXY(X, Y);

    
    Wire.beginTransmission(OLED_address);

    // Set data mode
    Wire.write(0x40);

    // Write data
    for(int i = 0; i < 8; i++)  //8
    {
        Wire.write(pgm_read_byte(myFont[data - 0x20] + i));
    }
    
    Wire.endTransmission();
}

// Used to send commands to the display.
static void sendcommand(unsigned char com)
{
    Wire.beginTransmission(OLED_address);

    // Set command mode
    Wire.write(0x80);

    // Send command
    Wire.write(com);

    Wire.endTransmission();
}

// Set the cursor position in a 16 COL * 8 ROW map.
static void setXY(unsigned char row, unsigned char col)
{
    // Set the page address
    sendcommand(0xb0 + row);

    // Set low collumn address  //8
    sendcommand(0x00 + (8 * col & 0x0f));    

    // Set high collumn address  //8
    sendcommand(0x10 + ((8 * col >> 4) & 0x0f));    
}

// Prints a string regardless the cursor position.
static void sendStr(unsigned char const *string)
{
    unsigned char i = 0;
    
    while(*string)
    {
        for(i = 0; i < 8; i++)  
        {
            SendChar(pgm_read_byte(myFont[*string - 0x20] + i));
        }
        *string++;
    }
}

//==========================================================//
// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
static void sendStrXY( char const *string, int X, int Y)
{
    setXY(X, Y);
    unsigned char i = 0;
    while(*string)
    {
        for(i = 0; i < 8; i++)  
        {
            SendChar(pgm_read_byte(myFont[*string - 0x20] + i));
        }
        *string++;
    }
}


//==========================================================//
// Inits oled and draws logo at startup
static void init_OLED(void)
{
    Wire.begin(14, 12); //14=SDA, 12=SCL
  
    sendcommand(0xae);          //display off
    sendcommand(0xa6);          //Set Normal Display (default) 
    
    // Adafruit Init sequence for 128x64 OLED module
    sendcommand(0xAE);          //DISPLAYOFF
    sendcommand(0xD5);          //SETDISPLAYCLOCKDIV
    sendcommand(0x80);          // the suggested ratio 0x80
    //sendcommand(0xA8);        //SSD1306_SETMULTIPLEX  // This line must be commented out or it's impossible to write to the top yellow area of the 2 color OLED
    sendcommand(0x2F);          //--1/48 duty    //NEW!!!
    sendcommand(0xD3);          //SETDISPLAYOFFSET
    sendcommand(0x0);           //no offset
    sendcommand(0x40 | 0x0);    //SETSTARTLINE
    sendcommand(0x8D);          //CHARGEPUMP
    sendcommand(0x14);
    sendcommand(0x20);          //MEMORYMODE
    sendcommand(0x00);          //0x0 act like ks0108
    
    sendcommand(0xA0 | 0x1);    //SEGREMAP   //Rotate screen 180 deg
    //sendcommand(0xA0);
    
    sendcommand(0xC8);          //COMSCANDEC  Rotate screen 180 Deg
    //sendcommand(0xC0);
    
    sendcommand(0xDA);          //0xDA
    sendcommand(0x12);          //COMSCANDEC
    sendcommand(0x81);          //SETCONTRAS
    sendcommand(0xCF);          //
    sendcommand(0xd9);          //SETPRECHARGE 
    sendcommand(0xF1); 
    sendcommand(0xDB);          //SETVCOMDETECT                
    sendcommand(0x40);
    sendcommand(0xA4);          //DISPLAYALLON_RESUME        
    sendcommand(0xA6);          //NORMALDISPLAY             

    clear_display();
    sendcommand(0x2e);            // stop scroll
  
    //----------------------------REVERSE comments----------------------------//
    //  sendcommand(0xa0);        //seg re-map 0->127(default)
    //  sendcommand(0xa1);        //seg re-map 127->0
    //  sendcommand(0xc8);
    //  delay(1000);
    //----------------------------REVERSE comments----------------------------//
    // sendcommand(0xa7);         //Set Inverse Display  
    // sendcommand(0xae);         //display off
    
    sendcommand(0x20);            //Set Memory Addressing Mode
    sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
    
    //  sendcommand(0x02);        // Set Memory Addressing Mode ab Page addressing mode(RESET)  
  
   setXY(0,0);
   
    /*
    for(int i = 0; i < 128 * 8; i++)     // show 128* 64 Logo
    {
        SendChar(pgm_read_byte(logo + i));
    }
    */
    
    sendcommand(0xaf);        //display on

    reset_display();           //Clear logo and load saved mode
}



