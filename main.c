#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "led.h"
#include "string.h"
#include "servo.h"
#include "keyboard.h"

xQueueHandle xPrintQueue;
extern xQueueHandle xServoQueue;

void Keyboard (void *pvParameters){
	while(1){
		switch(eKeyboardRead()){
			case BUTTON_0: ServoCallib();
			break;
			case BUTTON_1: ServoGoTo(12);
			break;
			case BUTTON_2: ServoGoTo(24);
			break;
			case BUTTON_3:
							ServoSpeed(8);
							ServoGoTo(12);
							ServoWait(10);
			
							ServoSpeed(4);
							ServoGoTo(24);
							ServoWait(10);
			
							ServoSpeed(20);
							ServoGoTo(36);
							ServoWait(10);
							
							ServoSpeed(1);
							ServoGoTo(0);
				break;
			default: break;
		}
	vTaskDelay(100);
	}
}

void ServoTask (void *pvParameters) {
	while(1) {
		Automat();
		vTaskDelay(5);
	}
}

// czas w tickach 0x0279 = 633

int main( void ){	
	//xPrintQueue = xQueueCreate(5, sizeof(char) * 64);
	
	ServoInit(100);
	KeyboardInit();
	
	xTaskCreate(Keyboard, NULL, 128, NULL, 2, NULL );
	xTaskCreate(ServoTask, NULL, 128, NULL, 2, NULL ); 	
	//xTaskCreate(LettersTx, NULL, 128, NULL, 1, NULL );
	//xTaskCreate(KeyboardTx, NULL, 128, NULL, 1, NULL );
		
	vTaskStartScheduler();
	
	while(1);
}