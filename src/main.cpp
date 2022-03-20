
#include <time.h>
#include <Servo.h>
#include "Motor.h"   // for controlling the base station
#include "Sensor.h"  // for controlling the Sensor Rig
#include "Steering.h"
#include "NavigationSystem.h"
//
#define SPEED 175

void setup()  // We'll control our own loops so we have deleted loop()
{
	Serial.begin(115200);   // provide feedback
  Serial.println("Smart car Initalizing...");

  SensorRig Sensor;
  Sensor.Initiate(5,55,45,2);

  Serial.println(". Sensor Rig Initialized");
  SteeringRig Steering;
  Steering.Initiate(6,130, true);
  Steering.CenterSteering();
  
  Serial.println(". Steering Rig Initialized");

	MotorController LeftMotor,RightMotor;
	RightMotor.setPins(9, 12, 3, 0); // Channel A
	LeftMotor.setPins(8, 13, 11, 1); // Channel B
	
	RightMotor.setIsInverted(false);
	LeftMotor.setIsInverted(true);
	
	MovementManager mMovementManager;
	mMovementManager.SetMotors(LeftMotor, RightMotor);
	
	Serial.println(". Motors Initialized");

	NavigationSystem mNavigationSystem(&Sensor, &mMovementManager);
	
	float pingedDistance = 0;

	Serial.println(". Entering Navigation Loop");
	
	while (true)
	{
		pingedDistance = 0;
		pingedDistance = Sensor.Ping();

		mNavigationSystem.Update(pingedDistance);
		


		delay(25);
	};
	
}

void loop()
{

}


