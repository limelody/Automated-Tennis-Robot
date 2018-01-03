/*
MTE 101/GENE 121 Final Project
Author(s): Zhao Feng Dai, Cameron Haas, Melody Li, Tom Paraschuk
Program description: Main Program 
Date: 2017-11-19
*/

#include "EV3Servo-lib-UW.c"

const int BASE_SERVO_TIME = 100;//ARBITRARY

const bool GAMEPLAY = true;
const bool PRACTICE = false;

const bool HARD = true;
const bool EASY = false;

const bool RIGHTSIDE = true; //corresponds to backhand stroke to practice
const bool LEFTSIDE = false; //corresponds to forehand stroke to practice

const int TOWARD_PLAYER_HARD = 20;
const int TOWARD_PLAYER_EASY = 80;

const int TABLE_EDGE_MOTORCOUNTS = 800;

const int HARD_POWER = 100;
const int EASY_POWER = 70;
const int PRACTICE_POWER = 85;

const int STANDARD_ANGLE = -47; //determined experimentally to be the 
							    //lowest possible angle of the lifter system
const int ANGLE_RANGE = 45; //experimentally determined to be the 
							//maximum possible angle of the lifter system

const int HARD_SPIN_MODIFIER = 10;
const int EASY_SPIN_MODIFIER = 5;
const int PRACTICE_SPIN_MODIFIER = 8;

const int SPINNER_MOTOR = motorC;
const int TOP_LAUNCHER_MOTOR = motorA;
const int BOTTOM_LAUNCHER_MOTOR = motorB;

const int ULTRASONIC_PLAYER_DETECTION_SENSOR = S4; //ARBITRARY
const int COLOR_SHOT_SUCCESS_SENSOR = S3; //ARBITRARY
const int GYRO_RETURN_DETECTION_SENSOR = S2; //ARBITRARY
const int TETRIX_MOTOR_GATEWAY = S1; //ARBITRARY

const int TETRIX_LIFTER_PORT = 1;
const int TETRIX_FEEDER_PORT = 2;
const int FEEDER_POWER = 10;

const int TRESHOLD_REFLECTION_VALUE = 6;
const int BALL_RETURN_THRESHOLD = 4;


/*************************shot struct************************/


typedef struct {
	int nLauncher_Motor_Top_Power,nLauncher_Motor_Bottom_Power, 
		nMotorcounts_Right_From_Centerline, nMotorcounts_Down_From_Horizontal;
} Shot;


/*************************ball launch functions************************/


void height_change(int current_angle)
{
	setServoSpeed(TETRIX_MOTOR_GATEWAY, TETRIX_LIFTER_PORT, current_angle);
	wait1Msec(BASE_SERVO_TIME);
}

void rotate_Launcher(int encoder_target, bool & exit)
{
	if (!exit)
	{
		if (encoder_target < 0)
		{
			nMotorEncoder[SPINNER_MOTOR] = 0;
			motor[SPINNER_MOTOR] = -95;
			while (nMotorEncoder[SPINNER_MOTOR] > encoder_target && !exit)
			{
				if (getButtonPress(buttonAny))
					exit = true;
			}

		}
		else
		{
			nMotorEncoder[SPINNER_MOTOR] = 0;
			motor[SPINNER_MOTOR] = 95;
			while (nMotorEncoder[SPINNER_MOTOR] < encoder_target && !exit)
			{
				if (getButtonPress(buttonAny))
					exit = true;
			}
		}
		motor[SPINNER_MOTOR] = 0;
	}
}

