/**
 * This is the main file of the ESPLaboratory Demo project.
 * It implements simple sample functions for the usage of UART,
 * writing to the display and processing user inputs.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */
#include "includes.h"
#include <math.h>
#include <string.h>
#include "timers.h"

#define mPI acos(-1.0)
#define debounceTick 50
#define defaultButton 1
#define EXERCISE_2 0
#define EXERCISE_3_1 1
#define EXERCISE_3_2 2
#define EXERCISE_4 3

// start and stop bytes for the UART protocol
static const uint8_t startByte = 0xAA, stopByte = 0x55;

static const uint16_t displaySizeX = 320, displaySizeY = 240;

//Dynamic Stack
#define dynamicStackSize 1000

//Static Stack
#define staticStackSize 1000
/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStack[staticStackSize];

//State Machine
unsigned int state = 0;

//relative Position of Joystick
unsigned char tmpA = 1, tmpA3 = 0;
unsigned char buttonStateA = 0;
unsigned char tmpB = 1, tmpB3 = 0;
unsigned char buttonStateB = 0;
unsigned char tmpC = 1;
unsigned char buttonStateC = 0;
unsigned char tmpD = 1;
unsigned char buttonStateD = 0;
unsigned char tmpK = 1;
unsigned char buttonStateE = 0;
unsigned char tmpE = 1;
unsigned char buttonStateK = 0;
unsigned long long lastPressedTick = 0;

unsigned int tmpTask4 = 0;

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

// Stores lines to be drawn
QueueHandle_t JoystickQueue;

//Task parameter
unsigned char redCircleDefault = 1;
unsigned char blueCircleDefault = 1;

unsigned char taskCState = 0;
unsigned char taskCParameter = 0;

static TaskHandle_t xTaskDrawTask = NULL;
static TaskHandle_t xTaskA = NULL;
static TaskHandle_t xTaskB = NULL;
static TaskHandle_t xTaskC = NULL;
static TaskHandle_t xTaskStatic = NULL;
static TaskHandle_t xTaskDynamic = NULL;
static TaskHandle_t xTaskDrawCircle = NULL;
static TaskHandle_t xTask4_1 = NULL;
static TaskHandle_t xTask4_2 = NULL;
static TaskHandle_t xTask4_3 = NULL;
static TaskHandle_t xTask4_4 = NULL;
static TaskHandle_t xTaskDisplayScreen = NULL;
static TaskHandle_t xTaskStateMachine = NULL;


static TaskHandle_t xTaskCheckJoystick = NULL;
static TaskHandle_t xTaskChangeState = NULL;
static TaskHandle_t xTaskBigLoop = NULL;


//static SemaphoreHandle_t xTaskB = NULL;
SemaphoreHandle_t xSemaphore;
SemaphoreHandle_t xSemaphore4;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//Timer
//const char pcTimerName = "myTimer" ;
//const TickType_txTimerPeriod = 1000;
//const UBaseType_tuxAutoReload = pdTrue;
//void * const pvTimerID = ;

//TimerHandle_t xTimerCreate( &pcTimerName,
//							TickType_txTimerPeriod,
//							UBaseType_tuxAutoReload,
//							(void * 0,)
//		TimerCallbackFunction_t pxCallbackFunction );

#define NUM_TIMERS 5

 /* An array to hold handles to the created timers. */
 TimerHandle_t xTimers[ NUM_TIMERS ];

 /* Define a callback function that will be used by multiple timer
 instances.  The callback function does nothing but count the number
 of times the associated timer expires, and stop the timer once the
 timer has expired 10 times.  The count is saved as the ID of the
 timer. */
// void vTimerCallback( TimerHandle_t xTimer )
// {
// const uint32_t ulMaxExpiryCountBeforeStopping = 10;
// uint32_t ulCount;
//
//    /* Optionally do something if the pxTimer parameter is NULL. */
//    configASSERT( xTimer );
//
//    /* The number of times this timer has expired is saved as the
//    timer's ID.  Obtain the count. */
//    ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );
//
//    /* Increment the count, then test to see if the timer has expired
//    ulMaxExpiryCountBeforeStopping yet. */
//    ulCount++;
//
//    /* If the timer has expired 10 times then stop it from running. */
//    if( ulCount >= ulMaxExpiryCountBeforeStopping )
//    {
//        /* Do not use a block time if calling a timer API function
//        from a timer callback function, as doing so could cause a
//        deadlock! */
//        //xTimerStop( pxTimer, 0 ); //IS THIS WRONG ?
//    	xTimerStop( xTimer, 0 );
//    }
//    else
//    {
//       /* Store the incremented count back into the timer's ID field
//       so it can be read back again the next time this software timer
//       expires. */
//       vTimerSetTimerID( xTimer, ( void * ) ulCount );
//    }
// }

 void vTimer1Callback( TimerHandle_t xTimer )
 {
	 unsigned char str[100];
	 font_t font1; // Load font for ugfx
	 font1 = gdispOpenFont("DejaVuSans24*");

	 sprintf(str, "RESET RESET RESET RESET RESET RESET");
	 gdispDrawString(0, 60, str, font1, Black);

	 tmpA3 = 0;
	 tmpB3 = 0;

	 xTimerStop(xTimer, 1000);
 }

 void vTimer2Callback( TimerHandle_t xTimer )
 {
	 if(taskCState)
	 {
		 taskCParameter++;
		 vTaskResume( xTaskC );
	 }
 }

 void vTimer3Callback( TimerHandle_t xTimer )
 {
	vTaskSuspend(xTaskDisplayScreen);
	vTaskSuspend(xTask4_1);
	vTaskSuspend(xTask4_2);
	vTaskSuspend(xTask4_3);
	vTaskSuspend(xTask4_4);
}


