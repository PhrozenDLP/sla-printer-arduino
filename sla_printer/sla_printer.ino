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
#define SERIAL_BAND    (9600)
#define MAX_BUF        (64)

// For Stepper motor
#define STEPS_PER_MOTOR_REVOLUTION    (32)
#define STEPS_PER_OUTPUT_REVOLUTION   (32 * 64) // 2048
#define STEPPER_SPEED                 (700)

// For Servo motor
#define SERVO_ANGLE_MIN    (0)
#define SERVO_ANGLE_MAX    (180)
#define SERVO_ANGLE_STEP   (5)


/*** Program variables ***/
// For Serial
char buffer[MAX_BUF];
int sofar;

// For Stepper motor
Stepper stepper(STEPS_PER_MOTOR_REVOLUTION, 8, 10, 9, 11);
int Steps2Take;
char command;

// For Servo motor
int angle = SERVO_ANGLE_MIN;

// Misc
char mode_abs = 1;    // absolute mode
float px, py;         // location
float fr = 0;         // speeds

void setup() {
  Serial.begin(SERIAL_BAND);
  while (!Serial) {}
  help();
  ready();
}

void loop() {
  // listen for commands
  while(Serial.available() > 0) {  // if something is available
    char c=Serial.read();  // get it
    Serial.print(c);  // repeat it back so I know you got the message
    if(sofar<MAX_BUF) buffer[sofar++]=c;  // store it
    if(buffer[sofar-1]==';') break;  // entire message received
  }

  if(sofar>0 && buffer[sofar-1]==';') {
    // we got a message and it ends with a semicolon
    buffer[sofar]=0;  // end the buffer so string functions work right
    Serial.print(F("\r\n"));  // echo a return character for humans
    processCommand();  // do something with the command
    ready();
  }
}

// Print help messages
void help() {
  Serial.print(F("SLA Printer ver: "));
  Serial.println(VERSION);
  Serial.println(F("Commands:"));
  Serial.println(F("G00 [X(steps)] [Y(steps)] [F(feedrate)]; - linear move"));
  Serial.println(F("G01 [X(steps)] [Y(steps)] [F(feedrate)]; - linear move"));
  Serial.println(F("G04 P[seconds]; - delay"));
  Serial.println(F("G90; - absolute mode"));
  Serial.println(F("G91; - relative mode"));
  Serial.println(F("G92 [X(steps)] [Y(steps)]; - change logical position"));
  Serial.println(F("M18; - disable motors"));
  Serial.println(F("M100; - this help message"));
  Serial.println(F("M114; - report position and feedrate"));
}

/**
 * prepares the input buffer to receive a new message
 * and tells the serial connected device it is ready for more.
 */
void ready() {
  sofar=0;  // clear input buffer
  Serial.print(F(">"));  // signal ready to receive input
}

/**
 * Read the input buffer and find any recognized commands.  One G or M command per line.
 */
void processCommand() {
  // look for commands that start with 'G'
  int cmd=parsenumber('G',-1);
  switch(cmd) {
  case  0: // move in a line
//  case  1: // move in a line
//    feedrate(parsenumber('F',fr));
//    line( parsenumber('X',(mode_abs?px:0)) + (mode_abs?0:px),
//          parsenumber('Y',(mode_abs?py:0)) + (mode_abs?0:py) );
//    break;
  case  2: // clockwise arc
  case  3: // counter-clockwise arc
  case  4:  pause(parsenumber('P',0)*1000);  break;  // wait a while
  case 90:  mode_abs=1;  break;  // absolute mode
  case 91:  mode_abs=0;  break;  // relative mode
  case 92:  // set logical position
//    position( parsenumber('X',0),
//              parsenumber('Y',0) );
    break;
  default:  break;
  }

  // look for commands that start with 'M'
  cmd=parsenumber('M',-1);
  switch(cmd) {
  case 18:  stepper.release();  break;  // turns off power to steppers (releases the grip)
  case 100:  help();  break;  // print help
  case 114:  where();  break;  // prints px, py, fr, and mode.
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
float parsenumber(char code, float val) {
  char *ptr=buffer;
  while(ptr && *ptr && ptr<buffer+sofar) {
    if(*ptr==code) {
      return atof(ptr+1);
    }
    ptr=strchr(ptr,' ')+1;
  }
  return val;
}

void where() {
  output("X",px);
  output("Y",py);
  output("F",fr);
  Serial.println(mode_abs ? "ABS":"REL");
}

/**
 * write a string followed by a float to the serial line.  Convenient for debugging.
 * @input code the string.
 * @input val the float.
 */
void output(char *code, float val) {
  Serial.print(code);
  Serial.println(val);
}

/**
 * Pause devices
 * @input wait_millis the milli-seconds to wait
 */
 void pause(int wait_millis) {
   delay(wait_millis);
 }
