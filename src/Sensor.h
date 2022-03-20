#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <time.h>
#include <NewPing.h>

#define _SENSOR_DEBUG 0

#define US_SHORT_ERROR 65535
class SensorRig
{
public:
	SensorRig()
	{
		bServoControlPin = 0;
		bServoActive = false;
		usAbsolutePosition = 0;
		bLimitedServoRange = true;  // assume its a normal servo
		usSweepRange = 0;
		LookatDirection = 0;
		minSweep = 0;
		maxSweep = 0;
		eSWEEPDIR = SERVO_STOP;
		fSweepTime = 0;
		lastSweepDegree = 0;

	};
	void Initiate(const unsigned char inControlPin,  unsigned short inFrontTrim, unsigned short inSweepRange, float inSweepTime, bool inSelfTest = false)
	{
		bServoControlPin = inControlPin;
		sRotationTrimFront = inFrontTrim;
		bServoActive = true;
		bLimitedServoRange = true;
		mServo.attach(inControlPin);
		usSweepRange = inSweepRange;
		fSweepTime = inSweepTime;
		if (bLimitedServoRange)
		{
			minSweep = 0;
			maxSweep = 180;
		}

		if (inSelfTest)
		{
		mServo.write(0);
		delay(500);
		mServo.write(160);
		delay(500);
		mServo.write(inFrontTrim);
		delay(250);
		}
		usAbsolutePosition = inFrontTrim;
	};

	unsigned short UpdateSweep(unsigned long inTime)
	{
		
		// See if we are stopped, if so, choose start direction
		if (eSWEEPDIR == SERVO_STOP)
		{
			if (usAbsolutePosition >= sRotationTrimFront) // to the right of center
			{
				eSWEEPDIR = SERVO_LEFT; // move left
			}
			else
			{
				eSWEEPDIR = SERVO_RIGHT;
			}
		}

		if (ulLastUpdate == 0)
		{
			// skip this cycle, probably first run, and set our time
			ulLastUpdate = inTime;
			lastSweepDegree = 0;
			return US_SHORT_ERROR; // error max value
		}

		unsigned long delta = inTime - ulLastUpdate;

		// Catch if its been unresonably long since last update
		
		if (delta > 2000)
		{
			
			ulLastUpdate = inTime;
			return US_SHORT_ERROR;
		}

		// Calculate new position in relative position
		unsigned short currentPos = usAbsolutePosition ;
		unsigned short targetPos = 0;
		float deltaSeconds = 0;
		float degreesPerSecond = 0;
		float deltaMove = 0;

		deltaSeconds = ((float)delta) / 1000;
		
		degreesPerSecond =  ((float)usSweepRange) / fSweepTime;
		deltaMove = degreesPerSecond * deltaSeconds;   // calculate how much to move

		lastSweepDegree += deltaMove;                       // combined movement this cycle
	
		if (lastSweepDegree >= 1)  // we are moving at least 1 degree
		{
			

			if (eSWEEPDIR == SERVO_RIGHT)   // we are currently (or were) moving right
			{
				
				targetPos = currentPos + round(lastSweepDegree);
				
				
				if (targetPos >= (sRotationTrimFront + usSweepRange))
				{
					// no movement, reverse direction
					eSWEEPDIR = SERVO_LEFT;
					
				}
				else
				{
					_MoveServo(targetPos);
					delay(15);
					lastSweepDegree = 0;
				}
			}
			else if (eSWEEPDIR == SERVO_LEFT)
			{
				targetPos = currentPos - round(lastSweepDegree);
				


				if (targetPos <= (sRotationTrimFront - usSweepRange))
				{
					eSWEEPDIR = SERVO_RIGHT;
					
				}
				else
				{
					_MoveServo(targetPos);
					delay(15);
					lastSweepDegree = 0;
				}
			}
		}
		else
		{
			// no movement this cycle as its less than 1 degree, which our servo can't do
		}
		
			
	
		ulLastUpdate = inTime;
	};
	// Look at a direction, returns if this is possible
	bool LookAt(unsigned short inDirection)
	{
		unsigned char moveDirection = SERVO_STOP;

		// ensure its within our bounds
		if (!_RangeCheck(inDirection))
		{
			Serial.println("Error: Requested sensor lookat is out of bounds");
			return false;
		}
			

		if (inDirection > usAbsolutePosition + sRotationTrimFront)  // Need to look right
		{
			moveDirection = SERVO_RIGHT;
			_MoveServo(inDirection);
		}
		else if (inDirection < usAbsolutePosition+ sRotationTrimFront)
		{
			moveDirection = SERVO_LEFT;
			_MoveServo(inDirection);
		}
		else
		{
			moveDirection = SERVO_STOP;
		}

	};
	void PanRight(const short inMovement) { _MoveServo(usAbsolutePosition+inMovement); };
	void PanLeft(const short inMovement) { _MoveServo(usAbsolutePosition-inMovement); };
	void Center() { LookAt(sRotationTrimFront); };

