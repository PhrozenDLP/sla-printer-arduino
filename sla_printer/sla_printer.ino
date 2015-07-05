#include <Servo.h>
#include <Stepper.h>

#include "mySoftwareSerial.h"
#include "PinChangeInt.h"
#include "AccelStepper.h"
#include "Config.h"

/*** Program variables ***/
// For Serial
char buffer[MAX_BUF];
int sofar;

volatile bool hitUpperLimit = false;
volatile bool hitLowerLimit = false;

// For Stepper 
AccelStepper stepperBase(AccelStepper::DRIVER, STEPS_MOTOR_PIN_1, STEPS_MOTOR_PIN_2);

// Vivitek D517 control command
SoftwareSerial projector(RS232_RX, RS232_TX);
const char on_command[9] =
        {'V', '9', '9', 'S', '0', '0', '0', '1', '\r'};
const char off_command[9] =
        {'V', '9', '9', 'S', '0', '0', '0', '2', '\r'};

void setup()
{
  Serial.begin(SERIAL_BAND);
  while (!Serial) {}
  setupSteppers();
  setupProjector();
  setupLimits();
  help();
  ready();
}

void setupSteppers()
{
  stepperBase.setMaxSpeed(MAX_MOTOR_RPM);
  stepperBase.setAcceleration(MAX_MOTOR_RPM);
  stepperBase.setMinPulseWidth(20);
  stepperBase.setSpeed(160);
}

void setupProjector()
{
  projector.begin(PROJECTOR_BAUD);
}

void setupLimits()
{
  // setup upper limit
  pinMode(UPPER_LIMIT_PIN, INPUT);
  digitalWrite(UPPER_LIMIT_PIN, HIGH);
  PCintPort::attachInterrupt(UPPER_LIMIT_PIN, onHitUpperLimit, RISING);

  // Setup lower limit
  pinMode(LOWER_LIMIT_PIN, INPUT);
  digitalWrite(LOWER_LIMIT_PIN, HIGH);
  PCintPort::attachInterrupt(LOWER_LIMIT_PIN, onHitLowerLimit, RISING);
}

void loop()
{
  // listen for commands
  while(Serial.available() > 0)
  {  // if something is available
    char c = Serial.read();  // get it
    //Serial.print(c);  // repeat it back so I know you got the message
    if(sofar < MAX_BUF) buffer[sofar++] = c;  // store it
    if(buffer[sofar-1] == ';') break;  // entire message received
  }

  if(sofar > 0 && buffer[sofar-1] == ';')
  {
    // we got a message and it ends with a semicolon
    buffer[sofar] = 0;  // end the buffer so string functions work right
    //Serial.print(F("\r\n"));  // echo a return character for humans
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
  Serial.println(F("G02 Z(steps);   - linear move up"));
  Serial.println(F("G03 Z(steps);   - linear move down"));
  Serial.println(F("G04 P(seconds); - delay"));
  Serial.println(F("G50;            - Send power on command"));
  Serial.println(F("G51;            - Send power off command"));
  Serial.println(F("M02 R(rpm);     - set motor speed (rpm)"));
  Serial.println(F("M99;            - version info"));
  Serial.println(F("M100;           - this help message"));
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
  int cmd = parsenumber('G', -1);
  int arg_value = 0;
  switch(cmd) {
  case  0:  break;  // move in a line
  case  2:  move_up(parsenumber('Z', 0));  break;  // Up
  case  3:  move_down(parsenumber('Z', 0));  break;  // Down
  case  4:  pause(parsenumber('P', 0) * 1000);  break;  // wait a while
  case 50:  projector_on(); break; // Turn on projector
  case 51:  projector_off(); break; // Turn off projector
  default:  break;
  }

  // look for commands that start with 'M'
  cmd = parsenumber('M', -1);
  switch(cmd) {
  case   2:
    arg_value = parsenumber('R', 0);
    if (arg_value > MAX_MOTOR_RPM) {
      arg_value = MAX_MOTOR_RPM;
    }
    set_stepper_speed(arg_value);
    break;
  case  99:  Serial.println(F(VERSION));  break;
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
  hitLowerLimit = false;
  if (hitUpperLimit)
  {
    return;
  }
  for (int i = 0; i < steps; i++)
  {
    if (hitUpperLimit) {
      break;
    }
    drive_motor(stepperBase, 1);
  }
}

void move_down(int steps)
{
  hitUpperLimit = false;
  if (hitLowerLimit)
  {
    return;
  }
  for (int i = 0; i < steps; i++)
  {
    if (hitLowerLimit) {
      break;
    }
    drive_motor(stepperBase, -1);
  }
}

void drive_motor(AccelStepper motor, int steps)
{
  motor.move(steps);
  while (has_steps(motor))
  {
    motor.run();
  }
}

boolean has_steps(AccelStepper motor)
{
  return motor.distanceToGo() != 0;
}

void projector_on()
{
  for (int i = 0; i < sizeof(on_command); i++)
  {
    projector.write((byte)on_command[i]);
  }
}

void projector_off()
{
  for (int i = 0; i < sizeof(off_command); i++) {
    projector.write((byte)off_command[i]);
  }
}

void onHitUpperLimit()
{
  hitUpperLimit = true;
}

void onHitLowerLimit()
{
  hitLowerLimit = true;
}

void set_stepper_speed(int rpm)
{
  stepperBase.setSpeed(rpm);
}
