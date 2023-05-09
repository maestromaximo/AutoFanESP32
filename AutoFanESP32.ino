//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"
#include "DHT.h"

#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 4

DHT dht(DHTPIN, DHTTYPE);
int relay = 19;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


//deepsleep
#define Threshold 40 /* Greater the value, more the sensitivity */

RTC_DATA_ATTR int bootCount = 0;
touch_pad_t touchPin;
//deepsleep

BluetoothSerial SerialBT;


String message = "";
char incomingChar;
String temperatureString = "";

unsigned long previousMillis = 0;
const long interval = 10000;

bool forced = false;


void setup() {
  pinMode(relay, OUTPUT);
  dht.begin();
  Serial.begin(115200);

  delay(100);
  ++bootCount;
   print_wakeup_reason();
  print_wakeup_touchpad();

   //Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(T3, callback, Threshold);

  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();
  
  SerialBT.begin("HouseEla"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
}

void loop() {

  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    //tempread

    
     float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  //
  if (isnan(h) || isnan(t) ) {
   Serial.println(F("Failed to read from DHT sensor!"));
   return;}
  float hic = dht.computeHeatIndex(t, h, false);
  //
  if(hic >= 23){
    OpenRelay();
  }else{
    CloseRelay();
  }
    temperatureString = String(hic);
    SerialBT.println(temperatureString);
  }




  
  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();
    if(incomingChar != '\n'){
      message += String(incomingChar);
    }else{
      message = "";
    }
    Serial.write(incomingChar);
  }
  if (message == "relay_on") {
    ForcedOpenRelay();
  }else if(message == "relay_off"){
    ForcedCloseRelay();
  }
  else if(message == "timed_relay"){
    ForcedOpenRelay();
    delay(1800000);
    esp_deep_sleep_start();
    ForcedCloseRelay();
  }else if(message == "touch_sleep"){
    esp_deep_sleep_start();
  }
  delay(20);
}//loop


void OpenRelay(){

if(!forced){
  digitalWrite(relay, HIGH);
  }
}

void CloseRelay(){
  if(!forced){
  digitalWrite(relay, LOW);
  }
}

void ForcedOpenRelay(){


  digitalWrite(relay, HIGH);
  forced = true;
}

void ForcedCloseRelay(){
  
  digitalWrite(relay, LOW);
  forced = false;
}

void print_wakeup_touchpad(){
  touchPin = esp_sleep_get_touchpad_wakeup_status();

  switch(touchPin)
  {
    case 0  : Serial.println("Touch detected on GPIO 4"); break;
    case 1  : Serial.println("Touch detected on GPIO 0"); break;
    case 2  : Serial.println("Touch detected on GPIO 2"); break;
    case 3  : Serial.println("Touch detected on GPIO 15"); break;
    case 4  : Serial.println("Touch detected on GPIO 13"); break;
    case 5  : Serial.println("Touch detected on GPIO 12"); break;
    case 6  : Serial.println("Touch detected on GPIO 14"); break;
    case 7  : Serial.println("Touch detected on GPIO 27"); break;
    case 8  : Serial.println("Touch detected on GPIO 33"); break;
    case 9  : Serial.println("Touch detected on GPIO 32"); break;
    default : Serial.println("Wakeup not by touchpad"); break;
  }
}

void callback(){
  //placeholder callback function
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
