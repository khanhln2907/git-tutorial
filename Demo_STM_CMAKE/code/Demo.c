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
#define mPI acos(-1.0)
#define debounceTick 50
#define defaultButton 1

// start and stop bytes for the UART protocol
static const uint8_t startByte = 0xAA, stopByte = 0x55;

static const uint16_t displaySizeX = 320, displaySizeY = 240;

//Static Stack
#define STACK_SIZE 100
/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStack[STACK_SIZE];


//State Machine
unsigned int state = 1;

//relative Position of Joystick
unsigned char tmpA = 1;
unsigned char buttonStateA = 0;
unsigned char tmpB = 1;
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

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

// Stores lines to be drawn
QueueHandle_t JoystickQueue;

int main() {
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	/*
	 *	THIS IS PART OF THE DEMO*/
	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));

	// Initializes Tasks with their respective priority
	//xTaskCreate(drawTask, "drawTask", 1000, NULL, 5, NULL);
	//xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 3, NULL);
	//xTaskCreate(changeState, "changeState", 1000, NULL, 4, NULL);
	//TaskHandle_t xHandle = NULL;
	xTaskCreate(dynamicTask, "dynamicTask", 1000, NULL, 7, NULL);
	xTaskCreateStatic(staticTask, "staticTask", STACK_SIZE, NULL, 6, xStack, &xTaskBuffer);
	//xTaskCreate(staticTask, "staticTask", STACK_SIZE, NULL, 5,NULL);


	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

void staticTask()
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
	//vTaskDelay(500);
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
				state = 1;
			}

		}

		vTaskDelayUntil(&xLastWakeTime, tickFramerate); //NEED TO ASK TUTOR !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
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

	char dynamicString[50] = { "EXERCISE_2" };
	uint16_t dynamicStringPositionX = 0;
	uint16_t dynamicStringPositionY = displaySizeY - 30;
	uint16_t dynamicStringTmp = 0;

	while (TRUE) {

		switch(state)
		{
			case 1:
			// *********************************************** FIRST STATE *******************************************************
			{

				//		// wait for buffer swap
				while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE)
					;
				gdispClear(White);

				//EXERCISE 2
				//2.1 EXERCISE 3 FIGURES ----------------------------------------------------------------------------------------------------------------
				//CALCULATING NEW ANGLE POSITION FOR ROTATING FIGURES
				double rotatingSpeed = 22.5;
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
				dynamicStringPositionX = (dynamicStringPositionX + 1)
						% displaySizeX;
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
				gdispDrawString(0 + relativeJoystickPositionX,
						30 + relativeJoystickPositionY, str, font1, Black);

				// Wait for display to stop writing
				xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
				// swap buffers
				ESPL_DrawLayer();

				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
				break;
			}
			// ******************************************** END FIRST STATE *******************************************************

		case 2:
			// ********************************************* SECOND STATE *********************************************************
			{
				//gdispClear(White);
				font_t font1; // Load font for ugfx
				font1 = gdispOpenFont("DejaVuSans24*");

				char str[100]; // buffer for messages to draw to display

				gdispClear(White);
				sprintf(str, "A: %d |B: %d |C: %d |D: %d",
						GPIO_ReadInputDataBit(ESPL_Register_Button_A,
								ESPL_Pin_Button_A),
						GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B),
						GPIO_ReadInputDataBit(ESPL_Register_Button_C,
								ESPL_Pin_Button_C),
						GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D));
				gdispDrawString(0, 0, str, font1, Black);

				// Wait for display to stop writing
				xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
				// swap buffers
				ESPL_DrawLayer();

				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
				break;
			}
			// *********************************************** END SECOND STATE *******************************************************

		default:
			// *********************************************** THIRD STATE ************************************************************
			{
				gdispClear(White);

				// Wait for display to stop writing
				xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
				// swap buffers
				ESPL_DrawLayer();

				vTaskDelayUntil(&xLastWakeTime, tickFramerate);
				break;

			}

			// *********************************************** END THIRD STATE *********************************************************
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
 *
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