void launcher_rotation_calibration()
{
	bool valid_input = false;
	bool stop_turn = false;
	do
		{
			eraseDisplay();
			displayString(0, "Calibrate launcher rotational position?");
			displayString(1, "Press up button for yes");
			displayString(2, "Press down button for no.");
			while(!getButtonPress(buttonAny))
			{}
			if (getButtonPress(buttonUp))
			{
				while (getButtonPress(buttonUp))
				{}
				valid_input = true;
				displayString(0, "Press any button to set 0");
				while (!stop_turn)
				{
					rotate_Launcher(-TABLE_EDGE_MOTORCOUNTS, stop_turn);
					rotate_Launcher(2*TABLE_EDGE_MOTORCOUNTS, stop_turn);
				}
				nMotorEncoder[SPINNER_MOTOR] = 0;

			}
			else if (getButtonPress(buttonDown))
			{
				while(getButtonPress(buttonAny))
				{}
				valid_input = true;
			}
			else
			{
				while(getButtonPress(buttonAny))
				{}
				valid_input = false;
			}
		} while (valid_input == false);
		eraseDisplay();
}

void run_Launcher_Wheels(int top_wheel, int bottom_wheel)
{
		motor[TOP_LAUNCHER_MOTOR] = -top_wheel;
		motor[BOTTOM_LAUNCHER_MOTOR] = -bottom_wheel;
}

int launch(Shot theShot, bool bMode, bool & exit)
{
	height_change(theShot.nMotorcounts_Down_From_Horizontal);
	if (bMode == GAMEPLAY)
		rotate_Launcher(theShot.nMotorcounts_Right_From_Centerline, exit);
	run_Launcher_Wheels(theShot.nLauncher_Motor_Top_Power, 
						theShot.nLauncher_Motor_Bottom_Power);

	return theShot.nMotorcounts_Right_From_Centerline;
}


/*************************shot choice functions************************/


void setShot_Toward_Side(int nSide, Shot & theShot)
{
	if (nSide == RIGHTSIDE)
		theShot.nMotorcounts_Right_From_Centerline = TABLE_EDGE_MOTORCOUNTS;
	//else if it is to launch to the left side of the table
	else
		theShot.nMotorcounts_Right_From_Centerline = -TABLE_EDGE_MOTORCOUNTS;
}

void generate_Shot (Shot & theShot, bool nMode, bool nDifficulty_Or_Stroke, 
					bool nPlayer_Side, int nConsecutive_Returns)
{
	if (nMode == GAMEPLAY)
	{
		int nToward_Player = rand() % 50;
		int nShot_Angle = rand () % ANGLE_RANGE + STANDARD_ANGLE;

		if (nDifficulty_Or_Stroke == HARD)
		{

			theShot.nLauncher_Motor_Top_Power = HARD_POWER;
			theShot.nLauncher_Motor_Top_Power = HARD_POWER-HARD_SPIN_MODIFIER; 
			theShot.nMotorcounts_Down_From_Horizontal = nShot_Angle;

			if (nToward_Player < TOWARD_PLAYER_HARD + nConsecutive_Returns) 
			//TOWARD_PLAYER_HARD is less than TOWARD_PLAYER_EASY - 
			//which means nToward_Player is less likely
			//to be less than TOWARD_PLAYER_HARD - nConsecutive_Returns, 
			//which means the machine is less likely to hit towards the player
			//Makes assumption that because more likely to not hit towards 
			//player's side, whatever performance the player was able to achieve
			//was against a shot that was launched away from the previously - 
			//therefore only nToward_Player is compared in some manner against
			//nConsecutive_Returns
			setShot_Toward_Side(nPlayer_Side, theShot);

			else
				setShot_Toward_Side(!nPlayer_Side, theShot);
		}

		else
		{

			theShot.nLauncher_Motor_Bottom_Power = EASY_POWER;
			theShot.nLauncher_Motor_Bottom_Power = 
												EASY_POWER-EASY_SPIN_MODIFIER;

			if (nToward_Player < TOWARD_PLAYER_EASY - nConsecutive_Returns)
				setShot_Toward_Side(nPlayer_Side, theShot);

			else
				setShot_Toward_Side(!nPlayer_Side, theShot);
		}
	}

	else
	{
		theShot.nLauncher_Motor_Top_Power = PRACTICE_POWER;
		theShot.nLauncher_Motor_Bottom_Power = 
										PRACTICE_POWER-PRACTICE_SPIN_MODIFIER;
		theShot.nMotorcounts_Down_From_Horizontal = STANDARD_ANGLE;

		if (nDifficulty_Or_Stroke == RIGHTSIDE)
			setShot_Toward_Side(RIGHTSIDE, theShot);

		else
			setShot_Toward_Side(LEFTSIDE, theShot);
	}
}


