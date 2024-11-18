/* ------------------------------------------------------------------------- *
 * Name   : C3_SHACK_UDP
 * Author : Stéphane HELAIEM - F4IRX
 * Date   : January 26, 2024
 * Purpose: Server UDP for Statut of QO100 Amplifier
 * Versions:
 *    0.1  : Initial code base, test IP/UDP
 *    0.1b : Add Display 1.8 SPI
 *     
 *     
 *     
 *     
 *     
 *     
 *     
  * ------------------------------------------------------------------------- */
#define progVersion "0.1b"          

/* ------------------------------------------------------------------------- *
 *       Include libraries 
 * ------------------------------------------------------------------------- */
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
/* ------------------------------------------------------------------------- *
 *       Other definitions
 * ------------------------------------------------------------------------- */
#define TFT_CS         10
#define TFT_RST        9
#define TFT_DC         8
#define REMOTE_IP      "192.168.1.158"

unsigned long currentTime=0;
unsigned long previousTime=0;
// color definitions
const uint16_t  Display_Color_Black        = 0x0000;
const uint16_t  Display_Color_Blue         = 0x001F;
const uint16_t  Display_Color_Red          = 0xF800;
const uint16_t  Display_Color_Green        = 0x07E0;
const uint16_t  Display_Color_Cyan         = 0x07FF;
const uint16_t  Display_Color_Magenta      = 0xF81F;
const uint16_t  Display_Color_Yellow       = 0xFFE0;
const uint16_t  Display_Color_White        = 0xFFFF;
const uint16_t  Display_Color_grey         = 0x8410;
const uint16_t  Display_Color_grey2         = 0x4A49;
//see https://barth-dev.de/online/rgb565-color-picker/ for color

char packetBuffer[100];

struct Contol_Data {
    bool Alarm;
    float Voltage;
    float Current;
    float Temp;
  };

struct Contol_Data AmpliQO100Info;
unsigned int localPort = 9999;
const char *ssid = "****";
const char *password = "****";

/* ------------------------------------------------------------------------- *
 *       Create Object
 * ------------------------------------------------------------------------- */
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
WiFiUDP udp;




/* ------------------------------------------------------------------------- *
 *       Setup
 * ------------------------------------------------------------------------- */
void setup() {
  Serial.begin(115200);

  Serial.println(F("F1ZGM telecommande IP : " __VERSION__)); 
    Serial.println(F("Built on " __DATE__ " at " __TIME__));  
  Serial.print("RemoteIP version :");
  Serial.println(progVersion);
  Serial.println("Connection WIFI");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("Start UDP");
  udp.begin(localPort);
  Serial.printf("UDP server : %s:%i \n", WiFi.localIP().toString().c_str(), localPort);
  Serial.print("[Server Connected] ");
  Serial.println(WiFi.localIP());
  Serial.println("Init TFT");
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setFont();
  tft.fillScreen(Display_Color_Black);
  tft.setTextWrap(true);
  tft.drawRoundRect(0, 10 , 128, 45,5, Display_Color_grey);  //x,y,long,heihgt
  tft.drawRect(0, 60 , 128, 45, Display_Color_grey);  //x,y,long,heihgt
  tft.drawRect(0, 110 , 128, 45, Display_Color_grey);  //x,y,long,heihgt
  tft.setTextSize(1);
  tft.setTextColor(Display_Color_White);
  tft.setCursor(4, 14);
  tft.print("Temperature");
  tft.setCursor(4, 64);
  tft.print("Voltage");
  tft.setCursor(4, 114);
  tft.print("Current");
  tft.setTextSize(1);
  tft.setTextColor(Display_Color_grey2);
  tft.setCursor(1, 1);  // longueur ecran-longuer chaine/2
  tft.print(WiFi.localIP().toString().c_str());
  Serial.println("Ready");
}

/* ------------------------------------------------------------------------- *
 *       CheckWIFI
 * ------------------------------------------------------------------------- */
void CheckWIFI() {
    Serial.print(F("Check WIFI : "));
      while (WiFi.status() != WL_CONNECTED) {
        ESP.restart();
      }
    long rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
    tft.setTextSize(1);
    tft.setTextColor(Display_Color_grey2);
    tft.setCursor(1, 1);  // longueur ecran-longuer chaine/2
    tft.print(WiFi.localIP().toString().c_str());
    tft.setCursor(90, 1);  // longueur ecran-longuer chaine/2
    tft.print(rssi);
    Serial.println(WiFi.localIP().toString().c_str());}
/* ------------------------------------------------------------------------- *
 *       Loop
 * ------------------------------------------------------------------------- */
void loop() {
    currentTime=millis();
    if((currentTime-previousTime)>30000){
        previousTime=currentTime;
        CheckWIFI();
       }  // every 30sec
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
    Serial.print(" Received packet from : ");
    Serial.print(udp.remoteIP());
    Serial.print(" Size : ");
    Serial.print(packetSize);
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len - 1] = 0;
    Serial.printf("  Data : %s\n", packetBuffer);
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("UDP packet was received OK\r\n");
    udp.endPacket();
    char *ptr=NULL;
    byte index=0;
    char *strings[100];
    ptr=strtok(packetBuffer,",");
    while (ptr!=NULL) {
        strings[index]=ptr;
        index++;
        ptr=strtok(NULL,",");
      }
    if ( udp.remoteIP()==REMOTE_IP) {
        AmpliQO100Info.Alarm=atoi(strings[3]);
        tft.fillRect(10, 25 , 110, 25, Display_Color_Black);  //x,y,long,heihgt
        tft.fillRect(10, 75 , 110, 25, Display_Color_Black);  //x,y,long,heihgt
        tft.fillRect(10, 125 , 110, 25, Display_Color_Black);  //x,y,long,heihgt
        tft.setTextSize(3);
        if(AmpliQO100Info.Alarm==1) {tft.setTextColor(Display_Color_Red);}else{tft.setTextColor(Display_Color_Blue);}
        tft.setCursor(22, 27);  // longueur ecran-longuer chaine/2
        tft.print(strings[0]);
        tft.setTextColor(Display_Color_Blue);
        tft.setCursor(22, 76);  // longueur ecran-longuer chaine/2
        tft.print(strings[1]);
        tft.setTextColor(Display_Color_Blue);
        tft.setCursor(22, 126);  // longueur ecran-longuer chaine/2
        tft.print(strings[2]);
      }
    tft.setTextSize(1);
    tft.setTextColor(Display_Color_Red);
    tft.setCursor(112, 1);  // longueur ecran-longuer chaine/2
    tft.print("R");
    delay(250);
    tft.setTextColor(Display_Color_Black);
    tft.setCursor(112, 1);  // longueur ecran-longuer chaine/2
    tft.print("R");
  }
}