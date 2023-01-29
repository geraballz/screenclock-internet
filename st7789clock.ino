/**************************************************************************
 * 
 * WiFi Internet clock (NTP) with ESP8266 NodeMCU board and
 *   ST7789 TFT display.
 * This is a free software with NO WARRANTY.
 *
 *************************************************************************/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>        // include NTPClient library
#include <TimeLib.h>          // Include Arduino time library
#include <Adafruit_GFX.h>     // include Adafruit graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "DHT.h"
#include "bitmaps.h"
#include "st7789clockBK.h"
#include "Elianto_Regular20.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define TFT_RST   D1   
#define TFT_CS    D8    
#define TFT_DC    D0 
#define DHTPin    D2
#define BuzzerPin D6
#define DHTTYPE DHT11 // DHT 11
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

DHT dht(DHTPin, DHTTYPE);

float temperature;
float humidity; 
long tiempoUltimaLectura=0;
String lastDay = "";
long lastDate = 0;
long lastH = 0;
long lastM = 0;
long lastS = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.nist.gov", -21600);//timezone(-5)*60*60
unsigned long unix_epoch;

void setup(void)
{
  Serial.begin(9600);
  pinMode(DHTPin, INPUT);
  dht.begin();
  pinMode(BuzzerPin,OUTPUT);
  WiFiManager wm;
  tft.init(240,280);  
  tft.setFont(&Elianto_Regular12pt7b);   
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK); 
  drawTitle();
  tft.drawRGBBitmap(55,50,bluewifi,128,128);
  tft.setTextSize(1); 
  tft.setCursor(1, 220); 
  tft.print("WIFI:AutoConnectAP");
  //wm.resetSettings();

  delay(1000);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
   bool res;
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    
    
    if(!res) {
     Serial.println("Failed to connect");
     // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
           tft.fillScreen(ST77XX_BLACK); 
           tft.drawRGBBitmap(0,0,st7789clockBK,240,280);
           drawTitle();
    }

  delay(1000);
  
  timeClient.begin();
}

void RTC_display()
{
  char dow_matrix[7][10] = {"DOMINGO", "LUNES  ", "MARTES   ", "MIERCOLES", "JUEVES", "VIERNES", "SABADO"};
  char month_matrix[12][5] = {"En", "Febr", "Mzo", "Abr", "My", "Jun", "Jul", "Ag", "Sept", "Oct", "Nov", "Dic"};
  byte x_pos[7] = {19, 19, 13, 1, 7, 19, 7};
  static byte previous_dow = 0;
  long days = day(unix_epoch);
  if( previous_dow != weekday(unix_epoch) )
  {
    previous_dow = weekday(unix_epoch);
    tft.setTextSize(1); 
    tft.setCursor(10, 70);
    String dayString = dow_matrix[previous_dow-1];
    if (dayString != lastDay) {
    tft.fillRect(40, 80, 180, 25, ST77XX_BLACK);
    lastDay = dayString;
    }
    tft.fillRect(10, 50, 200, 25, ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);       
    tft.print( dayString );
  }

  // print date
  tft.setCursor(40, 100);
  tft.setTextSize(1);  
  if (days != lastDate) {
    tft.fillRect(40, 80, 180, 25, ST77XX_BLACK);
    lastDate = days;
  }
  
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  tft.printf( "%02u %s %04u", days, month_matrix[month(unix_epoch) - 1], year(unix_epoch) );
  // print time
  long hours = hour(unix_epoch) ;
  long minutes =  minute(unix_epoch);
  long seconds = second(unix_epoch);
  tft.setCursor(40, 150);
  tft.setTextSize(2); 
  if (minutes != lastM) {
     tft.fillRect(40, 110, 180, 45, ST77XX_BLACK);
     lastM = minutes;
  }
  
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);     // set text color to green and black background
  tft.printf( "%02u:%02u", hours, minutes );
  tft.setCursor(180, 150);
  tft.setTextSize(1); 
  if (seconds != lastS) {
    lastS = seconds;
    tft.fillRect(175, 130, 40, 25, ST77XX_BLACK);
  }
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.printf( "%02u", seconds);
 
  if (hours != lastH) {
    lastH = hours;
    tone(BuzzerPin, 800);
    delay(200);
    tone(BuzzerPin, 100);
  } else {
    noTone(BuzzerPin);
  }
}

void temperatureManager() {
  unsigned long TiempoAhora = millis();
  if(TiempoAhora-tiempoUltimaLectura>5000)
    {
      //humidity = dht.readHumidity();
      temperature = dht.readTemperature();
      tiempoUltimaLectura = TiempoAhora;
      if (isnan(temperature)) {
        return;
      }
      tft.fillRect(80, 225, 110, 30, ST77XX_BLACK);
    }
    tft.drawRGBBitmap(50,220,termometer,32,32);
    char result[8]; // Buffer big enough for 7-character float
    dtostrf(temperature, 6, 2, result); // Leave room for too large numbers!
    tft.setTextSize(1); 
     tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red and black background
     tft.setCursor(80, 245);
     tft.printf(result,2);
     tft.print(" ");
     //tft.setTextSize(1); //not working with this font :/
     //tft.cp437(true);//not working with this font :/
     //tft.write(char(167));//not working with this font :/
     tft.print("C");   
  
}
void drawTitle() {    
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);     
  tft.setTextSize(0.5);                 
  tft.setCursor(40, 35);             
  tft.print("LOS BEJOS");
}

// main loop
void loop()
{
  timeClient.update();
  unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
  temperatureManager();
  RTC_display();
  delay(200);    // wait 200ms
}

// end of code.
