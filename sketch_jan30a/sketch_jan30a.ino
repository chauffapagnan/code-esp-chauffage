#include "WiFi.h"
#include "PubSubClient.h"
#define LED 2
#define ON 4
#define VENT 5

// WiFi
const char *ssid = "Pixel_2035";
const char *password = "grossebite";
//MQTT Broker
const char *mqtt_broker = "192.168.72.92"; //148.60.35.79 sur univ rennes, sinon sur pixel
const char *topic = "LED";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

boolean led = false;


void callback(char* topic, byte* payload, unsigned int length) { 
  char msg[length];
  for (int i = 0; i < length; i++) { 
    msg[i] = (char) payload[i];
  } 
  msg[length] = '\0';

  Serial.print(msg);
  Serial.print("\n");

  if(strcmp(msg, "on")==0){
    Serial.print("Je turn on\n");
    digitalWrite(LED, HIGH);
  }

  if(strcmp(msg, "off")==0){
    Serial.print("Je turn off\n");
    digitalWrite(LED, LOW);
  }


      /*
       Serial.print("Message:"); 
       for (int i = 0; i < length; i++) { 
           Serial.print((char) payload[i]); 
       } 
       Serial.println(led); 
       Serial.println("-----------------------"); 
       led = !led;
       if(led){
        digitalWrite(LED, HIGH);
       }
       else{
        digitalWrite(LED, LOW);
       }
       */
}


void setup() {
  //Initialisation de la communication Serial et Wifi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.println("Connecting to wifi...");
  }

  client.setServer(mqtt_broker, mqtt_port); 
  client.setCallback(callback); 
  while (!client.connected()) { 
      String client_id = "esp32-client-"; 
      client_id += String(WiFi.macAddress()); 
      Serial.printf("The client %s connects to the public MQTT broker ", client_id.c_str()); 
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) { 
          Serial.println("Public EMQX MQTT broker connected"); 
      } else { 
          Serial.print("failed with state "); 
          Serial.println(client.state()); 
          delay(2000); 
      }
  }
  // publish and subscribe 
  client.publish(topic, "Salut, Je suis L'ESP32"); 
  client.subscribe(topic);

  //Init des pins
  pinMode(LED, OUTPUT);
  pinMode(ON, INPUT_PULLUP);
  pinMode(VENT, OUTPUT);
}

void loop() {
  client.loop();
}