	bool isActive() { return bServoActive; };
	void setActive(bool inState) { bServoActive = inState; };
	bool toggleActive() { bServoActive = !bServoActive; return bServoActive; };
	unsigned short getTrim() { return sRotationTrimFront; };

	// Do we want to incorporate the delay to the ping in the command?
	float Ping(bool bWithDelay = false) 
	{	

		float dist = mSonar.Ping();

		fPrevPingDistance = dist;   // TODO: Add sanity checking
		return dist;
	}

	inline float getServoPosition() { return usAbsolutePosition+ sRotationTrimFront;};
	inline unsigned short getSweepRange() { return usSweepRange;};

	unsigned short getSweepMin() { return }
	unsigned short getSweepMax()
private:

	class PingSensor
	{
	public:
		PingSensor(unsigned char inTriggerPin = 4, unsigned char inEchoPin = 2)
		{
			triggerPin = inTriggerPin;
			echoPin = inEchoPin;
			maxDistance = 400;
			mSonar = new NewPing(triggerPin, echoPin, maxDistance);
			DeviationError = 0;
			usPingDelay =35 ; // how long to wait for return
		};

		float Ping() {

			float distance = 0;
			float duration = 0;

			duration = mSonar->ping();
			distance = (duration / 2)*0.0343;
			
			return distance;
		};
		void setPingLength(unsigned short inPingLength) { usPingLength = inPingLength; };
	private:
		NewPing *mSonar;
		unsigned short maxDistance;
		unsigned char triggerPin, echoPin;
		unsigned short usPingLength, usPingDelay;

		float DeviationError;  // how much variation should we ignore
		
		float duration;
		float distance;
	};


	PingSensor mSonar;

	void _MoveServo(const short inPos)    // This is private because should never allow direct movement of servorig
	{
		if (bLimitedServoRange)   // If we have a traditional Servo, shouldn't receive negative numbers
		{
			if (inPos < 0)
			{
				usAbsolutePosition = 0;
				return;
			}
			
			if (inPos > 180)
			{
				usAbsolutePosition = 180;
				return;
			}
				

			if (!isActive())
				return;

			
			

			if (mServo.attached())
				mServo.write(inPos);

			usAbsolutePosition = inPos;
		}
		else   // Continious Servo not implemented yet
		{

		}
	};

	// move relative to our current position
	void _relativeMoveServo(const short inDelta)
	{
		_MoveServo(usAbsolutePosition + sRotationTrimFront+ inDelta);
	};

	bool _RangeCheck(short inPotentialDirection)
	{
		// Ensure our planned direction won't move outside bounds
		return ((inPotentialDirection <= maxSweep) && (inPotentialDirection >= minSweep) ? true : false);
	};

	enum : short
	{
		SERVO_STOP = 0,        // Servo should be stopping
		SERVO_RIGHT = 1,       // Moving Right
		SERVO_LEFT = -1        // Moving LEFT
	};

	unsigned long ulLastUpdate;
	float fSweepTime;          // how many seconds for a half sweep
	float lastSweepDegree;     // make this a float incase its a partial so the rounder doesn't zero it out
	 short eSWEEPDIR;
	Servo mServo;
	unsigned short sRotationTrimFront;          // Offset to have servo point towards front of vehicle
	unsigned char bServoControlPin;
	bool bServoActive;                      // TRUE = Do things, FALSE = Disabled
	unsigned short usAbsolutePosition;     // This is the absolute direction of the sensor motor
	bool bLimitedServoRange;                // TRUE if a typical servo, FALSE if a continious servo
	unsigned short usSweepRange;            // Range on either side of center to sweep during normal operation
	short LookatDirection;         // Direction we are trying to look at
	short minSweep, maxSweep;      // Minimum and maximum movement threshold

	float fPrevPingDistance;                // For sanity checking
	
};

#endif