int main() {
	xTimers[0] = xTimerCreate
						( /* Just a text name, not used by the RTOS
	                     kernel. */
	                     "Timer1",
	                     /* The timer period in ticks, must be
	                     greater than 0. */
	                     5000,
	                     /* The timers will auto-reload themselves
	                     when they expire. */
	                     pdTRUE,
	                     /* The ID is used to store a count of the
	                     number of times the timer has expired, which
	                     is initialised to 0. */
	                     ( void * ) 0,
	                     /* Each timer calls the same callback when
	                     it expires. */
						 vTimer1Callback);


	xTimers[1] = xTimerCreate
							( /* Just a text name, not used by the RTOS
		                     kernel. */
		                     "Timer2",
		                     /* The timer period in ticks, must be
		                     greater than 0. */
		                     1000,
		                     /* The timers will auto-reload themselves
		                     when they expire. */
		                     pdTRUE,
		                     /* The ID is used to store a count of the
		                     number of times the timer has expired, which
		                     is initialised to 0. */
		                     ( void * ) 0,
		                     /* Each timer calls the same callback when
		                     it expires. */
							 vTimer2Callback);


	xTimers[2] = xTimerCreate
								( /* Just a text name, not used by the RTOS
			                     kernel. */
			                     "Timer3",
			                     /* The timer period in ticks, must be
			                     greater than 0. */
			                     15,
			                     /* The timers will auto-reload themselves
			                     when they expire. */
			                     pdFALSE,
			                     /* The ID is used to store a count of the
			                     number of times the timer has expired, which
			                     is initialised to 0. */
			                     ( void * ) 0,
			                     /* Each timer calls the same callback when
			                     it expires. */
								 vTimer3Callback);


	// Initialize Board functions and graphics
	ESPL_SystemInit();
	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));

	// Initializes Tasks with their respective priority

	//EXERCISE 2
	xTaskCreate(drawTask, "drawTask", 1000, NULL, 7, &xTaskDrawTask);
	xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 3, &xTaskCheckJoystick);
	xTaskCreate(changeState, "changeState", 1000, NULL, 5, &xTaskChangeState);

	//EXERCISE 3
	xTaskCreate(dynamicTask, "dynamicTask", dynamicStackSize, &blueCircleDefault, 5, &xTaskDynamic);
	xTaskStatic = xTaskCreateStatic(staticTask, "staticTask", staticStackSize, &redCircleDefault, 5, xStack, &xTaskBuffer); // xTaskStatic is Task_Handler
	xTaskCreate(draw2Circle, "draw2Circle", 1000, NULL, 6, &xTaskDrawCircle);

	xTaskCreate(taskTriggerA, "taskTriggerA", 1000, NULL, 9, &xTaskA);
	xTaskCreate(taskTriggerB, "taskTriggerB", 1000, NULL, 9, &xTaskB);
	xTaskCreate(taskTriggerC, "taskTriggerC", 1000, &taskCParameter, 9, &xTaskC);

	//EXERCISE 4
	xTaskCreate(task4_1, "task4_1", 1000, NULL, 4, &xTask4_1);
	xTaskCreate(task4_2, "task4_2", 1000, NULL, 3, &xTask4_2);
	xTaskCreate(task4_3, "task4_3", 1000, NULL, 2, &xTask4_3);
	xTaskCreate(task4_4, "task4_4", 1000, NULL, 1, &xTask4_4);


	xTaskCreate(stateMachine, "stateMachine", 1000, &state,8, &xTaskStateMachine);
	xTaskCreate(displayScreen, "displayScreen", 1000, NULL, 4, &xTaskDisplayScreen);



	vTaskSuspend(xTaskDrawTask);
	vTaskSuspend(xTaskDynamic);
	vTaskSuspend(xTaskStatic);
	vTaskSuspend(xTaskDrawCircle);
	vTaskSuspend(xTaskA);
	vTaskSuspend(xTaskB);
	vTaskSuspend(xTaskC);
	vTaskSuspend(xTask4_1);
	vTaskSuspend(xTask4_2);
	vTaskSuspend(xTask4_3);
	vTaskSuspend(xTask4_4);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}




