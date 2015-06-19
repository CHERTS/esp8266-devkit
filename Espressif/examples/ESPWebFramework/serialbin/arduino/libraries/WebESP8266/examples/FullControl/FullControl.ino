/*
  WebESP8266 full control example
  This example demostrates how ESP8266 gets full control of Arduino

  Set ESP8266 UART speed to 9600 using web interface.
  
  This example code is in the public domain.

 */

#include <WebESP8266.h>
#include <SoftwareSerial.h>


// 10 = RX   -> to TX of ESP8266
// 11 = TX   -> to RX of ESP8266 (remember to shift levels from 5.0v to 3.3v!)
SoftwareSerial softSerial(10, 11);

// the WebESP8266 communication object
WebESP8266 webESP; 

void setup()
{
  //while(1);  // debug
  // setup software serial baud rate
  softSerial.begin(9600);
  
  // setup WebESP8266 stream object (in this case it is a soft serial)
  webESP.begin(softSerial);  
}


void loop()
{
  webESP.yield();
}

