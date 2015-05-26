// Import required libraries
#include <ESP8266WiFi.h>
#include <stdio.h>
#include <NeoPixelBus.h>

// we need to include these to be able to use the spi_flash_get_id() function
extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "os_type.h"
}

#include "Esp.h"

size_t fs_size() { // returns the FLASH chip's SIZE , in BYTES
  uint32_t id = spi_flash_get_id();  
  uint8_t mfgr_id = id & 0xff;
  uint8_t type_id = (id >> 8) & 0xff; // not relevant for SIZE calculation
  uint8_t size_id = (id >> 16) & 0xff; // lucky for us, WinBond ID's their chips as a form that lets us calculate the SIZE

  if(mfgr_id != 0xEF) // 0xEF is WinBond; that's all we care about (for now)
    return 0;
  return 1 << size_id;
}




// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);


//This code is a mess right now but does show something on the display
//Oled Display i'm using has an i2c address of 0x0C
//ESP-01 has only two I/O pins, 0=SDA, 2=SCL


//==========================================================//
//                                                          //
//    OLED Test Code                                        //                    
//                                                          //
//==========================================================//


#include <Wire.h>

//---------------FONT + GRAPHIC-----------------------------//
#include "data.h"
//==========================================================//

// OLED I2C bus address
#define OLED_address  0x3c

// WiFi parameters
const char* ssid = "Da Haus";
const char* password = "the5thelement";

boolean isConnected = false;

RgbColor led_colors[6] = { RgbColor(128, 0, 0), RgbColor(0, 128, 0), RgbColor(0, 0, 128), 
                           RgbColor(128, 0, 0), RgbColor(0, 128, 0), RgbColor(0, 0, 128) };
                                    
NeoPixelBus strip = NeoPixelBus(6, 13);

boolean lightsOn = true;

void setup()
{
  // Start Serial
  Serial.begin(115200);
 
  // Connect to WiFi
  WiFi.begin(ssid, password);
   
  // Initialize WS8212 (Pin 13)
  strip.Begin();
  strip.Show();
  
  // Initialize I2C and OLED Display
  Wire.pins(14, 12); //14=SDA, 12=SCL
  Wire.begin();
  init_OLED();
  reset_display();           // Clear logo and load saved mode
}


//==========================================================//
void loop()
{  

 /*
//   for(int i=0;i<128*8;i++)     // show 128* 64 Logo
//     SendChar(pgm_read_byte(logo+i));
//    delay(8000);
//    displayOn();
//    clear_display();

//   while(1)
//   {
//  
//    sendStrXY("Oled on",3,0);
//    sendStrXY("ESP8266 only!",4,0);
//    delay(4000);
//    reset_display();
//    clear_display();
//  
//    delay(1000);
//    reset_display();
//    
//   }      
*/

  while (WiFi.status() != WL_CONNECTED) 
  {
      sendStrXY("Connecting...", 0, 0);
      delay(500);
  }
  
  if (!isConnected)
  {
      isConnected = true;
      
      // Start the server
      server.begin();
      Serial.println("Server started");
      
      // Display the IP
      reset_display();
      IPAddress ip = (uint32_t)WiFi.localIP();
      char str[15];
      sprintf(str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      sendStrXY(str, 0, 0);
      sendStrXY("----------------", 1, 0);
      
      sendStrXY("0 clients TEST", 3, 0);
      sendStrXY("Access IP in web", 5, 0);
      sendStrXY("browser for fun.", 6, 0);
  }
  
  handle_web_client();
  
  // WS8212
  //static boolean turnOn = true;
  static int offset = 0;

  if (lightsOn)
  {
    for (int i = 0; i < 6; i++)
    {
        int index = i + offset;
        if (index >= 6)
        {
            index = index - 6;
        }
        
        strip.SetPixelColor(i, led_colors[index]);
    }
  }
  else
  {
    for (int i = 0; i < 6; i++)
    {
        strip.SetPixelColor(i, RgbColor(0, 0, 0));
    }
  }
  
  strip.Show();
  
  offset++;
  if (offset >= 6)
  {
    offset = 0;
  }
    
  delay(150);
}

void handle_web_client() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int val;
  if (req.indexOf("/test") != -1)
    val = 0;
  else if (req.indexOf("/fun") != -1)
    val = 1;
  else if (req.indexOf("/on") != -1)
    val = 2;
  else if (req.indexOf("/off") != -1)
    val = 3;
  else {
//    Serial.println("invalid request");
//    client.stop();
//    return;
    val = 4;
  }

  // Set GPIO2 according to the request
  //digitalWrite(2, val);
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html> ";
  
  switch(val)
  {
     case 0:
        s += "\r\nYou've reached the test page"; 
        break;
     case 1:
        s += "\r\nYou've reached the fun page"; 
        break;
     case 2:
        s += "\r\nLights turned on!"; 
        lightsOn = true;
        break;
     case 3:
        s += "\r\nLights turned off!"; 
        lightsOn = false;
        break;
     case 4:
        s += "\r\nRequest:\r\n" + req + "page"; 
        break;
  }
  
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}


