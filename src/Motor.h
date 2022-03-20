#ifndef _MOTOR_H__
#define _MOTOR_H__

#include "Arduino.h"


typedef unsigned char PinNum;
typedef unsigned char uchar;
enum Direction
{
	MOTOR_UNDEFINED,
	MOTOR_FORWARD,
	MOTOR_BACKWARD,
	MOTOR_HALTED
};
// This will control each motor, there may be multiple Motor Controllers
class MotorController
{
public:
	MotorController():bIsBraked(false),BrakePin(0),DirectionPin(0),PowerPin(0),SensingPin(0){};
	void setPins(PinNum inBrakePin, PinNum inDirectionPin, PinNum inPowerPin, PinNum inSensingPin)
	{
		BrakePin = inBrakePin;
		DirectionPin = inDirectionPin;
		PowerPin = inPowerPin;
		SensingPin = inSensingPin;
		_setPinInputOutput();
		setMotorForward(0);
	};
	bool getBrake(){ return bIsBraked; };
	void setBrake(bool inBraked)
	{
		bIsBraked = inBraked;
		digitalWrite(BrakePin, inBraked);
	};
	void BrakeOn(){ setBrake(HIGH); };
	void BrakeOFF(){ setBrake(LOW); };
	void setMotorForward(uchar inPower = 255)
	{
		if (!getIsInverted())  // Motor might be facing opposite direction!
		{
			digitalWrite(DirectionPin, HIGH);
		}
		else
		{
			digitalWrite(DirectionPin, LOW);
		}

		setPower(inPower);
		eMotorDirection = MOTOR_FORWARD;  // For reference tho, this will be considered "forward" regardless as to what pin is set to


	};
	void setMotorBackward(uchar inPower = 255)
	{
		if (!getIsInverted())  // Motor might be facing opposite direction!
		{
			digitalWrite(DirectionPin, LOW);
		}
		else
		{
			digitalWrite(DirectionPin, HIGH);
		}
		setPower(inPower);
		eMotorDirection = MOTOR_BACKWARD;
	};
	void setPower(uchar inPower)
	{
		curPower = inPower;
		analogWrite(PowerPin,inPower);
	};
	uchar getPower()
	{
		return curPower;
	};
	bool getIsInverted()
	{
		return bIsInverted;
	};
	void setIsInverted(bool inInverted)
	{
		bIsInverted = inInverted;
	};
private:
	void _setPinInputOutput()
	{
		pinMode(BrakePin, OUTPUT);
		pinMode(DirectionPin, OUTPUT);
		pinMode(PowerPin, OUTPUT);
		pinMode(SensingPin, INPUT);
	}
	bool bIsInverted; // If motor will be facing opposite direction, will this be inverted  (IE, left motor and right motor, one will have to be inverted)
	bool bIsBraked;
	uchar curPower;
	PinNum BrakePin, DirectionPin, PowerPin; // Digital
	PinNum SensingPin;  // Analog
	Direction eMotorDirection;
};

// This will control movement of the robot
class MovementManager
{
public:
	void SetMotors(MotorController inLeftMotor, MotorController inRightMotor)
	{
		LeftMotor = inLeftMotor;
		RightMotor = inRightMotor;
	};
	
	void MoveForward(uchar inSpeed=255)
	{
		_setMoveDirection(MOVING_FORWARD);
		LeftMotor.BrakeOFF();
		RightMotor.BrakeOFF();
		LeftMotor.setMotorForward(inSpeed);
		RightMotor.setMotorForward(inSpeed);

	};
	void MoveBackward(uchar inSpeed=255)
	{
		_setMoveDirection(MOVING_BACKWARDS);
		LeftMotor.BrakeOFF();
		RightMotor.BrakeOFF();
		LeftMotor.setMotorBackward(inSpeed);
		RightMotor.setMotorBackward(inSpeed);
	};
	void FullStop(bool UseBrake = false)
	{
		_setMoveDirection(IDLE);
		LeftMotor.setPower(0);
		RightMotor.setPower(0);
	};

	// Turn off power to the motors, as there's no reason to apply power and brake
	void BrakesOn()
	{
		_setMoveDirection(MOVING_BRAKED);
		LeftMotor.setPower(0);
		RightMotor.setPower(0);
		LeftMotor.BrakeOn();
		RightMotor.BrakeOn();
	};

	void BrakesOff()
	{
		LeftMotor.BrakeOFF();
		RightMotor.BrakeOFF();
	};

	// 0.0f = only right engine making the turn, left engine off
	// 0.5f means right engine full power, left engine half
	// 1.0f means yer gonna be going straight
	void LeftTurn(uchar inPower,float OpposingEnginePower = 0.5f)
	{

		RightMotor.setMotorForward(inPower);
		LeftMotor.setMotorForward(inPower * OpposingEnginePower);
	};
	// 1.0f = only left engine making the turn, 0.5f means left engine full power, right engine half
	void RightTurn(uchar inPower, float OpposingEnginePower = 0.5f)
	{
		
		RightMotor.setMotorForward(inPower * OpposingEnginePower);
		LeftMotor.setMotorForward(inPower);
	};

	void StaticTurnLeft(uchar inPower = 255)
	{
		_setMoveDirection(MOVING_LOCKED_LEFT);
		LeftMotor.setMotorBackward(inPower);
		RightMotor.setMotorForward(inPower);
	};
	void StaticTurnRight(uchar inPower = 255)
	{
		_setMoveDirection(MOVING_LOCKED_RIGHT);
		LeftMotor.setMotorForward(inPower);
		RightMotor.setMotorBackward(inPower);
	};

	void SetSpeed(uchar inPower)
	{
		LeftMotor.setPower(inPower);
		RightMotor.setPower(inPower);
	};


	enum CURMOVEDIRECTION
	{
		IDLE = 0,          // No movement
		MOVING_FORWARD,    // controller moving 'forward'
		MOVING_BACKWARDS,  // controller moving 'backward'
		MOVING_LEFT,       // controller moving 'left' with steering
		MOVING_RIGHT,      // Controller moving 'right' with steering
		MOVING_LOCKED_LEFT,// Controller is moving left with left wheel locked
		MOVING_LOCKED_RIGHT,// Controller is moving right with right wheel locked
		MOVING_BACK_LEFT,  // Controller is moving backwards to the left
		MOVING_BACK_RIGHT, // Controller is moving backwards to the right
		MOVING_BRAKED      // Brakes engaged
	};

	inline CURMOVEDIRECTION getMoveDirection() { return chasisDirection;};
private:
	CURMOVEDIRECTION chasisDirection;
	MotorController LeftMotor, RightMotor;

	void _setMoveDirection(CURMOVEDIRECTION inMoveDirection) { chasisDirection = inMoveDirection;};

};

#endif 