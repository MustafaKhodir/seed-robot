// Seed dispenser, servo only.
// This was the first version I tested. The 9g servo sits inside the 3D printed
// pot and opens the gate for a short burst so a few seeds drop out, then closes.
// One full cycle takes 2 seconds (300 ms open + 1700 ms closed).

#include <Servo.h>

Servo myservo;  // Creates a servo object to control the rotor

const int servoPin = 8;
const int openAngle = 65;   // gate open position in degrees
const int closedAngle = 0;  // gate closed position in degrees

void setup() {
  myservo.attach(servoPin);
  myservo.write(closedAngle);
}

void loop() {
  myservo.write(openAngle);   // Servo motor opens to a 65 degree angle
  delay(300);                 // Waits for 300 milliseconds so the seeds drop

  myservo.write(closedAngle); // Returns the rotor to its initial state (0 degrees)
  delay(1700);                // Then waits for 1.7 seconds before repeating the loop
}
