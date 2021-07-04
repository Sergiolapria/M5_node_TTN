
/*
Based on a program written by Achim Kern
called "M5_Stack_CORE_TTN_SensorNode.ino"                       
*/
//
char application[]="M5STACK_CORE TTN ";
char aktu_version[]="V1.5";
char author[]="SergioPria";
//
#include <Arduino.h>
#include <M5Stack.h>
#include "Free_Fonts.h"
//#include "ttn.h"
#include <Wire.h>
#include <AHT10.h>
//#include "MAX44009.h"
///////////////////////////
/////////////////////////
/// VALUES 
////////////////////////
int luxometer=0;
float temperature=0.0;
float humidity=0.0;
int luxometer_ttn=0;
int temperature_ttn=0;
int humidity_ttn=0;
int bat_ttn=0;
String chip;
String response;
bool error=false;
///////////////////////
int TTN_Counter=0;
uint32_t battery_int=0;
uint32_t temp_int=0;
uint32_t hum_int=0;
uint32_t diff1=200;
int32_t diff2=100;
int32_t diff3=300;

////////
uint32_t old_battery_int=0;
uint32_t old_temp_int=0;
uint32_t old_hum_int=0;
///////////////////////
///////////////////////
AHT10 aht(AHT10_ADDRESS_0X38);


/*-------------------------------------------------------------------------------*/
/* Function bool ReceiveAT(uint32_t timeout)                                     */
/*                                                                               */
/* TASK    : receive AT msg's from the M5Stack COM.LoRaWAN Module                */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
bool ReceiveAT(uint32_t timeout)
{
  uint32_t nowtime = millis();
  while(millis() - nowtime < timeout){
    if (Serial2.available() !=0) {
      String str = Serial2.readString();  
      if (str.indexOf("+OK") != -1) {
        Serial.println(str);
        error=false;
        return true;
      }
      if(str.indexOf("+ERROR") !=-1){
        Serial.print("[X]ERROR");
        Serial.print(" ");
        Serial.println(str);
        response=str.substring(7);
        Serial.println(response);
        error=true;
        return true;
      }
        else {
        Serial.println("[!] Syntax Error");
        error=true;
        break;
      }
    }
    M5.Speaker.mute();
  }
  Serial.println("[!] Timeout");
  return false;
}
/*-------------------------------------------------------------------------------*/
/* Function void ATCommand(char cmd[],char date[], uint32_t timeout = 50)        */
/*                                                                               */
/* TASK    : send AT commands to the M5Stack COM.LoRaWAN Module                  */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void ATCommand(char cmd[],char date[], uint32_t timeout = 50)
{
  char buf[256] = {0};
  if(date == NULL)
  {
    sprintf(buf,"AT+%s",cmd);
  }
  else 
  {
    sprintf(buf,"AT+%s=%s",cmd,date); 
  }
  Serial2.write(buf);
  Serial.println(buf);
  delay(200);
  ReceiveAT(timeout);
}
/*-------------------------------------------------------------------------------*/
/* Function void array_to_string(byte array[], unsigned int len, char buffer[])  */
/*                                                                               */
/* TASK    : build string out of payload data                                    */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void array_to_string(byte array[], unsigned int len, char buffer[])
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len*2] = '\0';
}

