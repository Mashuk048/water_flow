//------------------------------------------------While client not conncected---------------------------------//


boolean reconnect() {

    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266-";
    clientId += String(random(0xffff), HEX);
  
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");     

      if(client.subscribe(configTopic)){
        Serial.println("Subscribed to configTopic");
      }
      
      delay(200);
      
      if(client.subscribe(cmdTopic)){
        Serial.println("Subscribed to cmdTopic"); 
      }
      
      delay(200);
      
    }else{  
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again");
      mqttTryCounter++;
      Serial.println(mqttTryCounter);
      // Wait 6 seconds before retrying
//      delay(5000); //blocking
    }

    if(mqttTryCounter==5){
      Serial.print("MQTT connection Failled ...");
      Serial.print(mqttTryCounter);
      mqttTryCounter=0;
    }
  return client.connected();
}// reconnect() ends
