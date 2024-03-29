#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define LED 2
#define ON 4
#define VENT 5

// Paramètres WiFi
const char* ssid = "Pixel_2035";
const char* password = "chauffage";

// Paramètres du broker MQTT
const char* mqtt_server = "3f68ce49b7714ea2ac988e755d35fd99.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // Port MQTT avec SSL

// Identifiants MQTT
const char* mqtt_user = "chauffapagnan";
const char* mqtt_password = "Chauffapagnan24";

WiFiClientSecure espClient;
PubSubClient client(espClient);

bool on = false;

void setup_wifi() {
  delay(10);
  // Connexion au réseau WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char msg[length];
  for (int i = 0; i < length; i++) { 
    msg[i] = (char) payload[i];
  } 
  msg[length] = '\0';

  Serial.print(msg);
  Serial.print("\n");

  if(strcmp(msg, "1")==0){
    Serial.print("Je turn on\n");
    on = true;
  }

  if(strcmp(msg, "0")==0){
    Serial.print("Je turn off\n");
    on = false;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("DATA", "hello world");
      // ... and resubscribe
      client.subscribe("CONTROL");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Configuration de la connexion SSL/TLS
  espClient.setInsecure();
  //espClient.setCACert(nullptr); // Aucun certificat requis

    //Init des pins
  pinMode(LED, OUTPUT);
  pinMode(ON, INPUT_PULLUP);
  pinMode(VENT, OUTPUT);
}

void loop() {

  //Gestion des messages
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Gestion allumage manuel || GROS PROBLème SAVOIR COMMENT ON SYNCHRONISE
  /*
  if(digitalRead(ON)){
    on = true;
  }else{

  }
  */

  //Gestion allumage
  if(on == 1){
    Serial.println("Allumage");
    digitalWrite(LED, HIGH);
    digitalWrite(VENT, HIGH);
  }
  else{
    digitalWrite(LED, LOW);
    digitalWrite(VENT, LOW);
  }
}
