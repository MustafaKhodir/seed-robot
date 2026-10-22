// Final Arduino sketch for the seed robot.
//
// Three jobs run together on the Uno that rides on top of the TurtleBot:
//   1. Read the MPU6050 IMU and light a green LED when the robot accelerates
//      forwards and a red LED when it accelerates backwards, so people nearby
//      can see what the robot is doing.
//   2. Blink a yellow LED on and off every 0.5 seconds the whole time, as a
//      general "robot is active" warning.
//   3. Open and close the seed pot gate with the 9g servo, 300 ms open then
//      1700 ms closed, so seeds drop every 2 seconds.
//
// The servo and the yellow LED run on millis() timers instead of delay(), so
// neither one holds up the other and the IMU is read on every pass of the loop.
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

// Servo positions and timing (2 second dispense cycle)
const int openAngle = 65;
const int closedAngle = 0;
const unsigned long openTime = 300;     // ms the gate stays open
const unsigned long closedTime = 1700;  // ms the gate stays closed

// Yellow LED blink timing
const unsigned long blinkTime = 500;    // ms on, then ms off

// Raw accelerometer reading on the Y axis above this means accelerating
// forwards, below the negative of it means accelerating backwards
const int moveThreshold = 3000;

// timers for the non blocking parts
unsigned long lastServoChange = 0;
bool gateOpen = false;
unsigned long lastBlinkChange = 0;
bool yellowOn = false;

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

  // Work out which way the TurtleBot is accelerating from the Y axis
  if (ay > moveThreshold) {               // forwards, green on
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
  } else if (ay < -moveThreshold) {       // backwards, red on
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
  } else {                                // no real acceleration, both off
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, LOW);
  }

  controlMotor();   // open and close the seed gate
  blinkYellow();    // keep the yellow warning light flashing

  // Print the Y reading so it can be watched in the Serial Plotter
  Serial.println(ay);
}

// Seed dispensing: 300 ms open, 1700 ms closed, repeated
void controlMotor() {
  unsigned long now = millis();
  if (gateOpen && now - lastServoChange >= openTime) {
    myservo.write(closedAngle);   // back to 0 degrees
    gateOpen = false;
    lastServoChange = now;
  } else if (!gateOpen && now - lastServoChange >= closedTime) {
    myservo.write(openAngle);     // open to 65 degrees
    gateOpen = true;
    lastServoChange = now;
  }
}

// Yellow LED blinks 0.5 second on, 0.5 second off
void blinkYellow() {
  unsigned long now = millis();
  if (now - lastBlinkChange >= blinkTime) {
    yellowOn = !yellowOn;
    digitalWrite(yellowLedPin, yellowOn ? HIGH : LOW);
    lastBlinkChange = now;
  }
}
