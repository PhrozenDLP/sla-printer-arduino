#include <Stepper.h>
#include <Servo.h>

/*** Program constants ***/
#define VERSION        (1)

// Pin definition
#define STEPS_MOTOR_PIN_1  (8)
#define STEPS_MOTOR_PIN_2  (10)
#define STEPS_MOTOR_PIN_3  (9)
#define STEPS_MOTOR_PIN_4  (11)

#define SERVO_MOTOR_PIN    (12)

// For Serial
#define BAUD           (9600)
#define MAX_BUF        (64)

// For Stepper motor
#define STEPS_PER_MOTOR_REVOLUTION 32
#define STEPS_PER_OUTPUT_REVOLUTION 32 * 62 // 2048

// For Servo motor
#define SERVO_ANGLE_MIN    (0)
#define SERVO_ANGLE_MAX    (180)
#define SERVO_ANGLE_STEP   (5)


/*** Program variables ***/
// For Serial
char buffer[MAX_BUF];
int sofar;

// For Stepper motor
Stepper small_stepper(STEPS_PER_MOTOR_REVOLUTION, 8, 10, 9, 11);
int Steps2Take;
char command;

// For Servo motor
int angle = SERVO_ANGLE_MIN;

// Misc
char mode_abs = 1;    // absolute mode
float px, py;         // location
float fr = 0;         // speeds

void setup() {
}

void loop() {
}