void stateMachine(unsigned int *state)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	unsigned char flag = 0;

	while(1)
	{
		unsigned int stateNOW = *state;
		switch(stateNOW){
			case EXERCISE_2:
			{
				vTaskResume(xTaskDrawTask);
				vTaskResume(xTaskDisplayScreen);
				vTaskResume(xTaskCheckJoystick);
				vTaskResume(xTaskChangeState);

				vTaskSuspend(xTaskDynamic);
				vTaskSuspend(xTaskStatic);
				vTaskSuspend(xTaskDrawCircle);
				vTaskSuspend(xTaskA);
				vTaskSuspend(xTaskB);
				vTaskSuspend(xTaskC);



				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
				break;
			}

			case EXERCISE_3_1:
			{
				vTaskSuspend(xTaskDrawTask);

				vTaskResume(xTaskDynamic);
				vTaskResume(xTaskStatic);
				vTaskResume(xTaskDrawCircle);
				vTaskResume(xTaskDisplayScreen);
				vTaskResume(xTaskCheckJoystick);
				vTaskResume(xTaskChangeState);
				//vTaskResume(xTaskA);
				//vTaskResume(xTaskB);
				//vTaskResume(xTaskC);

				unsigned char str[100];
				sprintf(str, "THIS IS EXERCISE 3_1!");
				gdispDrawString(0, 20, str, font1, Black);

				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
				break;
			}
//
			case EXERCISE_3_2:
			{

				vTaskResume(xTaskDisplayScreen);
				vTaskResume(xTaskCheckJoystick);
				vTaskResume(xTaskChangeState);

				vTaskSuspend(xTaskDrawTask);
				vTaskSuspend(xTaskDynamic);
				vTaskSuspend(xTaskStatic);
				vTaskSuspend(xTaskDrawCircle);

				unsigned char str[100];
				sprintf(str, "THIS IS EXERCISE 3_2. PLEASE PRESS A AND B !");
				gdispDrawString(0, 20, str, font1, Black);

				vTaskResume(xTaskB);

					if (checkButtonK())
					{
						tmpA3 = 0;
						tmpB3 = 0;
					}

					if(checkButtonA())
					{
						tmpA3++;
						vTaskResume(xTaskA); // Won t work without this
						xTaskNotifyGive(xTaskA);
					}

					if(checkButtonB())
					{
						tmpB3++;
						xSemaphoreGive(xSemaphore);
					}

					if(checkButtonC())
					{
						flag = !flag; // Change State so Task C wont be suspended anymore
					}
					if (flag)
					{
						vTaskResume(xTaskC);
					}


				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
				break;
			}
			default:
			{
				vTaskSuspend(xTaskDrawTask);
				vTaskSuspend(xTaskDynamic);
				vTaskSuspend(xTaskStatic);
				vTaskSuspend(xTaskDrawCircle);
				vTaskSuspend(xTaskA);
				vTaskSuspend(xTaskB);
				vTaskSuspend(xTaskC);

				vTaskResume(xTask4_1);
				vTaskResume(xTask4_2);
				vTaskResume(xTask4_3);
				vTaskResume(xTask4_4);

				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
				break;
			}
		}

	}
}

