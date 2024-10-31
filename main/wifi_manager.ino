//for LED status
Ticker ticker;

void toggleLed()
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));     // set pin to the opposite state
  
  digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
}


void configModeCallback (WiFiManager *myWiFiManager) {
  PRINT("Entered config mode");
  PRINT(WiFi.softAPIP());
  //if you used auto generated SSID, PRINT it
  PRINT(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.25, toggleLed);
}


void wifi_manager(){
  
    // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.5, toggleLed);
  PRINT("AP Mode");
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  char buff[15];//max 15 digit AP name
  did.toCharArray(buff, 15);
  
  wifiManager.autoConnect(buff, "support123");//START AP AS DEVICE ID NAME

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect()) {
    PRINT("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  PRINT("connected...yeey :)");
  ticker.detach();
  
  alwaysOnLed();
}

void alwaysOnLed(){
    digitalWrite (LED_BUILTIN, LOW);
    digitalWrite(STATUS_LED, HIGH);
}
