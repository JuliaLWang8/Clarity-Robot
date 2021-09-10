#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *left_motor = AFMS.getMotor(1);
Adafruit_DCMotor *right_motor = AFMS.getMotor(2);

Servo door;

#define LOWEST_TRAVERSAL_CONST 7600
#define MEDIAN_TRAVERSAL_CONST 8600
#define HIGHEST_TRAVERSAL_CONST 9600
#define LOWEST_TILE 1
#define MIDDLE_TILE 2
#define HIGHEST_TILE 3

#define DOOR_PIN 10
#define OPEN_DOOR 0
#define CLOSE_DOOR 185

#define DEFAULT_WIGGLE_CYCLE 100
#define MAX_SHAKES 10
#define SENSOR_PIN A0
#define IR_SENSOR_THRESHOLD 700

#define BALL_IN 25
#define BALL_DROPPED 27

#define MISSION_COMPLETED 1
#define MISSION_PENDING 0

#define CONFIG_1_PIN 5
#define CONFIG_2_PIN 6
#define CONFIG_3_PIN 7

int mission_status = MISSION_PENDING;

void setup() {
  // Setup Motor Shield
  AFMS.begin();
  left_motor->setSpeed(255);
  right_motor->setSpeed(255);

  // Setup door config
  door.attach(DOOR_PIN);
  operate_door(CLOSE_DOOR);

  // Setup IR sensor
  pinMode(SENSOR_PIN, INPUT);

  // Setup switch config
  pinMode(CONFIG_1_PIN, INPUT);
  pinMode(CONFIG_2_PIN, INPUT);
  pinMode(CONFIG_3_PIN, INPUT);

  // Pause for dramatic effect
  delay(5000);

//   Simple test case
//  operate_door(OPEN_DOOR);
//  delay(1000);
//  wiggle(DEFAULT_WIGGLE_CYCLE, 10);
//  operate_door(CLOSE_DOOR);
//  delay(1000);
//  Serial.begin(9600);
}

void loop() {
  Serial.println(analogRead(SENSOR_PIN));
  if ((mission_status == MISSION_PENDING) && (check_container_state() == BALL_IN)) {
    int tile = get_target_setting();
//    Serial.print("The target setting is: ");
//    Serial.println(tile);
    drive_to_target(tile);
    delay(500);
    Serial.println("Deploying target");
    mission_status = deploy_target();
    // style_points();
    mission_status = MISSION_COMPLETED;
  }
}

void drive_to_target(int target_id) {
  if (target_id == LOWEST_TILE) { 

    left_motor->run(BACKWARD);
    right_motor->run(BACKWARD);
    delay(LOWEST_TRAVERSAL_CONST);
  }
  else if (target_id == MIDDLE_TILE) {
    left_motor->run(BACKWARD);
    right_motor->run(BACKWARD);
    delay(MEDIAN_TRAVERSAL_CONST);
  }
  else if (target_id == HIGHEST_TILE) {
    left_motor->run(BACKWARD);
    right_motor->run(BACKWARD);
    delay(HIGHEST_TRAVERSAL_CONST);
  }

  left_motor->run(RELEASE);
  right_motor->run(RELEASE);
  delay(10);
  operate_door(OPEN_DOOR);
}

void operate_door(int opcode) {
  door.write(opcode); // Write the servo angle to the door motor
}

void wiggle(int cycle, int num_cycles) {
  for (int i = 0; i < num_cycles; i++) {
    left_motor->run(FORWARD);
    right_motor->run(FORWARD);
    delay(cycle);
    left_motor->run(BACKWARD);
    right_motor->run(BACKWARD);
    delay(cycle);
    left_motor->run(RELEASE);
    right_motor->run(RELEASE);
    delay(10);
  }
}

int check_container_state() {
  if (analogRead(SENSOR_PIN) > IR_SENSOR_THRESHOLD) {
    return BALL_IN;
  }
  return BALL_DROPPED;
}

int deploy_target() { 
  operate_door(OPEN_DOOR);
  int shakes = 0;
  while ((check_container_state() == BALL_IN) && (shakes < MAX_SHAKES)) {
    wiggle(DEFAULT_WIGGLE_CYCLE, 3);
    shakes++;
  }
  operate_door(CLOSE_DOOR);
  return MISSION_COMPLETED;
}

int get_target_setting() {
  if (digitalRead(CONFIG_1_PIN) == HIGH) {
    return LOWEST_TILE;
  }
  if (digitalRead(CONFIG_2_PIN) == HIGH) {
    return MIDDLE_TILE;
  }
  if (digitalRead(CONFIG_3_PIN) == HIGH) {
    return HIGHEST_TILE;
  }
  return MIDDLE_TILE;
}
