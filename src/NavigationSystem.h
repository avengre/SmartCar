#ifndef __NAVIGATIONSYSTEM_H
#define __NAVIGATIONSYSTEM_H

#include "Sensor.h"
#include "Motor.h"
#include <list>

struct Barrier
{
   float BarrierDistance;             // How far away is the barrier
   unsigned long tBarrierEncountered; // When did we encounter the barrier
   bool BarrierActive;

   // Barrier bounds left
   // Barrier bounds right
};

// Point for a noted barrier
struct BarrierPoint 
{
    float BarrierDistance;
    float Angle;
};


class NavigationSystem
{
public:
    NavigationSystem()
    {
        minClearanceDistance = 20.0f;
        pingedDistance = 0;

        // default the barrier
        mBarrier.BarrierActive = false;
        mBarrier.BarrierDistance = 0;
        mBarrier.tBarrierEncountered = 0;
        curNavmode = NAVMODE_IDLE;
        mSensorRig = NULL;
    };

    NavigationSystem(SensorRig *inSensorRig, MovementManager *inMotorController) : NavigationSystem()
    {
        mSensorRig = inSensorRig;
        mMovementManager = inMotorController;
    };

    inline unsigned char GetCurrentNavMode() { return curNavmode;};
    void StartMoving()
    {
        if (GetCurrentNavMode() == NAVMODE_IDLE)
        {
            SetCurrentNavMode(NAVMODE_MOVING);
        }
    }


    unsigned char Update(float inPingedDistance)  // returns true what the current plan for navigation is
    {
        pingedDistance = inPingedDistance;
   
        

        switch (GetCurrentNavMode())
        {
        case NAVMODE_IDLE: // Doing nothing
            _NavModeIDLE();
            break;
        case NAVMODE_EVALBARRIER: // Appears to be a barrier, evaluating
            _NavModeEVALBARRIER(inPingedDistance);
            break;
        case NAVMODE_BARRIERPLANNING: // Planning how to get out of this obstruction
            _NavModeBARRIERPLANNING();
            break;
        case NAVMODE_SOLUTIONFOUND:
            _NavModeSOLUTIONFOUND();
            break;
        case NAVMODE_MOVING:
            _NavModeMOVING(inPingedDistance);
            break;
            
        
        default:
            // Shouldn't ever happen...
            break;
        }

        return GetCurrentNavMode();
    };


private:

    enum NAVMODE : unsigned char
    {
        NAVMODE_IDLE = 0, // Doing nothing
        NAVMODE_EVALBARRIER, // Appears to be a barrier, evaluating
        NAVMODE_BARRIERPLANNING, // Planning how to get out of this obstruction
        NAVMODE_SOLUTIONFOUND,
        NAVMODE_MOVING,    // Moving
        NAVMODE_MANUAL     // Manual Control TODO
    };

    void _NavModeIDLE()
    {
        SetCurrentNavMode(NAVMODE_MOVING);
        mMovementManager->MoveForward(175);
        
    };
    void _NavModeEVALBARRIER(float inPingedDistance)
    {
        
        if (pingedDistance > 0)
            if (pingedDistance < minClearanceDistance)
                if (millis() >= (mBarrier.tBarrierEncountered + 150)) // see if the barrier is real / persistant
                {
                    // We got a problem
                    Serial.println ("Barrier persists... Entering planning phase");
                    SetCurrentNavMode(NAVMODE_BARRIERPLANNING);
                    mMovementManager->FullStop();
                }
                else
                {
                    // Lets be hopeful it was a sensor error
                }
                
            else
            {
                // Guess the object moved or was erroneous reading
                
                    switch (GetCurrentNavMode())
                    {
                        case NAVMODE_MOVING:                    // We were moving or idle, resume what we were doing (unlikely to get to this stage)
                        case NAVMODE_IDLE:
                            SetCurrentNavMode(GetPrevNavMode());
                        break;
                        
                        case NAVMODE_SOLUTIONFOUND:            // These only occur when we are moving previously
                        case NAVMODE_EVALBARRIER:
                        case NAVMODE_BARRIERPLANNING:
                            SetCurrentNavMode(NAVMODE_MOVING);
                        
                        default:
                            SetCurrentNavMode(NAVMODE_IDLE);
                        break;
                    };
                
                    ClearBarrierFlag();
                    SetCurrentNavMode(NAVMODE_MOVING);
            }
            
    };
    void _NavModeBARRIERPLANNING()
    {
       
         // Something is infront of us, lets find a way past it
        if (mSensorRig->getServoPosition() != 90)
            mSensorRig->Center(); // Centor


        std::list<BarrierPoint> pointList;

        float dist = 0;

        // Scan in front of us
        bool bAreaScanned = false;
        unsigned short scanIncrement = 15;  // degrees
        unsigned short curIncrement = 0;
        
        bool bScanCompleted = false;

        float sweepMin = 1000;
        float sweepMax = 0;

        while (!bScanCompleted)
        {
           mSensorRig->UpdateSweep(millis());    
           dist = mSensorRig->Ping();
           BarrierPoint point;
           point.BarrierDistance = dist;
           point.Angle = mSensorRig->getServoPosition();

           Serial.print("Barrier Point: "); Serial.print( point.BarrierDistance ); Serial.print( " @ "); Serial.println( point.Angle);

            if (mSensorRig->getServoPosition() > sweepMax)
                sweepMax = mSensorRig->getServoPosition();

            if (mSensorRig->getServoPosition() < sweepMin)
                sweepMin = mSensorRig->getServoPosition();

            Serial.print("Min: "); Serial.print(sweepMin); Serial.print(" Max: "); Serial.println(sweepMax);
        }

    };
    void _NavModeSOLUTIONFOUND()
    {

    };

    // To be implemented, manual control with a remote?
    void _NavModeMANUAL()
    {

    };

    void _NavModeMOVING(float inPingedDistance)
    {
        // Center up our sensor just to be sure
        mSensorRig->Center();

         if (pingedDistance > 0) // Distance is > max or sensor is obstructed
        {
            if (pingedDistance < minClearanceDistance)    // Found a barrier
            {
                    mBarrier.BarrierDistance = pingedDistance;
                    mBarrier.tBarrierEncountered = millis();
                    mBarrier.BarrierActive = true;
                    SetCurrentNavMode(NAVMODE_EVALBARRIER);
                    Serial.print ("Glimpsed a barrier at: "); Serial.print(pingedDistance); Serial.println(" cm");
                              
            }
            else // No current barrier
            {
                if (mBarrier.BarrierActive)
                {
                    Serial.println("Barrier Cleared");
                    SetCurrentNavMode(NAVMODE_IDLE);
                }
                mBarrier.BarrierActive = false;             // Not currently a problem
                mBarrier.BarrierDistance = pingedDistance;  // But there's something ahead
            }
            
        }
    };


    void SetCurrentNavMode( unsigned char inNaveMode) { prevNavMode = curNavmode; curNavmode = inNaveMode;};

    void ClearBarrierFlag()
    {
        mBarrier.BarrierActive = false;
        mBarrier.BarrierDistance = 0;
    };
    
    unsigned char GetPrevNavMode() { return prevNavMode;};
    unsigned char curNavmode;
    unsigned char prevNavMode;
    float minClearanceDistance;  // Minimum distance we should stop if something is infront of us
    float pingedDistance;
    SensorRig *mSensorRig; // Sensor rig
    MovementManager *mMovementManager;
    Barrier mBarrier;
};


#endif