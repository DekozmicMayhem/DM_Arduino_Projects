/*
 Name:		Boebot_LineFollower.ino
 Created:	3/28/2019 4:22:29 PM
 Author:	gsdec
*/

/* This is a line following sketch with 2 LED's attached to illuminate the sensor values.              Created by Gary Decosmo 4/02/2018
 *  The Robot uses a 5 photoresistor lateral sensor array                                              * Updated 4/22/2018 @ 2233
 *  A calibration will be needed to ensure optimal functionality when changing environments.
 *  A filter to shield the sensor array from ambient light to keep values consistent is recommended.
 *  This is an ongoing project and will be updated with improvements.
 *
 *
 */
 //***********************************************************   Functions Index  **************************************************************************************//
 /*
  * headlights_on()
  * left_whiskerPressed()
  * right_whiskerPressed()
  * both_whiskerPressed()
  * no_whiskerPressed()
  * line_follow()
  * sensor_calc()
  * slow_forward()
  * slow_right()
  * slow_left()
  * hard_left()                                                        BOUNDARIES
  * sharp_left()
  * hard_right()                                     || -9999  -2     -1    0     1      2   9999  ||
  * sharp_right()                                    ||   LB   LMB   cLMB   MB  cRMB    RMB   RB   ||
  * backup(int time)
  * pivot_right(int time)
  * pivot_left(int time)
  * wide_right_turn()
  * wide_left_turn(int time)
  * halt()
  * motor_detach()
  * photo_calibrate()
  *

*/

//****************************************************************  Declarations    *********************************************************************//

#include <Servo.h>                        // Servo library

Servo Lservo;                             // defines left servo
Servo Rservo;                             // defines right servo

int Left_Sensor;                          // defines left sensor in sensor array as a boundary detector
int Right_Sensor;                         // defines left sensor in sensor array as a boundary detector
int L1;                                   // Leftmost sensor on analog pin 0 in sensor array
int L2;                                   // Leftcenter sensor on analog pin 1 in sensor array
int L3;                                   // Centor sensor on analog pin 2 in sensor array
int L4;                                   // Rightcenter sensor on analog pin 3 in sensor array
int L5;                                   // Rightmost sensor on analog pin 4 in sensor array
int Lwisk;                                // Left whisker switch
int Rwisk;                                // Right whisker switch
int Error;                                // Error value 
int Location;                             // Variable for determining position



//****************************************************************** Definitions ***********************************************************************//


# define Headlight1 13                       // LED on pin 13
# define Headlight2 2                        // LED on pin 2
# define LB  -9999                           // Left boundary value (-9999 for far left error resulting in max correction)
# define  LMB  -2                            // Left middle boundary value
# define  cLMB  -1                           // Center Left middle boundary value
# define  MB  0                              // Middle boundary value
# define  cRMB  1                            // Center Right middle boundary value
# define  RMB  2                             // Right Middle boundary value
# define  RB  9999                           // Right boundary value (-9999 for far left error resulting in max correction)

#define setpoint                              // Used for variable correction

//**********************************************************************  SETUP   *******************************************************************//

void setup() {

	pinMode(A0, INPUT);                         // Analog pin 0 declared as an input, this is the leftmost sensor in the sensor array
	pinMode(A1, INPUT);                         //  "
	pinMode(A2, INPUT);                         //  "
	pinMode(A3, INPUT);                         //  "
	pinMode(A4, INPUT);                         //  "
	Lwisk = digitalRead(7);                     // Left Whisker switch is on digital pin 7
	Rwisk = digitalRead(8);                     // Right Whisker switch is on digital pin 8
	pinMode(13, OUTPUT);                        // LED pin 13 declared an OUTPUT
	pinMode(2, OUTPUT);                          // LED pin 2 declared an OUTPUT
	Lservo.attach(10);                          // Left servo is attached to digital pin 10
	Rservo.attach(11);                          // Right servo is attached to digital pin 11
	Serial.begin(9600);                         // 8 bit serial communication with serial port monitor

}

//************************************************************   LOOP   *********************************************************************// 

void loop() {
	L1 = analogRead(0) + 41;                    // L1 sensor with numerical value added for sensor value balance
	L2 = analogRead(1);                       // "
	L3 = analogRead(2);                       // "
	L4 = analogRead(3);                       // "
	L5 = analogRead(4);                       // "
	byte Lwisk = digitalRead(7);              // converts Left whisker read to binary 
	byte Rwisk = digitalRead(8);              // converts Right whisker read to binary 

	headlights_on();

	if ((Lwisk == 1) && (Rwisk == 0))                  // If Left Whisker is pressed
	{
		backup(5000);
	}                                  // Movement command

	else if ((Lwisk == 0) && (Rwisk == 1))             // If Right Whisker is pressed
	{
		backup(5000);
	}                                  // Movement command

	else if ((Lwisk == 1) && (Rwisk == 1))             // If both Whiskers are pressed
	{
		backup(5000);
	}                                  // Movement command

	else                                               // If none of the above conditions are true
	{
		backup(10000);                                  // Action / movement command

		sensor_calc();                                   // Sensor calculation --- converts numeric values of sensor ranges into simplified numeric thresholds
														 // 0 is center threshold -1 is left center, 1 is right center, etc....(see hardcode in functions)

		line_follow();
	}                                  // line following using boundary thresholds   

} // End Bracket



//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    FUNCTIONS   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX//

//************************************************************  GENERAL   **************************************************************************//

