#include <Stepper.h>
#include <Servo.h>
#include "config.h"
#include "AccelStepper.h"

/*** Program variables ***/
// For Serial
char buffer[MAX_BUF];
int sofar;

// For Stepper 
AccelStepper stepperBase(AccelStepper::DRIVER, STEPS_MOTOR_PIN_1, STEPS_MOTOR_PIN_2);

// For Rotation
AccelStepper stepperRotate(AccelStepper::DRIVER, STEPS_MOTOR_PIN_3, STEPS_MOTOR_PIN_4);

void setup()
{
  Serial.begin(SERIAL_BAND);
  while (!Serial) {}
  setupSteppers();
  help();
  ready();
}

void setupSteppers() {
  stepperBase.setMaxSpeed(40000);
  stepperBase.setAcceleration(40000);
  stepperBase.setSpeed(40000);
  stepperRotate.setMaxSpeed(MAX_SPEED);
  stepperRotate.setAcceleration(ACC_SPEED);
  stepperRotate.setSpeed(MAX_SPEED);
}

void loop()
{
  // listen for commands
  while(Serial.available() > 0)
  {  // if something is available
    char c = Serial.read();  // get it
    Serial.print(c);  // repeat it back so I know you got the message
    if(sofar < MAX_BUF) buffer[sofar++] = c;  // store it
    if(buffer[sofar-1] == ';') break;  // entire message received
  }

  if(sofar > 0 && buffer[sofar-1] == ';')
  {
    // we got a message and it ends with a semicolon
    buffer[sofar] = 0;  // end the buffer so string functions work right
    Serial.print(F("\r\n"));  // echo a return character for humans
    processCommand();  // do something with the command
    ready();
  }
}

// Print help messages
void help()
{
  Serial.print(F("SLA Printer ver: "));
  Serial.println(VERSION);
  Serial.println(F("Commands:"));
  Serial.println(F("G02 [Z(steps)]; - linear move up"));
  Serial.println(F("G03 [Z(steps)]; - linear move down"));
  Serial.println(F("G04 P[seconds]; - delay"));
  Serial.println(F("M02 [Z(steps)]; - rotate servo"));
  Serial.println(F("M03 [Z(steps)]; - rotate servo"));
  Serial.println(F("M100; - this help message"));
}

/**
 * prepares the input buffer to receive a new message
 * and tells the serial connected device it is ready for more.
 */
void ready()
{
  sofar=0;  // clear input buffer
  Serial.print(F(">"));  // signal ready to receive input
}

/**
 * Read the input buffer and find any recognized commands.  One G or M command per line.
 */
void processCommand()
{
  // look for commands that start with 'G'
  int cmd = parsenumber('G',-1);
  int arg_value = 0;
  switch(cmd) {
  case  0:  break;  // move in a line
  case  2:  move_up(parsenumber('Z', 0));  break;  // Up
  case  3:  move_down(parsenumber('Z', 0) * -1);  break;  // Down
  case  4:  pause(parsenumber('P', 0) * 1000);  break;  // wait a while
    break;
  default:  break;
  }

  // look for commands that start with 'M'
  cmd=parsenumber('M', -1);
  switch(cmd) {
  case  2:
    rotateCW(parsenumber('Z', 0));
    break;
  case  3:
    rotateCCW(parsenumber('Z', 0));
    break;
  case 10:
    Serial.print("pos: ");
    Serial.println(stepperRotate.currentPosition());
    Serial.print("target: ");
    Serial.println(stepperRotate.targetPosition());
    break;
  case 100:  help();  break;  // print help
  default:  break;
  }

  // if the string has no G or M commands it will get here and the Arduino will silently ignore it
}

/**
 * Look for character /code/ in the buffer and read the float that immediately follows it.
 * @return the value found.  If nothing is found, /val/ is returned.
 * @input code the character to look for.
 * @input val the return value if /code/ is not found.
 **/
float parsenumber(char code, float val)
{
  char *ptr=buffer;
  while(ptr && *ptr && ptr<buffer+sofar)
  {
    if(*ptr==code)
    {
      return atof(ptr+1);
    }
    ptr=strchr(ptr,' ')+1;
  }
  return val;
}

/**
 * write a string followed by a float to the serial line.  Convenient for debugging.
 * @input code the string.
 * @input val the float.
 */
void output(char *code, float val)
{
  Serial.print(code);
  Serial.println(val);
}

/**
 * Pause devices
 * @input wait_millis the milli-seconds to wait
 */
void pause(int wait_millis)
{
  delay(wait_millis);
}

void move_up(int steps)
{
  drive_motor(stepperBase, steps);
}

void move_down(int steps)
{
  drive_motor(stepperBase, -1 * steps);
}

void rotateCW(int steps)
{
  drive_motor(stepperRotate, steps);
}

void rotateCCW(int steps) 
{
  drive_motor(stepperRotate, -1 * steps);
}

void drive_motor(AccelStepper motor, int steps)
{
  motor.move(steps);
  while (has_steps(motor))
  {
    motor.runSpeedToPosition();
  }
}

boolean has_steps(AccelStepper motor)
{
  return motor.distanceToGo() != 0;
}
