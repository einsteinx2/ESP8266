extern "C" {
    #include "spi_flash.h"
}

void handleWebClient() 
{
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) 
    {
        return;
    }
    
    // Wait until the client sends some data
    Serial.println("new client");
    while(!client.available()) 
    {
        delay(1);
    }
    
    // Read the first line of the request
    String req = client.readStringUntil('\r');
    Serial.println(req);
    client.flush();
    
    // Match the request
    int val;
    if (req.indexOf("/test") != -1)
    {
        val = 0;
    }
    else if (req.indexOf("/fun") != -1)
    {
        val = 1;
    }
    else if (req.indexOf("/on") != -1)
    {
        val = 2;
    }
    else if (req.indexOf("/off") != -1)
    {
        val = 3;
    }
    else 
    {
        //Serial.println("invalid request");
        //client.stop();
        //return;
        val = 4;
    }
    
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

// Returns the flash chip's size, in bytes
size_t fs_size() 
{ 
    uint32_t id = spi_flash_get_id();  
    uint8_t mfgr_id = id & 0xff;
    uint8_t type_id = (id >> 8) & 0xff;     // not relevant for size calculation
    uint8_t size_id = (id >> 16) & 0xff;    // lucky for us, WinBond ID's their chips as a form that lets us calculate the size

    // 0xEF is WinBond; that's all we care about (for now)
    return mfgr_id != 0xEF ? 0 : 1 << size_id;
}