void headlights_on() {
	digitalWrite(13, HIGH);
	digitalWrite(2, HIGH);
}

void left_whiskerPressed() {
	(Lwisk == HIGH) && (Rwisk == LOW);
}

void right_whiskerPressed() {
	(Lwisk == LOW) && (Rwisk == HIGH);
}

void both_whiskerPressed() {
	(Lwisk == HIGH) && (Rwisk == HIGH);
}

void no_whiskerPressed() {
	(Lwisk == LOW) && (Rwisk == LOW);
}

void line_follow() {
	if ((L3 < 1) && (L3 > -1)) {
		slow_forward();
		Serial.print("[ ^ Slow forward ^ ]");
	}

	else if (L3 == -1) {
		slow_right();
		Serial.print("[ > Slow Right > ]");
	}

	else if (L3 == -2) {
		hard_right();
		Serial.print("[ >> Hard Right > ]");
	}

	else if (L3 == -9999) {
		sharp_right();
		Serial.print("[ >>> Pivot Right >>> ]");
	}

	else if (L3 == 1) {
		slow_left();
		Serial.print("[ < Slow Left < ]");
	}

	else if (L3 == 2) {
		hard_left();
		Serial.print("[ << Hard Left << ]");
	}

	else if (L3 == 9999) {
		sharp_left();
		Serial.print("[ <<< Pivot Left <<< ]");
	}
}

//***********************************************************   Sensors   **********************************************************************//

void sensor_calc() {
	if ((L1 < 600) && (L5 < 600))                                      // Gray threshold
	{
		slow_forward();
		delay(2000);
		motor_detach();
	}
	else if ((L3 < 660) && (L1 > L5))
	{
		L3 = analogRead(0) * 0 * -1;
	}
	else if ((L3 < 660) && (L1 <= L5))
	{
		L3 = analogRead(0) * 0 * 1;
	}
	else if ((L3 < 750) && (L1 > L5))
	{
		L3 = analogRead(0) * 0 + 1 * -1;
	}
	else if ((L3 < 750) && (L1 <= L5))
	{
		L3 = analogRead(0) * 0 + 1 * 1;
	}
	else if ((L3 < 840) && (L1 > L5))
	{
		L3 = analogRead(0) * 0 + 2 * -1;
	}
	else if ((L3 < 840) && (L1 < L5))
	{
		L3 = analogRead(0) * 0 + 2 * 1;
	}
	else if ((L3 >= 840) && (L1 > L5))
	{
		L3 = analogRead(0) * 0 + -9999;
	}
	else if ((L3 >= 840) && (L1 < L5))
	{
		L3 = analogRead(0) * 0 + 9999;
	}

}




//************************************************************    Servo Movement   *****************************************************//

void slow_forward() {
	Lservo.writeMicroseconds(1550);
	Rservo.writeMicroseconds(1450);
	delay(50);
}
void slow_right() {
	Lservo.writeMicroseconds(1550);
	Rservo.writeMicroseconds(1500);
	delay(50);
}
void slow_left() {
	Lservo.writeMicroseconds(1500);
	Rservo.writeMicroseconds(1450);
	delay(80);
}
void hard_left() {
	Lservo.writeMicroseconds(1500);
	Rservo.writeMicroseconds(1300);
	delay(80);
}

void sharp_left() {
	Lservo.writeMicroseconds(1300);
	Rservo.writeMicroseconds(1300);
	delay(80);
}


void hard_right() {
	Lservo.writeMicroseconds(1700);
	Rservo.writeMicroseconds(1500);
	delay(80);
}

void sharp_right() {
	Lservo.writeMicroseconds(1700);
	Rservo.writeMicroseconds(1700);
	delay(80);
}


//************************************************************    Timed Manuevers  ***************************************************************//

void backup(int time) {
	Lservo.writeMicroseconds(1400);
	Rservo.writeMicroseconds(1600);
	delay(time);
}

void pivot_right(int time) {
	Lservo.writeMicroseconds(1700);
	Rservo.writeMicroseconds(1500);
	delay(time);
}

void pivot_left(int time) {
	Lservo.writeMicroseconds(1600);
	Rservo.writeMicroseconds(1600);
	delay(time);
}

void wide_right_turn() {
	Lservo.writeMicroseconds(1700);
	Rservo.writeMicroseconds(1400);

}

void wide_left_turn(int time) {
	Lservo.writeMicroseconds(1600);
	Rservo.writeMicroseconds(1300);
	delay(time);
}

void halt() {
	Lservo.writeMicroseconds(1500);
	Rservo.writeMicroseconds(1500);

}

void motor_detach() {
	Lservo.detach();
	Rservo.detach();
}

//************************************************************    Calibration   *******************************************************************//

void photo_calibrate() {                                     // Reads the raw sensor values in the array, use to tune values for threshold

	digitalWrite(2, HIGH);
	digitalWrite(13, HIGH);
	L1 = analogRead(0);
	L2 = analogRead(1);
	L3 = analogRead(2);
	L4 = analogRead(3);
	L5 = analogRead(4);
	Serial.print("P1 Sensor[");
	Serial.print(L1);
	Serial.print("]..P2 sensor[");
	Serial.print(L2);
	Serial.print("]..P3 sensor[");
	Serial.print(L3);
	Serial.print("]..P4 sensor[");
	Serial.print(L4);
	Serial.print("]..P5 sensor[");
	Serial.print(L5);
	Serial.println("]");
	delay(50);
}


