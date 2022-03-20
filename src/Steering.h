#include "Servo.h"

#ifndef uchar
typedef  unsigned char uchar;
#endif

class SteeringRig
{
public:
    SteeringRig()
    {
        ucHardTurn = 30;
        ucRegularTurn = 15;
        ucServoOffset = 0;
        ucPos = 0;
        lastUpdate = 0;
        refreshInterval = 0;
        intrinsicDelay = 15;
        bInverted = false;
    };

    bool Initiate(uchar inPin, uchar inOffset, bool inInverted = false)
    {
        mServo.attach(inPin);
        ucServoOffset = inOffset;
        bInverted = inInverted;

        return mServo.attached();
    }
    // Centers steering, returns current position
    uchar CenterSteering()
    {
        ucPos = 0;
        WriteServoPos(ucPos + ucServoOffset);
        delay(intrinsicDelay);
        return ucPos;
    }

    uchar SteeringLeft(bool inHard = false)
    {
        if (!inHard)
        {
            if (!bInverted)
            {
                ucPos = ucPos - 25;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;
            }
            else
            {
                ucPos = ucPos + 25;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;
            }
            
        }
        else
        {
            if (!bInverted)
            {
                ucPos = ucPos - 50;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;
            }
            else
            {
                ucPos = ucPos + 50;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;
            }
        }
    };

    uchar SteeringRight(bool inHard = false)
    {
        if (!inHard)
        {
            if (!bInverted)
            {
                ucPos = ucPos + 25;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;
            }
            else
            {
                ucPos = ucPos - 25;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;   
            }
           
        }
        else
        {
            if (!bInverted)
            {
                ucPos = ucPos + 50;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;
            }
            else
            {
                ucPos = ucPos - 50;
                WriteServoPos(ucPos + ucServoOffset);
                return ucPos;   
            }
           
        } 
    }

private:

    inline void WriteServoPos(uchar inPos){mServo.write(inPos);};

Servo mServo;
uchar ucServoOffset; // offset for 'center'
uchar ucPos; // current position of servo
uchar ucHardTurn;
uchar ucRegularTurn;

unsigned long lastUpdate;
unsigned long refreshInterval; // When should we update again
unsigned long intrinsicDelay;  // how long to delay execution while waiting on a command
bool bInverted;  // Is the servo inverted, may be caused by upside down servo in a steering mechanism
};