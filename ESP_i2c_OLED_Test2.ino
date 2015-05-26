#include <ESP8266WiFi.h>
#include <stdio.h>
#include <NeoPixelBus.h>

/* 
 *  Constants
 */

// Wifi
const char* ssid = "Da Haus";
const char* password = "the5thelement";

// NeoPixel
const int kNumPixels = 6;
const int kLedPin = 13;

/*
 * Globals
 */

// Objects
WiFiServer server(80);
NeoPixelBus strip(kNumPixels, kLedPin);

// Variables
boolean lightsOn = true;
boolean webServerStarted = false;

/*
 * Arduino Main Functions
 */

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
    init_OLED();
    sendStrXY("Connecting...", 0, 0);
}

void loop()
{
    // Start the web server if needed
    if (!webServerStarted && WiFi.status() == WL_CONNECTED)
    {
        startWebServer();
    }
    
    // Check for a web request and respond
    handleWebClient();
    
    // Display test sequence to WS2812 LEDs (NeoPixels)
    testSequence();

    // Wait and allow ESP Wifi functions to be performed
    delay(150);
}

void startWebServer()
{
    // Start the web server
    server.begin();
    
    // Display the IP on the OLED screen
    reset_display();
    IPAddress ip = WiFi.localIP();
    char str[15];
    sprintf(str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    sendStrXY(str, 0, 0);
    sendStrXY("----------------", 1, 0);
    sendStrXY("0 clients TEST",   3, 0);
    sendStrXY("Access IP in web", 5, 0);
    sendStrXY("browser for fun.", 6, 0);

    webServerStarted = true;
}