/*-------------------------------------------------------------------------------*/
/* Function void send_to_TTN(void)                                               */
/*                                                                               */
/* TASK    : send sensor data to TTN                                             */
/* UPDATE  : 25.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void send_to_TTN(void) 
{  
  byte payload[9];
  
  payload[0]= temp_int;
  payload[1] = temp_int >> 8;
  payload[2] = temp_int >> 16;
  
  payload[3] = hum_int;
  payload[4] = hum_int >> 8;
  payload[5] = hum_int >> 16;
  
  payload[6]  = battery_int;
  payload[7] = battery_int >> 8;
  payload[8] = battery_int >> 16;

  Serial.print(F("[?] actual TTN payload --> "));
  char str[32] = "";
  array_to_string(payload, 9, str);
 
  Serial.println(str);
  M5.Lcd.fillRect(0,0,75,90,TFT_RED);
  // now send all to TTN
  ATCommand("SendHex",str);
  M5.Speaker.beep();
  delay(500);
  M5.Speaker.mute();
  //ATCommand("SendStr",str); 
}
void draw_template(){
  ///////////////////////////////////////
  M5.Lcd.drawLine(75,0,75,260,TFT_BLACK);
  M5.Lcd.drawLine(0,90,75,90,TFT_BLACK);
    //60
  M5.Lcd.drawLine(75,70,320,70,TFT_BLACK);
    //130
  M5.Lcd.drawLine(75,140,320,140,TFT_BLACK);
    //230
  M5.Lcd.drawLine(75,240,320,240,TFT_BLACK);
  //////////////////////////////////////
  M5.Lcd.fillRect(0,0,75,90,TFT_GREEN);
  //////////////////////////////////////
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth("Battery:"))/2,20);
  M5.Lcd.print("Battery:");
  /////
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth("Temperature:"))/2,90);
  M5.Lcd.print("Temperature:");
  /////
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth("Humidity:"))/2,170);
  M5.Lcd.print("Humidity:");
  //////
}
void show_display(){
  M5.Lcd.fillScreen(TFT_WHITE);
  draw_template();
  M5.Lcd.setFreeFont(FMB24);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth(String(M5.Power.getBatteryLevel())))/2,60);
  M5.Lcd.print(String(M5.Power.getBatteryLevel()));
  M5.Lcd.print("%");
  M5.Lcd.setFreeFont(FMB24);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth(String (temperature)))/2,130);
  M5.Lcd.print(String (temperature));
  M5.Lcd.print("ºC");
  M5.Lcd.setFreeFont(FMB24);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth(String (humidity)))/2,230);
  M5.Lcd.print(String (humidity));
  M5.Lcd.print("%");
}
void setup_lorawan(){
    //We'll connect to the M5Stack the COM.LoRaWAN module
  // TX 0/3/17
  // RX 5/15/16
  // Make sure the dip switch is corrected
  //for the M5Stackwe'll use 16/17
  Serial2.begin(115200,SERIAL_8N1,16,17);
  //We'll have to set some parameters
  //the first order doesn't work,it'll be for awake module
  ATCommand("LORAWAN","?");
  delay(500);
  ATCommand("ChipID","?");
  //set LoRaWAN Mode
  ATCommand("OTAA","1");
  delay(500);
  ATCommand("Class","C");
  delay(500);
  ATCommand("ADR","1");
  delay(500);
  ATCommand("DutyCycle","15000");
  delay(500);
//TTN access data
/// TTN Parameters
///  ABP
//////////////////////////
// DevAddr=260B7F1F
// AppSKey=9EBB102678877A940C7170CFE6070C8B
// NwkSKey=0585D57068A19F9B80E2D7A4B00B0B67
////////////////////////
//   OTAA
///////////////////////
// DevEui=1111111111111111
// AppEui=AABBCCDDEEFF1122
// AppKey=3D8F9F0795582EF721979E10F8B8D98D
///////////////////////
ATCommand("DevEui","1111111111111111");
delay(500);
ATCommand("AppEui","AABBCCDDEEFF1122");
delay(500);
ATCommand("AppKey","3D8F9F0795582EF721979E10F8B8D98D");
delay(500);
ATCommand("ConfirmedNbTrials","4");
delay(500);
//We'll join the TTN network
ATCommand("Join","1");
delay(500);
ATCommand("Join","?");
delay(500);
}
void setup() {
  M5.begin();
  M5.Power.begin();
  Serial.begin(9600);
  //Boot
  delay(2000);
  Serial.println(F(""));
  Serial.println(F("Starting..."));
  Serial.print(F(application));
  Serial.print(F(" Version"));
  Serial.println(F(aktu_version));
  Serial.println(F("connected via TTN"));
  Serial.println(F(""));
  M5.Lcd.fillScreen(TFT_RED);
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth(application))/2,100);
  M5.Lcd.print(application);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth(aktu_version))/2,120);
  M5.Lcd.print(aktu_version);
  M5.Lcd.setCursor((320-M5.Lcd.textWidth(author))/2,160);
  M5.Lcd.print(author);
  delay(6000);
  while(!aht.begin()){
    Serial.println(F("AHT10 not connected"));
  }
  //SD card not develope
  //Battery level
  uint8_t bat=M5.Power.getBatteryLevel();
  Serial.println(F("[?] M5STACK BATTERY LEVEL -->"));
  Serial.print(bat);Serial.println(F(" %"));
  //call the setup_LORAWAN
  setup_lorawan();
}
void read_aht(){
  if(!M5.Power.canControl())
  {
    Serial.println(F("[!] No communication with IP5306 chip"));
  }
  // actual battery level
  uint8_t bat = M5.Power.getBatteryLevel();
  bat_ttn=bat;
  battery_int = bat * 100; 
  // now we create the payload and send it to the TTN
  temperature=aht.readTemperature();
  humidity=aht.readHumidity();
  temp_int      = temperature * 100;
  hum_int  = humidity * 100;
  Serial.print(temperature);
  Serial.print(" ");
  Serial.println(humidity);
}
bool unbounce(uint32_t magnitude,uint32_t old_magnitude,uint32_t diff){
  if(magnitude<<old_magnitude-diff || magnitude>>old_magnitude+diff){
    return true;
  }else{
    return false;
  }
}
void loop() {
    read_aht();
    if(hum_int<old_hum_int-diff3 || hum_int>old_hum_int+diff3 || M5.BtnA.wasPressed()){
      old_hum_int=hum_int;
      send_to_TTN();
      show_display();
      ATCommand("LPM","1");
      //Serial.println("SEND");
    }
    if(old_temp_int<old_temp_int-diff2 || temp_int>old_temp_int+diff2 || M5.BtnA.wasPressed()){
      old_temp_int=temp_int;
      send_to_TTN();
      show_display();
      ATCommand("LPM","1");
    }
    //else{
    //  Serial.println("no envio");
    //}
    if(!M5.BtnB.isReleased()){
      int diff=diff3/100;
      diff--;
      M5.Speaker.beep();
      delay(50);
      M5.Speaker.mute();
      if(diff==-1){
        diff=0;
      }
      Serial.print(diff);
      Serial.print(" ");
      diff3=diff*100;
      Serial.println(diff3);
    }
    if(!M5.BtnC.isReleased()){
      int diff=diff3/100;
      diff++;
      M5.Speaker.beep();
      delay(50);
      M5.Speaker.mute();
      if(diff==0){
        diff=1;
      }
      Serial.print(diff);
      diff3=diff*100;
      Serial.print(" ");
      Serial.println(diff3);
    }
    /*
    M5.Lcd.setTextColor(TFT_BLACK);
    M5.Lcd.setFreeFont(FMB12);
    M5.Lcd.setCursor(10,60);
    M5.Lcd.print("Diff:");
    M5.Lcd.setCursor(10,80);
    int diff=diff3/100;
    M5.Lcd.println(diff);
    */
    M5.update(); 
}