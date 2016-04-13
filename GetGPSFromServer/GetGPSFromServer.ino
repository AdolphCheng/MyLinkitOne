#include <LWiFi.h> 
#include <LWiFiClient.h> 
#include <LFlash.h>

#include "AccountInfo.h"
/*
#define User_API_Key "Your API Key"
#define SITE_URL "Your Website"
#define GPS_Rest_API "Your Rest API"
#define WIFI_AP "Your AP Name"                              //Your AP Name
#define WIFI_PWD "Your AP Password"                         //Your AP Password
 */
 
#define Frequency 10                                        //Period of Get GPS Information From Web(s)
#define Drv LFlash                                          //Use Internal 10M Flash

LWiFiClient c;
LFile myFile;

//Global Var
char buff[256];
char Counter = 0;
char Latitude_String[6];
char Longitude_String[6];

char Store_Latitude(char v, char Is_Latitude)
{
    switch (Is_Latitude)
    {
    case 0:
      if (v == 108)
        Is_Latitude = 1;
      else
        Is_Latitude = 0;
      break;
    case 1:
      if (v == 97)
        Is_Latitude = 2;
      else
        Is_Latitude = 0;
      break;
    case 2:
      if (v == 116)
        Is_Latitude = 3;
      else
        Is_Latitude = 0;
      break;
    case 3:
      if (v == 34)
        Is_Latitude = 4;
      else
        Is_Latitude = 0;
      break;    
    case 4:
      if (v == 58)
      {
        Is_Latitude = 5;
      }
      else
        Is_Latitude = 0;
      break;
    case 5:
      if (v == 44)
      {
        for (Counter=0; Counter<=6; Counter++)
        {
          if (Latitude_String[Counter] != NULL)
          {
            myFile.print((char)Latitude_String[Counter]);
            Serial.print((char)Latitude_String[Counter]); 
          }
        }
        Serial.print(","); 
        myFile.print(",");
        Is_Latitude = 0;
        Counter = 0;
      }
      else
      {
        Latitude_String[Counter] = (char)v;
        Counter++;
      }
      break;
    }
    return Is_Latitude;
}

char Store_Longitude(char v, char Is_Longitude)
{
    switch (Is_Longitude)
    {
    case 0:
      if (v == 108)
        Is_Longitude = 1;
      else
        Is_Longitude = 0;
      break;
    case 1:
      if (v == 110)
        Is_Longitude = 2;
      else
        Is_Longitude = 0;
      break;
    case 2:
      if (v == 103)
        Is_Longitude = 3;
      else
        Is_Longitude = 0;
      break;
    case 3:
      if (v == 34)
        Is_Longitude = 4;
      else
        Is_Longitude = 0;
      break;    
    case 4:
      if (v == 58)
      {
        Is_Longitude = 5;
      }
      else
        Is_Longitude = 0;
      break;
    case 5:
      if (v == 44)
      {
        for (Counter=0; Counter<=6; Counter++)
        {
          if (Longitude_String[Counter] != NULL)
          {
            myFile.print((char)Longitude_String[Counter]);
            Serial.print((char)Longitude_String[Counter]); 
          }
        }
        Serial.println(""); 
        myFile.println("");
        Is_Longitude = 0;
        Counter = 0;
      }
      else
      {
        Longitude_String[Counter] = (char)v;
        Counter++;
      }
      break;
    }
    return Is_Longitude;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  /*
  //Start setup() After User Open Serial Monirot Window
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  */
  
  //Init LED
  pinMode(13, OUTPUT);

  //Init WiFi & API
  LWiFi.begin();

  //Init Embeddded Flash
  Drv.begin();
  
  //Connect to AP
  Serial.print("Connecting to AP...");
  while (!LWiFi.connectWPA(WIFI_AP, WIFI_PWD))
  { 
    Serial.println("");
    Serial.print("Retry Connect to AP...");
    delay(1000); 
  } 
  Serial.println("OK!"); 
}

void loop() {
  // put your main code here, to run repeatedly:
  int v;
  char Is_Latitude = 0;
  char Is_Longitude = 0;
  
  Serial.println("********************************** Get GPS Data to Server **********************************");
  c.stop();
  if (c.connect(SITE_URL, 80))
  {
    c.println("GET " GPS_Rest_API "?api_key=" User_API_Key "&num=1 HTTP/1.1"); 
    c.println("Host: " SITE_URL); 
    c.println();
    digitalWrite(13, HIGH);
  }
  else
  {
    Serial.println("Connect is Stop, Retrying....."); 
  }
  delay(1000);
  Serial.println("********************************** Server Response **********************************");
  myFile = Drv.open("test.txt", FILE_WRITE);
  while(c.available()) 
  { 
    v = c.read(); 
    if(v < 0) 
      break; 
    //Serial.print((char)v);
    
    Is_Latitude = Store_Latitude(v, Is_Latitude);
    Is_Longitude = Store_Longitude(v, Is_Longitude);
  }
  myFile.close();   //Close The File
  Serial.println("********************************** Idle **********************************");
  digitalWrite(13, LOW);
  delay(Frequency * 1000);
  Serial.println("********************************** Continue **********************************");
}
