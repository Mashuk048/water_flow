//-----------------------Callback function-------------------------------------//

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
//-------------------------------------Getting config data---------------------//

if(strcmp(topic, cmdTopic) == 0){
  
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);

  String devId = doc["did"];
  int rst = doc["reset"];
  int ap = doc["ap"];
  Serial.println(devId);
  Serial.println(rst);
  Serial.println(ap);

  //reset command check
  if( devId == did ){
      if(rst == 1){
        Serial.println("RESET");

        char buffer[256];
        doc["status"] = "success";
        size_t n = serializeJson(doc, buffer);
        client.publish(responseTopic,buffer, n);
        delay(2000);
        ESP.reset();
      }else{
        Serial.println("INVALID RESET COMMAND");

        char buffer[256];
        doc["status"] = "failed";
        size_t n = serializeJson(doc, buffer);
        client.publish(responseTopic,buffer, n);
        delay(2000);
      }

   //AP command check
   if(ap == 1){     
        
        char buffer[256];
        doc["status"] = "success";
        size_t n = serializeJson(doc, buffer);
        client.publish(responseTopic,buffer, n);
        delay(200);

        onDemandAP();
        
      }else{
        Serial.println("INVALID ON-DEMAND AP COMMAND");

        char buffer[256];
        doc["status"] = "failed";
        size_t n = serializeJson(doc, buffer);
        client.publish(responseTopic,buffer, n);
        delay(2000);
      }
   

      
  }
  
}//strcmp ends

  

  if(strcmp(topic, configTopic) == 0){
    
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
  
    String devId = doc["did"];
    unsigned int ptime = doc["ptime"];
    unsigned int ddelay = doc["delay"];
    Serial.println(devId);
    Serial.println(ptime);
    Serial.println(ddelay);
   
      
    if( devId == did){
        if(ptime >=10 && ptime <= 600){ //publish time within 10s -10 min          
         
          publish_interval = ptime*1000;
          Serial.println("PUBLISH INTERVAL UPDATED");
          delay(500);
          char buffer[256];
          doc["status"] = "success";
          size_t n = serializeJson(doc, buffer);
          client.publish(responseTopic,buffer, n);
          delay(250);
        }
        else{
          Serial.println("INVALID PUBLISH TIME");
          char buffer[256];
          doc["status"] = "failed";
          size_t n = serializeJson(doc, buffer);
          client.publish(responseTopic,buffer, n);
          delay(250);
        }


        if(ddelay >=0 && ddelay <= 200){ //ddelay within 0-200ms
          Serial.println("DDELAY TIME UPDATED");
          calibrationFactor = ddelay;
          delay(500);
          char buffer[256];
          doc["status"] = "success";
          size_t n = serializeJson(doc, buffer);
          client.publish(responseTopic,buffer, n);
          delay(250);
        }
        else{
          Serial.println("INVALID DDELAY TIME");
          char buffer[256];
          doc["status"] = "failed";
          size_t n = serializeJson(doc, buffer);
          client.publish(responseTopic,buffer, n);
          delay(250);
        }
        
    }else{
         //do nothing
         Serial.println("DEVICE ID DID NOT MATCH");
    }
 }//strcomp ends
     
}//Callback ends