void task4_1()
{
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	const TickType_t tickFramerate = 1;
	unsigned char str[100];
	sprintf(str, "1");

	while(1)
	{
		gdispDrawString(tmpTask4, tmpTask4, str, font1, Black);
		tmpTask4 = tmpTask4 + 10;
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

void task4_2()
{
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 2;

	unsigned char str[100];
	sprintf(str, "2");


	while(1)
		{
			gdispDrawString(tmpTask4, tmpTask4, str, font1, Black);
			tmpTask4 = tmpTask4 + 10;

			xSemaphoreGive(xSemaphore4 );
			vTaskDelayUntil(&xLastWakeTime, tickFramerate);
		}
}

void task4_3()
{
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	xSemaphore4 = xSemaphoreCreateBinary();

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 3;

	unsigned char str[100];
	sprintf(str, "3");

	while(1)
		{
		if( xSemaphore4 != NULL )
				{
					/* See if we can obtain the semaphore.  If the semaphore is not
					available wait 10 ticks to see if it becomes free. */
					if( xSemaphoreTake( xSemaphore4,  portMAX_DELAY ) == pdTRUE )
					{
						/* We were able to obtain the semaphore and can now access the
						shared resource. */
						gdispDrawString(tmpTask4, tmpTask4, str, font1, Black);
						tmpTask4 = tmpTask4 + 10;

						/* We have finished accessing the shared resource.  Release the
						semaphore. */
						//xSemaphoreGive(xSemaphore);
					}
		//			else
		//			{
		//				gdispClear(white)
		//			}
				}
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
		}
}
void task4_4()
{

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 4;

	unsigned char str[100];
	sprintf(str, "4");

	while(1)
		{
			gdispDrawString(tmpTask4, tmpTask4, str, font1, Black);
			tmpTask4 = tmpTask4 + 10;
			vTaskDelayUntil(&xLastWakeTime, tickFramerate);
		}
}

// EXERCISE 4 PRIORITY
// SHOW SCREEN (15) 4(4)-3(3)-2(2)-1(1): 4 2 3 1 1 2 1 4 3 1 2 1 1 3 2 1 4 1 2 1 3 2 1 1
// SHOW SCREEN (15) 4(3)-3(3)-2(2)-1(1): 4 2 3 1 1 2 1 3 1 4 2 1 1 3 2 1 1 4 2 1 3 1 2 1
// SHOW SCREEN (15) 4(2)-3(2)-2(1)-1(1): 4 1 2 3 1 2 1 3 1 4 2 1 1 3 2 1 1 4 2 1 3 1 2 1
// SHOW SCREEN (15) 4(1)-3(3)-2(2)-1(4): 1 1 2 3 4 1 1 2 1 3 1 2 4 1 1 3 2 1 1 2 4 1 3 1
// SEMAPHORE CONFLICT

void staticTask(unsigned char * p1)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 1000;

	uint16_t circleRadius = 50;
	uint16_t circleLineWidth = 3;

	while(1)
	{
		*p1 = !(*p1); // Change State of the Circle
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}

}

void dynamicTask(unsigned char * p2)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 500;

	uint16_t circleRadius = 50;
	uint16_t circleLineWidth = 3;

	while(1)
	{
		*p2 = !(*p2); // Change State of the Circle
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

void draw2Circle()
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;
	uint16_t circleRadius = 50;

	while(1)
	{
		if(redCircleDefault == 1)
		{
			gdispFillCircle(displaySizeX/2 - 80  ,displaySizeY/2 , circleRadius, Red);
		}
		if(blueCircleDefault == 1)
		{
			gdispFillCircle(displaySizeX/2 + 80  ,displaySizeY/2 , circleRadius, Blue);
		}

		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

void taskTriggerA ()
{
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	char str[100]; // buffer for messages to draw to display

	while(1)
	{
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		sprintf(str, "TASK NOTIFICATION #: %d", tmpA3);
		gdispDrawString(0, 60, str, font1, Black);
		//xTimerStart(xTimers[0],0);
	}
}

void taskTriggerB ()
{
	xSemaphore = xSemaphoreCreateBinary();
	//gdispClear(White);
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	char str[100]; // buffer for messages to draw to display

	while(1)
	{
	  if( xSemaphore != NULL )
		{
			/* See if we can obtain the semaphore.  If the semaphore is not
			available wait 10 ticks to see if it becomes free. */
			if( xSemaphoreTake( xSemaphore,  portMAX_DELAY ) == pdTRUE )
			{
				/* We were able to obtain the semaphore and can now access the
				shared resource. */

				sprintf(str, "BINARY SEMAPHORE #: %d", tmpB3);
				gdispDrawString(0, 100, str, font1, Black);
				//xTimerStart(xTimers[0],0);

				/* We have finished accessing the shared resource.  Release the
				semaphore. */
				//xSemaphoreGive(xSemaphore);
			}
//			else
//			{
//				gdispClear(white)
//			}
		}
	}
}

//void taskTriggerC(unsigned char *p3)
//{
//	unsigned char str[100];
//	font_t font1; // Load font for ugfx
//	font1 = gdispOpenFont("DejaVuSans24*");
//
//	while(1)
//	{
////		if(!taskCState)
////		{
////			vTaskSuspend( xTaskC );
////		}
//		sprintf(str, "TASK C COUNTING: %d", *p3);
//		gdispDrawString(0, 140, str, font1, Black);
//	}
//}

void taskTriggerC(unsigned char *p3)
{

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 1000;

	unsigned char str[100];
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	while(1)
	{
		vTaskSuspend(xTaskC);
		taskCParameter++;
		sprintf(str, "TASK C COUNTING: %d", taskCParameter);
		gdispDrawString(0, 140, str, font1, Black);

		vTaskDelayUntil(&xLastWakeTime, tickFramerate);

	}
}

void displayScreen()
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	gdispClear(White);
	ESPL_DrawLayer();

	while(1)
	{
		//xTimerStart(xTimers[2],0);
		ESPL_DrawLayer();
		gdispClear(White);
		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers

		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

void changeState() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	while (1) {
		//volatile unsigned char but_val = !GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E);
		if (checkButtonE()) {
			//while(!GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E))
			//;
			unsigned char prev_state = state;
			state = prev_state + 1;
//			state++;

			if (state > 3) {
				state = 0;
			}
		}
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

/**
 * Example task which draws to the display.
 */
void drawTask() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	char str[100]; // buffer for messages to draw to display
	char str2[100];
	struct coord joystickPosition; // joystick queue input buffer

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	//Calculating Joysticks Position
	int relativeJoystickPositionX = 0; //related with screen
	int relativeJoystickPositionY = 0;

	//Counting Buttons
	uint16_t cntA = 0;
	uint16_t cntB = 0;
	uint16_t cntC = 0;
	uint16_t cntD = 0;
	uint16_t cntE = 0;
	uint16_t cntK = 0;

	//Initial Points and Positions
	double distanceOfObjects = 50; // rotating Radius around the triangle (middle point of screen)
	double anglePositionCircle = mPI;
	uint16_t circleRadius = 10;
	uint16_t circleLineWidth = 3;
	uint16_t circlePositionX = displaySizeX / 2 - distanceOfObjects; // Left X Axis
	uint16_t circlePositionY = (float) 1 / 2 * displaySizeY; // 1/3 Top Y Axis

	uint16_t trianglePointA_X = displaySizeX / 2 - 15;
	uint16_t trianglePointA_Y = (float) 1 / 2 * displaySizeY + 15;
	uint16_t trianglePointB_X = displaySizeX / 2 + 15;
	uint16_t trianglePointB_Y = (float) 1 / 2 * displaySizeY + 15;
	uint16_t trianglePointC_X = displaySizeX / 2;
	uint16_t trianglePointC_Y = (float) 1 / 2 * displaySizeY - 15;
	const point trianglePoint[3] = {{trianglePointA_X, trianglePointA_Y},
							  {trianglePointB_X, trianglePointB_Y},
							  {trianglePointC_X, trianglePointC_Y}};

	double anglePositionSquare = 0;
	uint16_t squareLength = 20;
	uint16_t squarePositionX = displaySizeX / 2 + distanceOfObjects - squareLength; // Right X Axis, 3rt term is linear translation to calibrate middle point of the square
	uint16_t squarePositionY = (float) 1 / 2 * displaySizeY - squareLength;

	char staticString[50] = { "HELLO ESPL" }; // Amount of character ?
	uint16_t staticStringPositionX = displaySizeX / 2 - 30;
	uint16_t staticStringPositionY = displaySizeY - 45;

	char dynamicString[50] = { "THIS IS EXERCISE 2" };
	uint16_t dynamicStringPositionX = 0;
	uint16_t dynamicStringPositionY = displaySizeY - 30;
	uint16_t dynamicStringTmp = 0;

	while (1) {

//		switch(state)
//		{
//			case EXERCISE_2:
			// *********************************************** FIRST STATE *******************************************************
			{

				//		// wait for buffer swap
				while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE)
					;

				//EXERCISE 2
				//2.1 EXERCISE 3 FIGURES ----------------------------------------------------------------------------------------------------------------
				//CALCULATING NEW ANGLE POSITION FOR ROTATING FIGURES
				double rotatingSpeed = 360;
				if (anglePositionCircle <= -2 * mPI) {
					anglePositionCircle = anglePositionCircle + 2 * mPI - mPI / rotatingSpeed;
				} else {
					anglePositionCircle = anglePositionCircle + mPI / rotatingSpeed;
				}

				if (anglePositionSquare <= -2 * mPI) {
					anglePositionSquare = anglePositionSquare + 2 * mPI - mPI / rotatingSpeed;
				} else {
					anglePositionSquare = anglePositionSquare + mPI / rotatingSpeed;
				}

				//CALCULATING NEW POSITION FOR DYNAMIC STRING
				dynamicStringPositionX = (dynamicStringPositionX + 1) % displaySizeX;
				//	if(abs((double)displaySizeX-(double)dynamicStringPositionX) < 10)
				//	{
				//		dynamicStringTmp = displaySizeX - dynamicStringPositionX; // Calculate the start index which will not be shown on display
				//	}
				//	else
				//	{
				//		dynamicStringTmp = 0;
				//	}
				//	char *stringTmp = dynamicString; CHECK LATER
				//	stringTmp += dynamicStringTmp;
				//

				//CALCULATING SCREEN'S MOVEMENT RELATED TO JOYSTICKPOSITION
				if (((abs(joystickPosition.x - 127) / 2) >= 10)
						| ((abs(joystickPosition.y - 127) / 2) >= 10)) // Avoid noise
						{
					relativeJoystickPositionX = (joystickPosition.x - 127) / 1.5; // relative position for programmer, 2 is the moving speed
					relativeJoystickPositionY = (joystickPosition.y - 127) / 1;
				} else {
					relativeJoystickPositionX = 0;
					relativeJoystickPositionY = 0;
				}

				//SHOW A TRIANGLE
//				gdispDrawLine(trianglePointA_X + relativeJoystickPositionX,
//						trianglePointA_Y + relativeJoystickPositionY,
//						trianglePointB_X + relativeJoystickPositionX,
//						trianglePointB_Y + relativeJoystickPositionY, Blue);
//				gdispDrawLine(trianglePointB_X + relativeJoystickPositionX,
//						trianglePointB_Y + relativeJoystickPositionY,
//						trianglePointC_X + relativeJoystickPositionX,
//						trianglePointC_Y + relativeJoystickPositionY, Blue);
//				gdispDrawLine(trianglePointC_X + relativeJoystickPositionX,
//						trianglePointC_Y + relativeJoystickPositionY,
//						trianglePointA_X + relativeJoystickPositionX,
//						trianglePointA_Y + relativeJoystickPositionY, Blue);
				double dynamicPositionTriangle_X = 0 + relativeJoystickPositionX;
				double dynamicPositionTriangle_Y = 0 + relativeJoystickPositionY;

				gdispDrawPoly(dynamicPositionTriangle_X ,dynamicPositionTriangle_Y , trianglePoint, 3, Blue);

				//SHOW A ROTATING CIRCLE
				double dynamicPositionCircleX = displaySizeX / 2 + distanceOfObjects * cos(anglePositionCircle); // relative to centre point of the screen X Axis
				double dynamicPositionCircleY = displaySizeY / 2 + distanceOfObjects * sin(anglePositionCircle); // relative to centre point of the scree Y Axis
				gdispFillCircle(dynamicPositionCircleX + relativeJoystickPositionX,dynamicPositionCircleY + relativeJoystickPositionY, circleRadius, Red);
				gdispFillCircle(dynamicPositionCircleX + relativeJoystickPositionX,dynamicPositionCircleY + relativeJoystickPositionY, circleRadius - circleLineWidth, White);

				//SHOW A ROTATING SQUARE
				double dynamicPositionSquareX = displaySizeX / 2 - squareLength / 2 + distanceOfObjects * cos(anglePositionSquare); // relative to centre point of the screen X Axis
				double dynamicPositionSquareY = displaySizeY / 2 - squareLength / 2 + distanceOfObjects * sin(anglePositionSquare); // relative to centre point of the scree Y Axis
				gdispFillArea(dynamicPositionSquareX + relativeJoystickPositionX, dynamicPositionSquareY + relativeJoystickPositionY, squareLength, squareLength, Green);

				//SHOW TEXT STATIC
				gdispDrawString(staticStringPositionX + relativeJoystickPositionX, staticStringPositionY + relativeJoystickPositionY, staticString, font1, Black);

				//SHOW TEXT DYNAMIC
				gdispDrawString(dynamicStringPositionX + relativeJoystickPositionX, dynamicStringPositionY + relativeJoystickPositionY, dynamicString, font1, Black);
				//	if(dynamicStringTmp > 0)
				//	{
				//		gdispDrawString(0, dynamicStringPositionY, *stringTmp, font1, Black); // NOT SURE
				//		gdispDrawString(staticStringPositionX, staticStringPositionY -10 , staticString, font1, Black);
				//		//while(1);
				//	}

				//2.2 BUTTON EXERCISE ---------------------------------------------------------------------------------------------------------------------
				if (checkButtonA()) {
					cntA++;
				}
				if (checkButtonB()) {
					cntB++;
				}
				if (checkButtonC()) {
					cntC++;
				}
				if (checkButtonD()) {
					cntD++;
				}
				if (checkButtonE()) {
					cntE++;
				}
				if (checkButtonK()) {
					cntK++;
					cntA = 0;
					cntB = 0;
					cntC = 0;
					cntD = 0;
				}

				// Generate string with current joystick values
				sprintf(str, "A: %d, #: %d |B: %d, #: %d |C %d, #: %d |D: %d, #: %d",
						GPIO_ReadInputDataBit(ESPL_Register_Button_A,
								ESPL_Pin_Button_A), cntA,
						GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B), cntB,
						GPIO_ReadInputDataBit(ESPL_Register_Button_C,
								ESPL_Pin_Button_C), cntC,
						GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D), cntD);
				sprintf(str2, "E: %d, #: %d|K: %d, #: %d|X: %d, Y: %d",
						GPIO_ReadInputDataBit(ESPL_Register_Button_E,
								ESPL_Pin_Button_E), cntE,
						GPIO_ReadInputDataBit(ESPL_Register_Button_K,
								ESPL_Pin_Button_K), cntK, joystickPosition.x,
						joystickPosition.y);
				// Print string of joystick values
				gdispDrawString(0 + relativeJoystickPositionX,
						0 + relativeJoystickPositionY, str, font1, Black);
				gdispDrawString(0 + relativeJoystickPositionX,
						15 + relativeJoystickPositionY, str2, font1, Black);

				//2.3 JOYSTICK -----------------------------------------------------------------------------------------------------------------------
				// Generate string with current joystick values
				sprintf(str, "Axis 1: %5d|Axis 2: %5d|VBat: %5d",
						ADC_GetConversionValue(ESPL_ADC_Joystick_1),
						ADC_GetConversionValue(ESPL_ADC_Joystick_2),
						ADC_GetConversionValue(ESPL_ADC_VBat));

				// Print string of joystick values
				gdispDrawString(0 + relativeJoystickPositionX, 30 + relativeJoystickPositionY, str, font1, Black);

//				// Wait for display to stop writing
//				xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
//				// swap buffers
//				ESPL_DrawLayer();



				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
//				break;
//			}
//
//			// ******************************************** END FIRST STATE *******************************************************
//
//		case EXERCISE_3 :
//			// ********************************************* SECOND STATE *********************************************************
//		{
//			sprintf(str, "THIS IS EXERCISE 3. PLEASE PRESS A AND B !");
//			gdispDrawString(0, 20, str, font1, Black);
//
//			if (checkButtonK())
//			{
//				tmpA3 = 0;
//				tmpB3 = 0;
//			}
//
//			if(checkButtonA())
//			{
//				tmpA3++;
//				//xTaskNotifyGive(xTaskA);
//			}
//
//			if(checkButtonB())
//			{
//				tmpB3++;
//				//xSemaphoreGiveFromISR( xSemaphore, NULL );
//				//xSemaphoreGive(xSemaphore);
//				//portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//			}
//
//			if(checkButtonC())
//			{
//				taskCState = !taskCState;
//				if(taskCState == 1)
//				{
//					xTimerStart(xTimers[1],0);
//
//				}
//				else
//				{
//					xTimerStop(xTimers[1],0);
//				}
//			}
//
//
////				if (checkButtonA()) {
////								cntA++;
////							}
////							if (checkButtonB()) {
////								cntB++;
////							}
////							if (checkButtonC()) {
////								cntC++;
////							}
////							if (checkButtonD()) {
////								cntD++;
////							}
////							if (checkButtonE()) {
////								cntE++;
////							}
////							if (checkButtonK()) {
////								cntK++;
////								cntA = 0;
////								cntB = 0;
////								cntC = 0;
////								cntD = 0;
////						}
////								sprintf(str, "A: %d, #: %d |B: %d, #: %d |C %d, #: %d |D: %d, #: %d",
////										GPIO_ReadInputDataBit(ESPL_Register_Button_A,
////												ESPL_Pin_Button_A), cntA,
////										GPIO_ReadInputDataBit(ESPL_Register_Button_B,
////												ESPL_Pin_Button_B), cntB,
////										GPIO_ReadInputDataBit(ESPL_Register_Button_C,
////												ESPL_Pin_Button_C), cntC,
////										GPIO_ReadInputDataBit(ESPL_Register_Button_D,
////												ESPL_Pin_Button_D), cntD);
////								sprintf(str2, "E: %d, #: %d|K: %d, #: %d|X: %d, Y: %d",
////										GPIO_ReadInputDataBit(ESPL_Register_Button_E,
////												ESPL_Pin_Button_E), cntE,
////										GPIO_ReadInputDataBit(ESPL_Register_Button_K,
////												ESPL_Pin_Button_K), cntK, joystickPosition.x,
////										joystickPosition.y);
////								// Print string of joystick values
////								gdispDrawString(0 + relativeJoystickPositionX,
////										0 + relativeJoystickPositionY, str, font1, Black);
////								gdispDrawString(0 + relativeJoystickPositionX,
////										15 + relativeJoystickPositionY, str2, font1, Black);
//
////					// Wait for display to stop writing
////					xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
////					// swap buffers
////					ESPL_DrawLayer();
//
//					vTaskDelayUntil(&xLastWakeTime, tickFramerate);
//
//				break;
//		}

			// *********************************************** END SECOND STATE *******************************************************

//		default:
//			// *********************************************** THIRD STATE ************************************************************
//			{
//				gdispClear(White);
//
//				// Wait for display to stop writing
//				xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
//				// swap buffers
//				ESPL_DrawLayer();
//
//				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
//				break;
//
//			}
//		}
//			 *********************************************** END THIRD STATE *********************************************************
	}
}
}


/**
 * This task polls the joystick value every 20 ticks
 */
void checkJoystick() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct coord joystickPosition = { 0, 0 };
	const TickType_t tickFramerate = 20;

	while (TRUE) {
		// Remember last joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(
				ESPL_ADC_Joystick_2) >> 4);
		joystickPosition.y = (uint8_t) 255
				- (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		xQueueSend(JoystickQueue, &joystickPosition, 100);

		// Execute every 20 Ticks
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

/**
 * Example function to send data over UART
 *
 * Sends coordinates of a given position via UART.
 * Structure of a package:
 *  8 bit start byte
 *  8 bit x-coordinate
 *  8 bit y-coordinate
 *  8 bit checksum (= x-coord XOR y-coord)
 *  8 bit stop byte
 */
void sendPosition(struct coord position) {
	const uint8_t checksum = position.x ^ position.y;

	UART_SendData(startByte);
	UART_SendData(position.x);
	UART_SendData(position.y);
	UART_SendData(checksum);
	UART_SendData(stopByte);
}

/**
 * Example how to receive data over UART (see protocol above)
 */
void uartReceive() {
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[5]; // Start byte,4* line byte, checksum (all xor), End byte
	struct coord position = { 0, 0 };
	while (TRUE) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

		// decode package by buffer position
		switch (pos) {
		// start byte
		case 0:
			if (input != startByte)
				break;
		case 1:
		case 2:
		case 3:
			// read received data in buffer
			buffer[pos] = input;
			pos++;
			break;
		case 4:
			// Check if package is corrupted
			checksum = buffer[1] ^ buffer[2];
			if (input == stopByte || checksum == buffer[3]) {
				// pass position to Joystick Queue
				position.x = buffer[1];
				position.y = buffer[2];
				xQueueSend(JoystickQueue, &position, 100);
			}
			pos = 0;
		}
	}
}

