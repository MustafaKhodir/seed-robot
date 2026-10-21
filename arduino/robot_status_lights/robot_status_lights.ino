// Final Arduino sketch for the seed robot.
//
// Three jobs run on the Uno that rides on top of the TurtleBot:
//   1. Read the MPU6050 IMU and light a green LED when the robot is moving
//      forward and a red LED when it is reversing, so people nearby can tell
//      what the robot is doing.
//   2. Blink a yellow LED on and off every 0.5 seconds the whole time, as a
//      general "robot is active" warning.
//   3. Open and close the seed pot gate with the 9g servo, 300 ms open then
//      1700 ms closed, so seeds drop every 2 seconds.
//
// Wiring (Arduino Uno):
//   MPU6050 VCC -> 5V, GND -> GND, SDA -> A4, SCL -> A5
//   Red LED    -> pin 12
//   Yellow LED -> pin 13
//   Green LED  -> pin 11
//   Servo signal -> pin 8, servo power from 5V and GND
//
// Library: "MPU6050" (the one that gives you mpu.initialize() and
// mpu.getAcceleration), installed through the Arduino Library Manager.

#include <Wire.h>
#include <MPU6050.h>
#include <Servo.h>

Servo myservo;  // Creates a servo object to control the motor
MPU6050 mpu;

// LED and servo pins
const int redLedPin = 12;
const int yellowLedPin = 13;
const int greenLedPin = 11;
const int servoPin = 8;

// Servo positions
const int openAngle = 65;   // gate open
const int closedAngle = 0;  // gate closed

// Raw accelerometer reading on the Y axis above this means forward,
// below the negative of it means backwards. I picked 3000 after watching
// the readings in the Serial Plotter while pushing the robot by hand.
const int moveThreshold = 3000;

void setup() {
  Serial.begin(9600);

  // Initialize MPU6050 over I2C
  Wire.begin();
  mpu.initialize();

  // Initialize LED pins as outputs
  pinMode(redLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  myservo.attach(servoPin);
  myservo.write(closedAngle);
}

void loop() {
  // Reads the accelerometer data from the IMU (three axes)
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Work out which way the TurtleBot is moving from the Y axis
  if (ay > moveThreshold) {               // moving forward, green on
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
  } else if (ay < -moveThreshold) {       // moving backwards, red on
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
  } else {                                // no real motion, both off
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, LOW);
  }

  controlMotor();   // open and close the seed gate
  blinkYellow();    // flash the yellow warning light

  // Print the Y reading so it can be watched in the Serial Plotter
  Serial.println(ay);
}

// Seed dispensing time is 2 seconds per dispense
void controlMotor() {
  myservo.write(openAngle);     // Servo motor opens to a 65 degree angle
  delay(300);                   // Waits for 300 milliseconds so the seeds drop

  myservo.write(closedAngle);   // Returns the rotor to its initial state (0 degrees)
  delay(1700);                  // Then waits for 1.7 seconds before repeating the loop
}

// Yellow LED blinks 0.5 second on, 0.5 second off
void blinkYellow() {
  digitalWrite(yellowLedPin, HIGH); // Yellow LED is turned on
  delay(500);                       // for 0.5 second

  digitalWrite(yellowLedPin, LOW);  // Turn off the yellow LED
  delay(500);                       // for 0.5 second
}
