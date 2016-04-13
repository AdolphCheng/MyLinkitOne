#include <LGPS.h>
#include <LWiFi.h> 
#include <LWiFiClient.h> 

#include "AccountInfo.h"
/*
#define User_API_Key "Your API Ket"
#define SITE_URL "Your Wedsite"
#define GPS_Rest_API "Your Rest API"
#define WIFI_AP "Your AP Name"                                 //Your AP Name
#define WIFI_PWD "Your AP Password"                                //Your AP Password
*/

#define Frequency 30                                        //Period of Sending GPS Information (s)

gpsSentenceInfoStruct info;
LWiFiClient c;

char buff[256];
String latitude_String;
String longitude_String;
char API_Status = 0;
  #define UpdateGPS 0
  #define Idle      1
float Last_Latitude;
float Last_Longitude;
/*
 * 0 = Send POST Request
 * 1 = Idle
*/

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

char parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  double latitude = 0;
  double longitude = 0;
  int tmp, hour, minute, second, num;
  int integet_temp;
  float float_temp;
  
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
    Serial.println(buff);

    //Get Latitude From GPGGA Message
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);

    //Transfer Latitude Formate
    latitude = latitude / 100;
    integet_temp = (int)latitude;
    float_temp = latitude - integet_temp;
    latitude = integet_temp + (float_temp * 100 / 60);
    latitude_String = String(latitude);

    //Get Longtitude From GPGGA Message
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    
    //Transfer Longitude Formate
    longitude = longitude / 100;
    integet_temp = (int)longitude;
    float_temp = longitude - integet_temp;
    longitude = integet_temp + (float_temp * 100 / 60);
    longitude_String = String (longitude);
    
    sprintf(buff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
    Serial.println(buff); 
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "satellites number = %d", num);
    Serial.println(buff); 
  }
  else
  {
    Serial.println("Not get data"); 
  }

  if ((Last_Latitude == latitude) && (Last_Longitude == longitude))
  {
    Serial.println("Do Not Need Update GPS....."); 
    return 0;
  }
  else
  {
    Serial.println("Need Update GPS....."); 
    Last_Latitude = latitude;
    Last_Longitude = longitude;
    return 1;
  }
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
  
  //Init GPS
  LGPS.powerOn();
  Serial.println("LGPS Power on, and waiting ..."); 
  
  //Init WiFi & API
  LWiFi.begin();
  
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
  char Need_Update_GPS;
  
  switch (API_Status){
    case 0: //Get and Send GPS Data to Server
      //Get GPS Data
      Serial.println("");
      Serial.println("********************************** Getting GPS Data **********************************");
      LGPS.getData(&info);
      Serial.println((char*)info.GPGGA); 
      Need_Update_GPS = parseGPGGA((const char*)info.GPGGA);

      //If Do Not Need Update GPS, Keep Idle Status
      if (!Need_Update_GPS)
      {
        API_Status = Idle;
        break;
      }

      //Send GPS Data to Server
      Serial.println(""); 
      Serial.println("********************************** Send GPS Data to Server **********************************");
      // close any connection before send a new request.
      // This will free the socket on the WiFi shield
      c.stop();
      if (c.connect(SITE_URL, 80))
      {
        c.println("POST " GPS_Rest_API "?api_key=" User_API_Key "&lat=" + latitude_String + "&lng=" + longitude_String + " HTTP/1.1"); 
        c.println("Host: " SITE_URL); 
        c.println();
        Serial.println("Done"); 
        digitalWrite(13, HIGH);
        API_Status = Idle;
      }
      else
      {
        Serial.println("Connect is Stop, Retrying....."); 
      }
      delay(1000);
      break;
    case 1:
      Serial.println(""); 
      Serial.println("********************************** Idle **********************************");
      digitalWrite(13, LOW);
      delay(Frequency * 1000);
      Serial.println("********************************** Continue **********************************");
      API_Status = UpdateGPS;
      break;
  }
}



/////////////////////////////////////////////////////////////// Code Buffer ///////////////////////////////////////////////////////////////
/*
      Serial.println("********************************** Server Response **********************************");
      while(c.available()) 
      { 
        v = c.read(); 
        if(v < 0) 
          break; 
        Serial.print((char)v); 
      } 


      Serial.println("********************************** Get GPS Data to Server **********************************");
      c.stop();
      if (c.connect(SITE_URL, 80))
      {
        c.println("GET " GPS_Rest_API "?api_key=" User_API_Key " HTTP/1.1"); 
        c.println("Host: " SITE_URL); 
        c.println();
        digitalWrite(13, HIGH);
        API_Status = 3;
      }
      else
      {
        Serial.println("Connect is Stop, Retrying....."); 
      }
*/