/*
 *  Hook definitions needed for FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while (TRUE) {
	};
}

unsigned int short checkButtonA() {
	unsigned char readingA = GPIO_ReadInputDataBit(ESPL_Register_Button_A,
			ESPL_Pin_Button_A);
	if (readingA != tmpA) {
		lastPressedTick = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastPressedTick) > 10) {
		if (readingA != buttonStateA) {
			buttonStateA = readingA;

			if (buttonStateA == 0) {
				return (uint8_t)1;
			}
		}
	}
	tmpA = readingA;
	return (uint8_t)0;
}

unsigned int short checkButtonB() {

	unsigned char readingB = GPIO_ReadInputDataBit(ESPL_Register_Button_B,
			ESPL_Pin_Button_B);
	if (readingB != tmpB) {
		lastPressedTick = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastPressedTick) > 10) {
		if (readingB != buttonStateB) {
			buttonStateB = readingB;

			if (buttonStateB == 0) {
				return 1;
			}
		}
	}
	tmpB = readingB;
	return 0;
}

unsigned int short checkButtonC() {
	unsigned char readingC = GPIO_ReadInputDataBit(ESPL_Register_Button_C,
			ESPL_Pin_Button_C);
	if (readingC != tmpC) {
		lastPressedTick = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastPressedTick) > 10) {
		if (readingC != buttonStateC) {
			buttonStateC = readingC;

			if (buttonStateC == 0) {
				return 1;
			}
		}
	}
	tmpC = readingC;
	return 0;
}

unsigned int short checkButtonD() {
	unsigned char readingD = GPIO_ReadInputDataBit(ESPL_Register_Button_D,
			ESPL_Pin_Button_D);
	if (readingD != tmpD) {
		lastPressedTick = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastPressedTick) > 10) {
		if (readingD != buttonStateD) {
			buttonStateD = readingD;

			if (buttonStateD == 0) {
				return 1;
			}
		}
	}
	tmpD = readingD;
	return 0;
}

unsigned int short checkButtonK() {
	unsigned char readingK = GPIO_ReadInputDataBit(ESPL_Register_Button_K,
			ESPL_Pin_Button_K);
	if (readingK != tmpK) {
		lastPressedTick = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastPressedTick) > 10) {
		if (readingK != buttonStateK) {
			buttonStateK = readingK;
			if (buttonStateK == 0) {
				return 1;
			}
		}
	}
	tmpK = readingK;
	return 0;
}

unsigned int short checkButtonE() {
	unsigned char readingE = GPIO_ReadInputDataBit(ESPL_Register_Button_E,
			ESPL_Pin_Button_E);
	if (readingE != tmpE) {
		lastPressedTick = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastPressedTick) > 10) {
		if (readingE != buttonStateE) {
			buttonStateE = readingE;
			if (buttonStateE == 0) {
				return 1;
			}
		}
	}
	tmpE = readingE;
	return 0;
}

//QUESTIONS
/*
 * 1) unsigned int short vs unsigned char in checkButton
 *2) Tick frequency ?
 *3) Reference of p3 in TaskC, why doesnt it change ?
 *
 *
 *---------------------
 *--------------------- NOW WORKING PROPERLY ALWAYS BLUE FIRST
 *void staticTask()
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 1000;

	uint16_t circleRadius = 50;
	uint16_t circleLineWidth = 3;

	while(1)
	{
		gdispClear(White);
		gdispFillCircle(displaySizeX/2 - 50  ,displaySizeY/2 , circleRadius, Blue);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

		while(1);

		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}

}

void dynamicTask()
{
	vTaskDelay(500);
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 1000;

	uint16_t circleRadius = 50;
	uint16_t circleLineWidth = 3;

	while(1)
	{
		gdispClear(White);
		gdispFillCircle(displaySizeX/2 + 50 ,displaySizeY/2 , circleRadius, Red);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

		while(1);

		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}
 *
 *
 *
 *
 *
 *
 */

//FOR STATIC AND I DONT KNOW WHERE TO PUT IT IN
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