/*************************feeder system functions************************/


void feeder_pwr(int sensor_Port, int tetrix_motor_port, int feeder_motor_power)
{
	setServoSpeed(sensor_Port, tetrix_motor_port, feeder_motor_power); 
}

bool check_last_launch(int color_port, bool & exit)
{
	clearTimer(T2);

	while (time1[T2] < 6000 && !exit)
	{
		if (SensorValue[color_port] >= TRESHOLD_REFLECTION_VALUE)
		{
			displayString (6, "successful launch %d", SensorValue[color_port]);
			return true;
		}
		if (getButtonPress(buttonAny))
			exit = true;
	}
	displayString(6, "failed launch");
	return false;
}


/*************************ball returned functions************************/


int ball_returned(float shot_frequency, int gyro_port, bool & exit)
{
	SensorValue[gyro_port] = 0;
	time1[T1] = 0;
	while (time1[T1] < 1/shot_frequency && !exit)
	{
		if (abs(SensorValue[gyro_port]) >= BALL_RETURN_THRESHOLD)
			return 1;
		if (getButtonPress(buttonAny))
			exit = true;
	}
	return 0;
}


/**********************player detection system functions*******************/


bool get_player_Side(int ultrasonic_port)
{
	int counter = 0;
	for (int measure = 0; measure < 5; measure++)
	{
		int current_distance = SensorValue[ultrasonic_port];
		if (current_distance != 255)
			counter++;
	}
	if (counter > 2)
	{
		displayString (0, "RIGHT SIDE"); 
		return RIGHTSIDE;

	displayString (0, "LEFT SIDE");
	return LEFTSIDE;
}


/*************************player input funtion************************/


void get_Player_Input(bool bMode, bool bDifficulty_or_stroke, bool & exit)
{ 	//add exit case

	bool bValid_Input = false;
	do
	{
		eraseDisplay();
		displayString(0,"Welcome to Pong 2.0.");
		displayString(1,"Please select a game mode.");
		displayString(3,"Press top button for GAMEPLAY.");
		displayString(4,"Press bottom button for PRACTICE.");
		displayString(5,"Press left button to quit.");

		while (!getButtonPress(buttonAny))
		{}

		if (getButtonPress(buttonUp))
		{
			bValid_Input = true;
			bMode = GAMEPLAY;

			while (getButtonPress(buttonUp))
			{}
		}

		else if (getButtonPress(buttonDown))
		{
			bValid_Input = true;
			bMode = PRACTICE;
			while (getButtonPress(buttonDown))
			{}
		}

		else if (getButtonPress(buttonLeft))
		{
			bValid_Input = true;
			exit = true;
			while (getButtonPress(buttonLeft))
			{}
		}

		else
		{
			while (getButtonPress(buttonAny))
			{}
			eraseDisplay();
			displayString(0,"Please enter a valid input");
			wait1Msec(3000);
			eraseDisplay();
		}

	} while(bValid_Input == false);


	eraseDisplay();

	if (bMode == GAMEPLAY && !exit)
	{
		bValid_Input = false;
		do
		{
			eraseDisplay();
			displayString(0,"Please enter a difficulty.");
			displayString(1,"Press top button for HARD");
			displayString(2,"Press bottom button for EASY");
			displayString(3,"Press left button to QUIT.");

			while (!getButtonPress(buttonAny))
			{}

			if (getButtonPress(buttonUp))
			{
				bValid_Input = true;
				bDifficulty_or_stroke = HARD;
				while (getButtonPress(buttonUp))
				{}
			}

			else if (getButtonPress(buttonDown))
			{
				bValid_Input = true;
				bDifficulty_or_stroke = EASY;

				while (getButtonPress(buttonDown))
				{}
			}

			else if (getButtonPress(buttonLeft))
			{
				bValid_Input = true;
				exit = true;
				while(getButtonPress(buttonLeft))
				{}
			}

			else
			{
				while (getButtonPress(buttonAny))
				{}
				eraseDisplay();
				displayString(0,"Please enter a valid input.");
				wait1Msec(3000);
				eraseDisplay();
			}

		} while(bValid_Input == false);
	}

	else if (!exit)
	{
		bValid_Input = false;
		do
		{
			eraseDisplay();
			displayString(2, "Please enter a stroke to practice.");
			displayString(3, "Press top button for right side.");
			displayString(4, "Press bottom button for left side.");
			displayString(5, "Press left button to quit.");

			while (!getButtonPress(buttonAny))
			{}

			if (getButtonPress(buttonUp))
			{
				bValid_Input = true;
				bDifficulty_or_stroke = LEFTSIDE;
				while (getButtonPress(buttonUp))
				{}
			}

			else if (getButtonPress(buttonDown))
			{
				bValid_Input = true;
				bDifficulty_or_stroke = RIGHTSIDE;
				while (getButtonPress(buttonDown))
				{}
			}
			else if (getButtonPress(buttonLeft))
			{
				bValid_Input = true;
				exit = true;
				while(getButtonPress(buttonLeft))
				{}
			}
			else
			{
				while (getButtonPress(buttonAny))
				{}
				eraseDisplay();
				displayString(0,"Please enter a valid input.");
				wait1Msec(3000);
				eraseDisplay();
			}

		} while(bValid_Input == false);
	}
}


/*************************play one feeder function************************/


int play_1_feeder (bool bMode, bool bDifficulty_or_stroke, bool & exit, 
				   int & total_number_shots)
{ //add shots output
	Shot current_shot;
	bool bLast_Launch_success = true;
	int nShots_returned_in_session = 0;
	bool bLast_shot_returned = false;
	int nConsecutive_shot_returns = 0;
	bool bPlayer_Side = LEFTSIDE;
	int nLauncher_Rotation = 0;

	feeder_pwr(TETRIX_MOTOR_GATEWAY, TETRIX_FEEDER_PORT, FEEDER_POWER);

	if (bMode == GAMEPLAY)
	{
		while (bLast_Launch_success && !exit)
		{

			bPlayer_Side = get_player_Side(ULTRASONIC_PLAYER_DETECTION_SENSOR);
			generate_Shot(current_shot, bMode, bDifficulty_or_stroke, 
						  bPlayer_Side, nConsecutive_shot_returns);
			nLauncher_Rotation = launch(current_shot, bMode, exit);
			if (getButtonPress(buttonAny))
				exit = true;
			bLast_Launch_success = check_last_launch(COLOR_SHOT_SUCCESS_SENSOR, 
													 exit);
			total_number_shots += bLast_Launch_success - 1;
			if (ball_returned(0.25,GYRO_RETURN_DETECTION_SENSOR, exit))
			{
				nShots_returned_in_session++;

				if (bLast_shot_returned)
					nConsecutive_shot_returns++;

				bLast_shot_returned = true;
			}

			else
				bLast_shot_returned = false;
			rotate_Launcher(-nLauncher_Rotation, exit); 
			//can move to in front of ball returned detection if neccessary
			//height_change(-(theshot.nMotorcounts_Down_From_Horizontal));
		}
		feeder_pwr(TETRIX_MOTOR_GATEWAY, TETRIX_FEEDER_PORT, 0);
	}

	else if (!exit)
	{
		//practice mode
		rotate_Launcher(nLauncher_Rotation, exit);
		while (bLast_Launch_success && !exit) {

			generate_Shot (current_shot, bMode, bDifficulty_or_stroke, 
						   bPlayer_Side, nConsecutive_shot_returns);
			launch(current_shot, bMode, exit); //what
			if (getButtonPress(buttonAny))
				exit = true;
			bLast_Launch_success = check_last_launch(COLOR_SHOT_SUCCESS_SENSOR,
													 exit);
			//rotate_Launcher(-nLauncher_Rotation, exit); 
			//can move to after ball returned detection if neccessary

			if (ball_returned(0.25,GYRO_RETURN_DETECTION_SENSOR, exit))
			{
				nShots_returned_in_session++;
				displayString(5,"%d",nShots_returned_in_session);
				if (bLast_shot_returned)
					nConsecutive_shot_returns++;
				bLast_shot_returned = true;
			}

			else
				bLast_shot_returned = false;

		}
		feeder_pwr(TETRIX_MOTOR_GATEWAY, TETRIX_FEEDER_PORT, 0);
		rotate_Launcher(-nLauncher_Rotation, exit);
	}
	return nShots_returned_in_session;
}


/**********************************Main***********************************/


task main()
{
  bool bExit_program = false;
  SensorType[TETRIX_MOTOR_GATEWAY]=sensorI2CCustom9V;
	SensorType[ULTRASONIC_PLAYER_DETECTION_SENSOR] = sensorEV3_Ultrasonic;
	SensorType[COLOR_SHOT_SUCCESS_SENSOR] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[COLOR_SHOT_SUCCESS_SENSOR] = modeEV3Color_Reflected;
	wait1Msec(50);
	SensorType[GYRO_RETURN_DETECTION_SENSOR] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorMode[GYRO_RETURN_DETECTION_SENSOR] = modeEV3Gyro_Rate;
	wait1Msec(50);
	SensorType[TETRIX_MOTOR_GATEWAY]=sensorI2CCustom9V;
	feeder_pwr(TETRIX_MOTOR_GATEWAY, TETRIX_FEEDER_PORT, 0);

	while (!bExit_program)
	{
		bool bMode = true;
		bool bDifficulty_or_stroke = true;

		get_Player_Input(bMode,bDifficulty_or_stroke, bExit_program);

		int number_shots_returned_this_session = 0;

		int total_number_shots = 0;
	 	float percent_returned = 0;

		bool bExit_play = false;

		//playing sequence
		while (!bExit_play && !bExit_program)
		{
			launcher_rotation_calibration();

			eraseDisplay();
			displayString(0,"Press any button to start");

			while(!getButtonPress(buttonAny))
			{}

			while(getButtonPress(buttonAny))
			{}

			eraseDisplay();

			nMotorEncoder[SPINNER_MOTOR] = 0;

			wait1Msec(10000); 
			// waits for the player to go to the other side of the table

			feeder_pwr(TETRIX_MOTOR_GATEWAY, TETRIX_FEEDER_PORT, FEEDER_POWER);

			number_shots_returned_this_session += 
			play_1_feeder(bMode,bDifficulty_or_stroke, bExit_play, 
						  total_number_shots);


	 		percent_returned = 
			number_shots_returned_this_session/total_number_shots;

			if (!bExit_play)
			{
				motor[motorA] = motor[motorB] = motor[motorC] = 0;
				eraseDisplay();
				displayString(0,"Magazine empty.");
				displayString(1,"Press the top button to refill and continue");
				displayString(2,"Press the bottom button to exit to home menu");

				while(!getButtonPress(buttonAny))
				{}

				if (getButtonPress(buttonUp))
				{
					while(getButtonPress(buttonUp))
					{}

					eraseDisplay();
					displayString(0,
						"Press middle button to ackowledge refill complete");

					while(!getButtonPress(buttonEnter))
					{}

					while(getButtonPress(buttonEnter))
					{}
				}

				else if (getButtonPress(buttonDown))
				{
					while (getButtonPress(buttonDown))
					{}
					bExit_play = true;
				}
			}
		}
		eraseDisplay();
		motor[motorA] = motor[motorB] = motor[motorC] = 0;
		displayString(0,"You returned a total of %d shots this session", 
					  number_shots_returned_this_session);
		displayString(1,"Your accuracy is &f", percent_returned);
		displayString(2, "Press any button to continue.");
		while(!getButtonPress(buttonAny))
		{}
		while(getButtonPress(buttonAny))
		{}
	}
}
