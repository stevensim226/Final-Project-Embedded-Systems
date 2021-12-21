#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WifiClient.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient client;

String ssid = "XXXXXXX"; // Censored!
String password = "XXXXXXX"; // Censored!

String topic = "home/firesystem";
String subbed_topic = "home/esp8266";

String mqtt_broker_ip = "192.168.1.18"; // Server IP in my local network
int mqtt_port = 2222;

void callback(char* topic, byte* payload, unsigned int length) {
    for (int i = 0; i < length; i++) {
        writeString(String((char)payload[i]));
    }
    Serial.write("\n");
}

void setup() {
  Serial.begin(9600);
  Serial.write("Starting ESP8266...\n");

  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.write("Connecting..\n");
  }

  Serial.write("Connecting to MQTT server...\n");
  
  client.setClient(wifiClient);
  client.setServer(mqtt_broker_ip.c_str(), mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "58841eedb322e49a4c0d6961edf752e0";
      
      if (client.connect(client_id.c_str())) {
      } else {
              Serial.write("failed with state ");
              writeString(String(client.state()));
              Serial.write("\n");
              delay(1000);
      }

  }
  client.subscribe(subbed_topic.c_str());
  Serial.write("Connected to MQTT Broker! writing to 'home/firesystem'\n");
  client.publish(topic.c_str(), "FINISH_CONN_MQTT");
}

void loop() {
  String s = "";
  boolean t = false;
  while(Serial.available() > 0) {
     s = Serial.readString();
     t = true;
  }

  if (t){
    client.publish(topic.c_str(), s.c_str());
    Serial.write("ACK\n");
  }
  client.loop();
}

void writeString(String stringData) { // Used to serially push out a String with Serial.write()
  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
}
