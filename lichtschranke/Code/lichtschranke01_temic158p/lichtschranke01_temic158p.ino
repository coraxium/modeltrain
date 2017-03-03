

#define PIN 5
#define LED 13 
int i=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Ready");
  
  pinMode(PIN, INPUT);
  // pinMode(LED, OUTPUT);      // Pin der LED als Ausgang
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.print(i++);
  Serial.print("\t"); 
  Serial.println(digitalRead(PIN));
  
  setPowerState(digitalRead(PIN));
  
  delay(500);
}



// Sets the LED for the power state -- 0 is OFF, 1 is ON
void setPowerState(bool state)
{
  if (state)
  {
    digitalWrite(LED, HIGH);  // set LED to on
  }
  else
  {
    digitalWrite(LED, LOW);  // set LED to off
  }
}
