/*** Program constants ***/
#define VERSION        "0.2.0"

// Pin definition
#define STEPS_MOTOR_PIN_1  (8)
#define STEPS_MOTOR_PIN_2  (9)
#define UPPER_LIMIT_PIN    (15)
#define LOWER_LIMIT_PIN    (16)

#define STEPS_MOTOR_PIN_3  (10)
#define STEPS_MOTOR_PIN_4  (11)
#define MAX_SPEED (200)
#define ACC_SPEED (200)

// Projector
#define RS232_TX  (7)  // TX of RS232 board (my board)
#define RS232_RX  (6)  // RX of RS232 board (my board)
#define PROJECTOR_BAUD (9600)

// For Serial
#define SERIAL_BAND    (9600)
#define MAX_BUF        (128)

// For Stepper motor
#define STEPS_PER_MOTOR_REVOLUTION    (200)  // 360 / 1.8 = 200
#define STEPPER_SPEED                 (40000)

