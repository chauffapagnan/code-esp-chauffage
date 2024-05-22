#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include <iostream>
#include <string> // Pour std::string, std::to_string

#include <DHT.h>
#include <SPI.h>

//LCD
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int colorR = 0;
const int colorG = 255;
const int colorB = 0;

#define LED 2
#define ON 4
#define VENT 5

#define TIN 33 //Sensor v1.2
#define TOUT 32 //DHT22
#define DHTTYPE DHT22

DHT dht(TOUT, DHTTYPE);

//CONSTANTES CALCUL ENERGIE
#define CAPA_THERMIQUE_AIR 1005
#define DEBIT_AIR 0.0218

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

//tempin
const float B = 4275.0;
const float R0 = 100000.0;

float temp_p = 0.0;
bool automatic = false; //gère si notre chauffage fonctionne en mode automatique ou manuel

//tempout

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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("INIT_ESP", "hello world");
      // ... and resubscribe
      client.subscribe("CONTROL/ONOFF");
      client.subscribe("CONTROL/TEMP");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//--------------------------------------------------------------------------------------------------------------------------------------------

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  //transforme le message en string
  char msg[length];
  for (int i = 0; i < length; i++) { 
    msg[i] = (char) payload[i];
  } 
  msg[length] = '\0';
  Serial.print(msg);
  Serial.print("\n");

  if(strcmp(topic, "CONTROL/ONOFF")==0){
    //comparaison pour savoir si on allume ou éteint
    automatic = false;
    if(strcmp(msg, "1")==0){
      setOn();
    }
    if(strcmp(msg, "0")==0){
      setOff();
    }
  }
  else if(strcmp(topic, "CONTROL/TEMP")==0){
    try{
      float t = std::stof(msg);
      setTemp(t);
    }
    catch (const std::invalid_argument& e){
      Serial.println("Erreur : entrée invalide");
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

  pinMode(TIN, INPUT);
  //pinMode(TOUT, INPUT);
  dht.begin();

  // set up the LCD's number of columns and rows:
  //lcd.begin(16, 2);
    
  //lcd.setRGB(colorR, colorG, colorB);
  //lcd.setCursor(0, 0);
  // Print a message to the LCD.
  //lcd.print("hello, world!");
}

void loop() {
  //Gestion des messages
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Serial.println(digitalRead(ON));
  
  //Gestion allumage manuel
  if (digitalRead(ON) == LOW){// Si le bouton est enfoncé (bas)
    automatic = false;
    Serial.print("Etat de l'interrupteur : ");
    Serial.println(on ? "ON" : "OFF");
    if(on == 1){
      setOff();
    }
    else{
      setOn();
    }
    delay(500);
  }

  if(automatic == 1){
    checkTemp();
  }

}

void setOn(){
  on = true;
  Serial.print("Je turn on\n");
  digitalWrite(LED, HIGH);
  digitalWrite(VENT, HIGH);
  client.publish("CONTROL/ACK", "1");
}

void setOff(){
  on = false;
  Serial.print("Je turn off\n");
  digitalWrite(LED, LOW);
  digitalWrite(VENT, LOW);
  client.publish("CONTROL/ACK", "0");
}

//Automatique
void setTemp(float t){
  automatic = true;
  temp_p = static_cast<int>(t * 10) / 10.0;
  Serial.print("Je mets la temp_p à ");
  Serial.print(temp_p);
  Serial.println(" °C");

  char buffer[10]; // Définir une taille suffisante pour contenir votre nombre avec sa partie décimale

  // Utiliser dtostrf() pour convertir le float en string
  dtostrf(temp_p, 3, 1, buffer); // 6 : largeur totale, 2 : nombre de décimales
  client.publish("DATA/TEMP-TARGET", buffer);
}

void  checkTemp(){
  // Gestion température
  float tin = analogRead(TIN);
  float R = 4095.0/tin-1.0;
  R = R0*R;
  float temp_c = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

  //Gestion allumage avec température
  int delta = 2;
  if(temp_c < temp_p && on == 0){
    setOn();
  }
  else if(temp_c > temp_p + delta && on == 1){ //à voir si on rajoute -delta
    setOff();
  }

  Serial.print("temp_c à ");
  Serial.print(temp_c);
  Serial.print(" °C  -  ");


  /* CODE LM35
  //temp_o
  float tout = analogRead(TOUT);
  float temp_o = tout * 3.3/40.96;
  */
  delay(2000);
  float temp_o = dht.readTemperature();
  Serial.print("temp_o à ");
  Serial.print(temp_o);
  Serial.println(" °C");
}
