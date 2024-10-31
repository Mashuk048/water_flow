/*
                  OUT TOPIC
  Machine running status topic  : autobots/dot/amp
  RPM topic                     : autobots/dot/rpm
  Device Response topic         : autobots/dot/response

                  IN TOPIC
  Configuration topic           : autobots/dot/config
  Command topic                 : autobots/dot/cmd

  did : KC002

  Command msg format:
  {
  "did":"DOT001",
  "reset":"1",
  "ap":"1"
  }

  Config change msg Format: //ptime in second
  {
  "did":"DOT001",
  "ptime":"30-600",
  "delay":"5-200"
  }
*/
//-----------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// Ticker for watchdog

#include <Ticker.h>
Ticker secondTick, thirdTick;

//Energy monitor
#include "EmonLib.h"
EnergyMonitor emon1;


//DEVICE NAME
String  did = "DOT_WFM_002";//might change so didn't define as const char*
String  type = "water_flow_sensor";

//MQTT  credentials
const char* mqtt_server = "broker.datasoft-bd.com";
const int mqttPort = 1883;
const char *mqtt_user = "iotdatasoft";
const char *mqtt_pass = "brokeriot2o2o";
int mqttTryCounter = 0;



//OUT TOPIC
const char* mrtimeTopic = "autobots/dot/wfm";
const char* rpmTopic = "autobots/dot/wfm";
const char* responseTopic = "autobots/dot/wfm/response";
//IN TOPIC
const char* configTopic = "autobots/dot/wfm/config";
const char* cmdTopic = "autobots/dot/wfm/cmd";



WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastWiFiCheckTime = 0;
unsigned long lastReconnectTime = 0;//MQTT reconnect

unsigned long wifi_check_interval = 5000;

unsigned long lastPublishedTime = 0;

//publish interval
unsigned long publish_interval = 60000;//


//---------------water flow sensor parameters----------------------------------------------------//

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

const int interval = 1000;

unsigned long calibrationFactor = 0.52;//++

volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres = 0.0;
unsigned long totalLitres = 0;
unsigned long totalMilliLitres = 0.0;

//prev
unsigned int sampleCount = 0;
float FinalIrms = 0.0;
float total = 0.0;
const byte SAMPLE_SIZE = 19 ;
unsigned int FLAG = 0;
//prev
//---------------Defining Pins----------------------------------------------//
#define anaPRINTPin A0
#define LED_BUILTIN 16// D0 BUILTIN LED
#define STATUS_LED 14 // D5
#define amp_calc 60.606
const int SENSOR = 5; //D1

float cutOffLimit = 0.50;




//-----------------------------------------WFM----------------------------------------------------//


void ICACHE_RAM_ATTR pulseCounter()
{
  pulseCount++;
}

//// Holds the current button state.
//volatile int state = 1;
//
//volatile byte interruptCounter = 0;
//
//
//// Holds the last time debounce was evaluated (in millis).
//volatile long lastDebounceTime = 0;
//
//// The delay threshold for debounce checking.
//unsigned int debounceDelay = 5;//(5ms) works fine for the machine with damaged magnet. Not a const type!


//TIMER FOR RPM
//unsigned long lastRpmCalDoneTime = 0;

#define DEBUG 0

#ifdef DEBUG
#define PRINT(x)  Serial.println(x)
#else
#define PRINT(x)
#endif



//--------------------Watchdog----------------------//
volatile int watchdogCount = 0;

void ISRwatchdog() {
  watchdogCount++;
  if (watchdogCount >= 180) {
    PRINT("Watchdog bites!!!");
    ESP.reset();
  }
}



// Gets called by the interrupt.
/*void   ICACHE_RAM_ATTR   ISR() {
  // Get the pin reading.
  int reading = digitalRead(rpmSensorPin);

  // Ignore dupe readings.
  if (reading == state) return;

  boolean debounce = false;
  // Check to see if the change is within a debounce delay threshold.
  if ((millis() - lastDebounceTime) <= debounceDelay) {
    debounce = true;
  }

  // This update to the last debounce check is necessary regardless of debounce state.
  lastDebounceTime = millis();

  // Ignore reads within a debounce delay threshold.
  if (debounce) return;

  // All is good, persist the reading as the state.
  state = reading;


  if (!state) { //if button pin is read 1(High Pulse) or read 0(LOW Pulse), change accordingly and remember it is a guaranteed press
    interruptCounter++;
    PRINT(interruptCounter);
  }

  }//ISR ENDS

*/



//--------------------------------Main Setup----------------------------------------------------//

void setup() {
  Serial.begin(115200);
  emon1.current(anaPRINTPin, amp_calc);   // Current: input pin, calibration.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  //  pinMode(rpmSensorPin, INPUT);
  //  attachInterrupt(rpmSensorPin, ISR,  CHANGE);
  pinMode(SENSOR, INPUT_PULLUP);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);

  secondTick.attach(1, ISRwatchdog);// Attaching ISRwatchdog function

  delay(10);
  //set_wifi();
  client.setServer(mqtt_server, mqttPort); //Connecting to broker
  client.setCallback(callback); // Attaching callback for subscribe mode
  PRINT("device started");
}
//setup ends




//------------------------------------Main Loop--------------------------------------------------//
void loop() {
//
  watchdogCount = 0;//SS
  delay(10);
  checkWiFi();//SS
  delay(10);
  setMqttReconnectInterval();//SS
  delay(10);
  dataRead();
  delay(10);
  checkWfmPublishTimer();
  delay(10);

  
  //  if (FLAG == LOW) {
  //    IrmsAvg();//SS++
  //  }

  //checkRPMTimer();//SS

}//LOOP ENDS





