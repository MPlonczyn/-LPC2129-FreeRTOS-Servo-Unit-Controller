#include <LPC21xx.H>
#include "FreeRTOS.h"
#include "queue.h"
#include "led.h"
#include "servo.h"

#define DETECTOR_bm (1<<10)

typedef struct {
    enum {CMD_GOTO, CMD_SPEED, CMD_WAIT, CMD_CALLIB} eType;
    unsigned int uiValue;
} ServoCommand_t;

QueueHandle_t xServoQueue;
ServoCommand_t xCommand;

void DetectorInit(){
    IO0DIR &= ~DETECTOR_bm;
}

enum DetectorState{ ACTIVE, INACTIVE };

enum DetectorState eReadDetector(){
    if((IO0PIN & DETECTOR_bm) != 0) return INACTIVE;
    return ACTIVE;
}

struct Servo sServo;

void Automat() {
    ServoCommand_t xCommand;
    
    switch(sServo.eState) {
        
        case IDLE:
            if(xQueueReceive(xServoQueue, &xCommand, portMAX_DELAY) == pdTRUE) {
                switch(xCommand.eType) {
                    case(CMD_GOTO):{
                        sServo.uiDesiredPosition = xCommand.uiValue;
                        sServo.eState = IN_PROGRESS;
                        break;
                    }
                    case(CMD_CALLIB):{
                        sServo.eState = CALLIB;
                        break;
                    }
                    case(CMD_SPEED):{
                        sServo.uiSpeed = xCommand.uiValue;
                        break;
                    }
                    case(CMD_WAIT):{
                        sServo.uiWaitTicks = xCommand.uiValue;
                        vTaskDelay(sServo.uiWaitTicks);
                        break;
                    }
                }
            }
            break;
                
        case CALLIB:
            if(eReadDetector() == INACTIVE){
                LedStepRight();
            }
            else{
                sServo.eState = IDLE;
                sServo.uiCurrentPosition = 0;
                sServo.uiDesiredPosition = 0;
            }

            vTaskDelay(sServo.uiSpeed);
            break;
                
        case IN_PROGRESS:
            if(sServo.uiCurrentPosition > sServo.uiDesiredPosition){
                LedStepRight();
                sServo.uiCurrentPosition--;
            }
            else if(sServo.uiCurrentPosition < sServo.uiDesiredPosition){
                LedStepLeft();
                sServo.uiCurrentPosition++;
            }
            else{
                sServo.eState = IDLE;
            }
						
            vTaskDelay(sServo.uiSpeed);
            break;		
    }
}

void ServoWait(unsigned int uiTicks) {
    xCommand.eType = CMD_WAIT;
    xCommand.uiValue = uiTicks;
    xQueueSend(xServoQueue, &xCommand, portMAX_DELAY);
}

void ServoSpeed(unsigned int uiDelay) {
    xCommand.eType = CMD_SPEED;
    xCommand.uiValue = uiDelay;
    xQueueSend(xServoQueue, &xCommand, portMAX_DELAY);
}

void ServoInit(unsigned int uiServoFrequency){
    sServo.eState = CALLIB;
    sServo.uiSpeed = 10;
    sServo.uiCurrentPosition = 0;
    sServo.uiDesiredPosition = 0;
    
    DetectorInit();
    LedInit();
    
    xServoQueue = xQueueCreate(20, sizeof(ServoCommand_t));
}

void ServoCallib(){
    xCommand.eType = CMD_CALLIB;
    xCommand.uiValue = 0;
    xQueueSend(xServoQueue, &xCommand, portMAX_DELAY);
}

void ServoGoTo(unsigned int uiPosition){
    xCommand.eType = CMD_GOTO;
    xCommand.uiValue = uiPosition;
    xQueueSend(xServoQueue, &xCommand, portMAX_DELAY);
}