#include "MotorGroup.h"

/*** BOARD A ***/
// MOTOR[0] (claw)        // Wire Colour
#define MOTOR_CLAW_A 3    // Purpel
#define MOTOR_CLAW_B 5    // Gray

// MOTOR[1] (uper motor)
#define MOTOR_UPER_A 6    // Yelloo
#define MOTOR_UPER_B 9    // Orange

/*** BOARD B ***/
// MOTOR [0] (lower motor)
#define MOTOR_LOWER_A 10  // Blue
#define MOTOR_LOWER_B 11  // Brown

// MOTOR [1] (rotate motor)
#define MOTOR_ROTATE_A 12 // Green
#define MOTOR_ROTATE_B 13 // White

#define LED_IS_LOWER  7
#define LED           A3

// Inputs
#define JOY_Y_TOGGLE  2
#define LED_TOGGLE    A0
#define CLAW_OPEN     A1
#define CLAW_CLOSE    A2
#define JOY_X         A4
#define JOY_Y         A5

MotorGroup board_a[] { MotorGroup(MOTOR_CLAW_A , MOTOR_CLAW_B),   MotorGroup(MOTOR_UPER_A , MOTOR_UPER_B)   };
MotorGroup board_b[] { MotorGroup(MOTOR_LOWER_A, MOTOR_LOWER_B), MotorGroup(MOTOR_ROTATE_A, MOTOR_ROTATE_B) };

int moter_speed = 175;

bool joy_y_is_lower = false;
bool joy_Y_button_pressed = false;

void setup() {
  Serial.begin(9600);

  // Inputs
  pinMode(JOY_Y_TOGGLE, INPUT_PULLUP);
  pinMode(LED_TOGGLE, INPUT);
  pinMode(CLAW_CLOSE, INPUT);
  pinMode(CLAW_OPEN, INPUT);
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  // Outputs
  // Board A
  pinMode(MOTOR_CLAW_A, OUTPUT);
  pinMode(MOTOR_CLAW_B, OUTPUT);
  pinMode(MOTOR_UPER_A, OUTPUT);
  pinMode(MOTOR_UPER_B, OUTPUT);
  
  // Board B
  pinMode(MOTOR_LOWER_A , OUTPUT);
  pinMode(MOTOR_LOWER_B , OUTPUT);
  pinMode(MOTOR_ROTATE_A, OUTPUT);
  pinMode(MOTOR_ROTATE_B, OUTPUT);

}

void loop() {
  
  update_joy_y();
  update_LED();
  update_claw();
  update_uper();
  update_lower();
  update_rotate();

}

void update_joy_y()
{
  if ( !joy_Y_button_pressed && digitalRead(JOY_Y_TOGGLE) == LOW )
  {
      joy_y_is_lower = !joy_y_is_lower;
      joy_Y_button_pressed = true;
  }
  else if( joy_Y_button_pressed && digitalRead(JOY_Y_TOGGLE) == HIGH )
  {
    joy_Y_button_pressed = false;
  }
  
  // Toggle the LED
  if ( joy_y_is_lower )
    digitalWrite(LED_IS_LOWER, HIGH);
  else
    digitalWrite(LED_IS_LOWER, LOW);
      
}

void update_LED()
{
    if ( analogRead(LED_TOGGLE) > 100 )
      analogWrite(LED, 255);
    else if (analogRead(LED_TOGGLE) < 100)
      analogWrite(LED, 0);
}

void update_claw()
{
  if ( analogRead(CLAW_CLOSE) > 100 )
    run_motor(board_a, 0, moter_speed, 1);
  else if (analogRead(CLAW_OPEN) > 100)
    run_motor(board_a, 0, moter_speed, -1);
  else
  run_motor(board_a, 0, 0, 0);
}

void update_uper()
{
    float joystick_position = -( ( analogRead( JOY_Y ) - 512.0f ) / 512.0f );   // in range of 0 - 1
    
    if ( !joy_y_is_lower && abs(joystick_position) > 0.1f )
    {
      int move_dir = 1;
      if ( joystick_position < 0 )
          move_dir = -1;
              
      run_motor(board_a, 1, moter_speed * abs( joystick_position ), move_dir);
    }
    else
    {
      run_motor( board_a, 1, 0, 0 );
    }
}

void update_lower()
{
    
    float joystick_position = -( ( analogRead( JOY_Y ) - 512.0f ) / 512.0f );   // in range of 0 - 1
    
    if ( joy_y_is_lower && abs(joystick_position) > 0.1f )
    {
      int move_dir = 1;
      if ( joystick_position < 0 )
          move_dir = -1;
              
      run_motor(board_b, 0, moter_speed * abs( joystick_position ), move_dir);
    }
    else
    {
      run_motor( board_b, 0, 0, 0 );
    }
    
}

void update_rotate()
{
    float joystick_position = ( ( analogRead( JOY_X ) - 512.0f ) / 512.0f );   // in range of 0 - 1

    if ( abs(joystick_position) > 0.1f )
    {
      int move_dir = 1;
      if ( joystick_position < 0 )
          move_dir = -1;
          
      run_motor(board_b, 1, 255, move_dir);
    }
    else
    {
      run_motor( board_b, 1, 0, 0 );
    }
    
}

void run_motor(MotorGroup m_group[], int active_id, int motor_speed, int motor_direction)
{
  /* So the drive boards that we are using work as so: (L9110 2-CHANNEL MOTOR DRIVER) (Datasheet: http://me.web2.ncut.edu.tw/ezfiles/39/1039/img/617/L9110_2_CHANNEL_MOTOR_DRIVER.pdf)
  ** (Both sides) if 'A' is HIGH 'B' must be LOW and vice versa, ie.
  ** (Both sides) [Forwards] 'A' is HIGH 'B' is LOW
  ** (Both sides) [Backwards] 'B' is HIGH 'A' is LOW
  ** (Sigle Side Only, with 2 inputs) if only want one side to run and the other not, the side that is not running must both 'A' and 'B' set HIGH       
  ** (Cheers Datasheet :| for documenting that )
  */
  
  int other_id = abs(active_id - 1);
  bool set_other_high = m_group[active_id].otherHigh(&m_group[other_id]);

  int motor_a_speed = 0;
  int motor_b_speed = 0;

  if ( motor_direction > 0 )
    motor_a_speed = motor_speed;
  else if ( motor_direction < 0)
    motor_b_speed = motor_speed;

  m_group[active_id].set_speed( motor_a_speed, motor_b_speed );

  if (m_group[active_id].idle) return;

  analogWrite(m_group[active_id].motor_a, motor_a_speed);
  analogWrite(m_group[active_id].motor_b, motor_b_speed);

  if (motor_a_speed == 0 && motor_b_speed == 0)
      m_group[active_id].set_idle();
  
  if( set_other_high )
  {
    analogWrite(m_group[other_id].motor_a, 255);
    analogWrite(m_group[other_id].motor_b, 255);
  }
  
}