void checkWiFi() {
  unsigned long now = millis();
  if (now - lastWiFiCheckTime > wifi_check_interval) {
    lastWiFiCheckTime = now;

    if (WiFi.status() != WL_CONNECTED) {
      wifi_manager();
      //set_wifi();
    }
    else {

      //      PRINT(" Wifi already connected");
    }
  }
}


void setMqttReconnectInterval() {
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectTime > 5000) {
      lastReconnectTime = now;
      PRINT("Ticking every 5 seconds for mqtt reconnect");

      //write your own code

      //          P_R_I_N_T(WiFi.status());

      if (WiFi.status() == WL_CONNECTED) {
        // Attempt to mqtt reconnect
        if (reconnect()) {
          lastReconnectTime = 0;//GOOD
        }
      }

    }//end of interval check
  } else {
    client.loop();
  }

}//setMqttReconnect

void dataRead()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    totalLitres = totalMilliLitres / 1000;

    //Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
    // Print the cumulative total of litres flowed since starting
    Serial.print("Total in Litres: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");

  }
}

void checkWfmPublishTimer() {
//  PRINT("Inside sensor data publish");

  if (totalLitres > 4 && totalLitres <5 ) {

    if (!client.connected()) {
      reconnect();
    } else {
      publishWfm();
      PRINT((String)"data pub" );
      totalLitres = 0.0;
    }

  }// Timer ends
}

float rssi_measure() {
  float rssi = WiFi.RSSI();
  //Serial.println(rssi);
  return rssi;
}

void publishWfm() {
  StaticJsonDocument<128> doc;
  doc["did"] = did;
  doc["type"] = type;
  doc["cal_fac"] = calibrationFactor;
  doc["Wfm"] = totalLitres;
  doc["rssi"] = rssi_measure();
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  int result = client.publish(mrtimeTopic, buffer, n);
  delay(250);
}
/*
  float IrmsAvg() {


  {
    if (millis() - previousMillis >= INTERVAL_MILLIS)
    {
      previousMillis = millis();
      total = total + anaPRINT_sensor_data();
      //      PRINT((String)"total " + total +" A" );
      sampleCount = sampleCount + 1 ;
    }

    if (sampleCount == SAMPLE_SIZE)
    {
      float avgData = (total / sampleCount);
      //      PRINT((String)"AvgData: " + avgData + " A" );
      // reset for the next group
      FinalIrms = avgData;
      FLAG = HIGH;
      sampleCount = 0;
      total = 0;

    }
    //    return avgData;
  }

  }


  double anaPRINT_sensor_data() {

  //
  unsigned int x = 0;
  double Irms = 0.0;
  float Samples = 0.0, AvgIrms = 0.0;

  for (int x = 0; x < 15; x++) { //Get 150 samples
    Irms = emon1.calcIrms(1480);     //Read current sensor values
    Samples = Samples + Irms;  //Add samples together
    delay (3); // let ADC settle before next sample 3ms
  }
  AvgIrms = (Samples / 15); //Taking Average of Samples

  //
  //  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  if (AvgIrms > cutOffLimit ) {
    //    Serial.print(" I: ");
    Irms = AvgIrms; // print the current with 2 decimal places
    //    Serial.println("A");
  } else {
    Irms = 0.0;
  }
  return Irms;
  delay (50);
  }


  if (result) {
    //    FinalIrms = 0.0;
    PRINT((String)"IrmsData " + FinalIrms + " A" );
    PRINT("Irms Published");
    FinalIrms = 0.0;
    FLAG = LOW;
    blip();
  } else {
    PRINT("Irms Publish Failed");
  }
  }

  void checkRPMTimer() {
  unsigned long now = millis();
  if (now - lastRpmCalDoneTime > 60000) {
    lastRpmCalDoneTime = now;
    //write code here
    unsigned int rpm = interruptCounter;//the pointer 'ptr' is now pointing to the address of the volatile data byte 'interruptCounter'

    if (!client.connected()) {
      reconnect();
    }  else  {
      publishRPM(rpm);
    }

    //    PRINT("RPM = "+rpm);//dereferencing ptr for PRINTing the CONTENT
    interruptCounter = 0;
  }
  }

  void publishRPM(unsigned int rpm) {
  StaticJsonDocument<128> doc;
  doc["did"] = did;
  doc["rpm"] = rpm;

  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  int result = client.publish(rpmTopic, buffer, n);
  delay(250);

  if (result) {
    PRINT("RPM Published");
    blip();
  } else {
    PRINT("RPM Publish Failed");
  }
  }


*/


void blip() {
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(STATUS_LED, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
}



void set_wifi() {
  delay(250);
  int tryCount = 0;
  PRINT("");
  PRINT("Connecting to WiFi");
  WiFi.begin("DataSoft_WiFi", "support123");
  while (WiFi.status() != WL_CONNECTED) {
    PRINT(".");
    delay(1000);        //........ 1 sec delay
    PRINT(".");
    tryCount++;
    if (tryCount == 10) return loop(); //exiting loop after trying 10 times
  }
  PRINT("");
  PRINT("Connected");
  PRINT(WiFi.localIP());
  delay(250);
}



void onDemandAP() {
  PRINT("OnDemandAP Starting...");

  // start ticker with 0.5 because we start in AP mode and try to connect
  thirdTick.attach(0.25, toggleLed);

  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //reset settings - for testing
  wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration

  //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
  //WiFi.mode(WIFI_STA);
  char buff[15];//max 15 digit AP name
  did.toCharArray(buff, 15);

  if (!wifiManager.startConfigPortal(buff, "support123")) {
    PRINT("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  PRINT("connected...yeey :)");
  thirdTick.detach();
}
