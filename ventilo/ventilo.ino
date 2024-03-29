#define LED 2
#define ON 4
#define VENT 5


void setup() {
  //Init des pins
  pinMode(LED, OUTPUT);
  pinMode(ON, INPUT_PULLUP);
  pinMode(VENT, OUTPUT);
}

void loop() {
  if(digitalRead(ON)){
    digitalWrite(LED, HIGH);
    digitalWrite(VENT, HIGH);
  }
  else{
    digitalWrite(LED, LOW);
    digitalWrite(VENT, LOW);
  }
}
