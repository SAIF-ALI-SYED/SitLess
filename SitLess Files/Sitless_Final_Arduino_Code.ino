#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm(0x40);

// --- servo settings ---
#define SERVO_FREQ 60
#define SERVOMIN 210
#define SERVOMAX 500
#define S1_CHANNEL 0  // base servo (rotates left/right)
#define S2_CHANNEL 1  // shoulder servo (raises the arm)
#define S3_CHANNEL 2  // elbow servo (does the tapping)

// how fast and smooth the arm moves
#define STEP_DELAY 10
#define STEP_SIZE 2

// arm starts here and returns here after each action
int s1Angle = 180;
int s2Angle = 90;
int s3Angle = 90;
bool stopMotion = false;

// target angles for each servo during a tap
#define S1_TAP_TARGET 40
#define S2_TAP_TARGET 150
#define S3_TAP_TARGET 40

// ultrasonic sensor pins
#define TRIG_PIN 18
#define ECHO_PIN 19
unsigned long lastUltrasonicRead = 0;
const unsigned long ultrasonicInterval = 200;

// buzzer pin
#define BUZZER_PIN 23

// convert an angle (0-180) into a PWM pulse value
uint16_t angleToPulse(int angle) {
  angle = constrain(angle, 0, 180);
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

// move a servo to a given angle
void setServo(uint8_t ch, int angle) {
  angle = constrain(angle, 0, 180);
  pwm.setPWM(ch, 0, angleToPulse(angle));
}

// slowly step a servo from its current angle to the target angle
void moveServoSmooth(uint8_t channel, int &currentAngle, int targetAngle) {
  targetAngle = constrain(targetAngle, 0, 180);
  while (currentAngle != targetAngle && !stopMotion) {
    if (currentAngle < targetAngle) currentAngle = min(currentAngle + STEP_SIZE, targetAngle);
    else currentAngle = max(currentAngle - STEP_SIZE, targetAngle);
    setServo(channel, currentAngle);
    delay(STEP_DELAY);
  }
}

// bring the arm back to its resting position (elbow first, then shoulder, then base)
void goRest() {
  stopMotion = false;
  moveServoSmooth(S3_CHANNEL, s3Angle, 90);
  delay(400);
  moveServoSmooth(S2_CHANNEL, s2Angle, 90);
  delay(500);
  moveServoSmooth(S1_CHANNEL, s1Angle, 180);
  delay(400);
}

// tap sequence: base rotates, shoulder raises, elbow extends, then arm returns
void performTap(int times = 1) {
  stopMotion = false;
  for (int i = 0; i < times && !stopMotion; i++) {
    moveServoSmooth(S1_CHANNEL, s1Angle, S1_TAP_TARGET);
    delay(300);
    if (stopMotion) break;
    moveServoSmooth(S2_CHANNEL, s2Angle, S2_TAP_TARGET);
    delay(300);
    if (stopMotion) break;
    moveServoSmooth(S3_CHANNEL, s3Angle, S3_TAP_TARGET);
    delay(300);
    if (stopMotion) break;
    goRest();
    delay(300);
  }
}

// fire the ultrasonic sensor and return the distance in cm
long readUltrasonicCM() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration / 58;
}

// beep the buzzer twice to alert the user
void beepNotify() {
  digitalWrite(BUZZER_PIN, HIGH); delay(60);
  digitalWrite(BUZZER_PIN, LOW);  delay(40);
  digitalWrite(BUZZER_PIN, HIGH); delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

// initial setup: serial, I2C, pins, and move arm to rest position
void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);
  setServo(S1_CHANNEL, s1Angle);
  setServo(S2_CHANNEL, s2Angle);
  setServo(S3_CHANNEL, s3Angle);
  Serial.println("=== SitLess 3-DOF Ready ===");
  Serial.println("Commands: TAP | REST | BEEP | STOP");
}

// main loop: send distance readings and listen for commands
void loop() {
  if (millis() - lastUltrasonicRead >= ultrasonicInterval) {
    lastUltrasonicRead = millis();
    Serial.print("DIST:");
    Serial.println(readUltrasonicCM());
  }
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim(); cmd.toUpperCase();
    if (cmd == "TAP") { Serial.println("TAP"); performTap(1); }
    else if (cmd == "REST") { Serial.println("REST"); goRest(); }
    else if (cmd == "BEEP") { Serial.println("BEEP"); beepNotify(); }
    else if (cmd == "STOP") { Serial.println("STOP"); stopMotion = true; digitalWrite(BUZZER_PIN, LOW); }
  }
}