//==========================================================//
// Resets display depending on the actual mode.
static void reset_display(void)
{
  displayOff();
  clear_display();

  displayOn();
}




//==========================================================//
// Turns display on.
void displayOn(void)
{
    sendcommand(0xaf);        //display on
}

//==========================================================//
// Turns display off.
void displayOff(void)
{
  sendcommand(0xae);		//display off
}

//==========================================================//
// Clears the display by sendind 0 to all the screen map.
static void clear_display(void)
{
  unsigned char i,k;
  for(k=0;k<8;k++)  //8
  {	
    setXY(k,0);    
    {
      for(i=0;i<128;i++)     //was 128
      {
        SendChar(0);         //clear all COL
      }
    }
  }
}



//==========================================================//
// Actually this sends a byte, not a char to draw in the display. 
// Display's chars uses 8 byte font the small ones and 96 bytes
// for the big number font.
static void SendChar(unsigned char data) 
{ 
  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode
  Wire.write(data);
  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Prints a display char (not just a byte) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15) 
// and 8 ROWS (0-7).
static void sendCharXY(unsigned char data, int X, int Y)
{
  setXY(X, Y);
  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode
  
  for(int i=0;i<8;i++)  //8
    Wire.write(pgm_read_byte(myFont[data-0x20]+i));
    
  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Used to send commands to the display.
static void sendcommand(unsigned char com)
{
  Wire.beginTransmission(OLED_address);     //begin transmitting
  Wire.write(0x80);                          //command mode
  Wire.write(com);
  Wire.endTransmission();                    // stop transmitting
}

//==========================================================//
// Set the cursor position in a 16 COL * 8 ROW map.
static void setXY(unsigned char row,unsigned char col)
{
  sendcommand(0xb0+row);                //set page address
  sendcommand(0x00+(8*col&0x0f));       //set low col address  //8
  sendcommand(0x10+((8*col>>4)&0x0f));  //set high col address  //8
  
}


//==========================================================//
// Prints a string regardless the cursor position.
static void sendStr(unsigned char const *string)
{
  unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)  
    {
      SendChar(pgm_read_byte(myFont[*string-0x20]+i));
    }
    *string++;
  }
}

//==========================================================//
// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
static void sendStrXY( char const *string, int X, int Y)
{
  setXY(X,Y);
  unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)  
    {
     SendChar(pgm_read_byte(myFont[*string-0x20]+i));
    }
    *string++;
  }
}


//==========================================================//
// Inits oled and draws logo at startup
static void init_OLED(void)
{
    sendcommand(0xae);		//display off
    sendcommand(0xa6);            //Set Normal Display (default) 
    // Adafruit Init sequence for 128x64 OLED module
    sendcommand(0xAE);             //DISPLAYOFF
    sendcommand(0xD5);            //SETDISPLAYCLOCKDIV
    sendcommand(0x80);            // the suggested ratio 0x80
    //sendcommand(0xA8);            //SSD1306_SETMULTIPLEX  // This line must be commented out or it's impossible to write to the top yellow area of the 2 color OLED
    sendcommand(0x2F); //--1/48 duty    //NEW!!!
    sendcommand(0xD3);            //SETDISPLAYOFFSET
    sendcommand(0x0);             //no offset
    sendcommand(0x40 | 0x0);      //SETSTARTLINE
    sendcommand(0x8D);            //CHARGEPUMP
    sendcommand(0x14);
    sendcommand(0x20);             //MEMORYMODE
    sendcommand(0x00);             //0x0 act like ks0108
    
    sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg
    //sendcommand(0xA0);
    
    sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg
    //sendcommand(0xC0);
    
    sendcommand(0xDA);            //0xDA
    sendcommand(0x12);           //COMSCANDEC
    sendcommand(0x81);           //SETCONTRAS
    sendcommand(0xCF);           //
    sendcommand(0xd9);          //SETPRECHARGE 
    sendcommand(0xF1); 
    sendcommand(0xDB);        //SETVCOMDETECT                
    sendcommand(0x40);
    sendcommand(0xA4);        //DISPLAYALLON_RESUME        
    sendcommand(0xA6);        //NORMALDISPLAY             

  clear_display();
  sendcommand(0x2e);            // stop scroll
  //----------------------------REVERSE comments----------------------------//
  //  sendcommand(0xa0);		//seg re-map 0->127(default)
  //  sendcommand(0xa1);		//seg re-map 127->0
  //  sendcommand(0xc8);
  //  delay(1000);
  //----------------------------REVERSE comments----------------------------//
  // sendcommand(0xa7);  //Set Inverse Display  
  // sendcommand(0xae);		//display off
  sendcommand(0x20);            //Set Memory Addressing Mode
  sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
  //  sendcommand(0x02);         // Set Memory Addressing Mode ab Page addressing mode(RESET)  
  
   setXY(0,0);
  /*
  for(int i=0;i<128*8;i++)     // show 128* 64 Logo
  {
    SendChar(pgm_read_byte(logo+i));
  }
  */
  sendcommand(0xaf);		//display on
}



