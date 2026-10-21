// Seed dispenser with an LED that shows when the gate is open.
// Same 2 second cycle as seed_dispenser, I just added an LED on pin 13 so I
// could see from across the lab when the servo was dispensing.

#include <Servo.h>

Servo myservo;  // Creates a servo object to control the rotor

const int servoPin = 8;
const int ledPin = 13;      // LED that is on while the gate is open
const int openAngle = 65;
const int closedAngle = 0;

void setup() {
  myservo.attach(servoPin);
  myservo.write(closedAngle);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  myservo.write(openAngle);     // Servo motor opens to a 65 degree angle
  digitalWrite(ledPin, HIGH);   // LED on while seeds are dropping
  delay(300);

  myservo.write(closedAngle);   // Back to 0 degrees, gate closed
  digitalWrite(ledPin, LOW);
  delay(1700);                  // Wait 1.7 seconds before the next dispense
}
