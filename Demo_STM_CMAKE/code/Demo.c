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

// start and stop bytes for the UART protocol
static const uint8_t startByte = 0xAA,
					 stopByte  = 0x55;

static const uint16_t displaySizeX = 320,
					  displaySizeY = 240;

//relative Position of Joystick
int relativeJoystickPositionX = 0;
int relativeJoystickPositionY = 0;

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
	xTaskCreate(drawTask, "drawTask", 1000, NULL, 4, NULL);
	xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 3, NULL);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();

}

void calculateNewPosition(uint16_t rotatingPointX, uint16_t rotatingPointY, uint16_t* positionX, uint16_t* positionY)
{
	// CALCUALTING THE OLD ANGLE
	double pi = acos(-1.0);
	double rotatingAngle = 2* pi / 360 /8; //rotate 1 Grad each time

	double angle = atan((abs(rotatingPointY - *positionY))/(abs(rotatingPointX - *positionX)));
	if(rotatingPointX - *positionX > 0)
	{
		;
	}
	else if(((rotatingPointX - *positionX) < 0) && ((rotatingPointY - *positionY)>= 0))
	{
		angle = angle + pi;
	}
	else if(((rotatingPointX - *positionX) < 0) && ((rotatingPointY - *positionY)< 0))
	{
		angle = angle - pi;
	}
	else ;

	// Here is the problem
	//double rotatingRadius = sqrt(abs(((double) rotatingPointX - (double) *positionX) * ((double) rotatingPointX - (double) *positionX) + ((double) rotatingPointY - (double) *positionY)*((double)rotatingPointY - (double)*positionY)));

	double rotatingRadius = 40;
	angle = angle + rotatingAngle; // Change the angle each time rotating

	*positionX = *positionX + rotatingRadius*cos(angle);
	*positionY = *positionY + rotatingRadius*sin(angle);

}

void displayExercise2()
{


//	gdispClear(White);
//	// DISPLAY A CIRCLE
//	gdispFillCircle(circlePositionX, circlePositionY, circleRadius, Red);
//	gdispFillCircle(circlePositionX, circlePositionY, circleRadius - circleLineWidth , White);
//
//	//DISPLAY A TRIANGLE
//	gdispDrawLine(trianglePointA_X, trianglePointA_Y, trianglePointB_X, trianglePointB_Y, Blue);
//	gdispDrawLine(trianglePointB_X, trianglePointB_Y, trianglePointC_X, trianglePointC_Y, Blue);
//	gdispDrawLine(trianglePointC_X, trianglePointC_Y, trianglePointA_X, trianglePointA_Y, Blue);
//
//	//DISPLAY A SQUARE
//	gdispFillArea(squarePositionX, squarePositionY, squareLength, squareLength, Green);

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

	//Counting Buttons
	uint16_t cntA = 0;
	uint16_t cntB = 0;
	uint16_t cntC = 0;
	uint16_t cntD = 0;
	//Initial Points and Positions
	double distanceOfObjects = 50; // rotating Radius aroung the triangle (middle point of screen)

	double anglePositionCircle = mPI;
	uint16_t circleRadius = 10;
	uint16_t circleLineWidth = 3;
	uint16_t circlePositionX = displaySizeX/2 - distanceOfObjects ; // Left X Axis
	uint16_t circlePositionY = (float)1/2 * displaySizeY; // 1/3 Top Y Axis

	uint16_t trianglePointA_X =  displaySizeX/2 -15;
	uint16_t trianglePointA_Y = (float)1/2 * displaySizeY + 15;
	uint16_t trianglePointB_X = displaySizeX/2 + 15;
	uint16_t trianglePointB_Y = (float)1/2 * displaySizeY + 15;
	uint16_t trianglePointC_X = displaySizeX/2;
	uint16_t trianglePointC_Y = (float)1/2 * displaySizeY - 15;

	double anglePositionSquare = 0;
	uint16_t squareLength = 20 ;
	uint16_t squarePositionX = displaySizeX/2 + distanceOfObjects - squareLength ; // Right X Axis, 3rt term is linear translation to calibrate middle point of the square
	uint16_t squarePositionY = (float)1/2 * displaySizeY - squareLength;

	char staticString[50] = {"HELLO ESPL"}; // Amount of character ?
	uint16_t staticStringPositionX = displaySizeX/2 - 30;
	uint16_t staticStringPositionY = displaySizeY - 45;

	char dynamicString[50] = {"EXERCISE_2"};
	uint16_t dynamicStringPositionX = 0;
	uint16_t dynamicStringPositionY =  displaySizeY - 30 ;
	uint16_t dynamicStringTmp = 0;



//	/* building the cave:
//	   caveX and caveY define the top left corner of the cave
//	    circle movment is limited by 64px from center in every direction
//	    (coordinates are stored as uint8_t unsigned bytes)
//	    so, cave size is 128px */
//	const uint16_t caveX    = displaySizeX/2 - UINT8_MAX/4,
//				   caveY    = displaySizeY/2 - UINT8_MAX/4,
//				   caveSize = UINT8_MAX/2;
//	uint16_t circlePositionX = caveX,
//			 circlePositionY = caveY;
//
//	// Start endless loop

while(TRUE) {
	//		// wait for buffer swap
	while(xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE)
			;
	gdispClear(White);

	//EXERCISE 2
	//2.1 EXERCISE 3 FIGURES ----------------------------------------------------------------------------------------------------------------
	//Calculating new angle for figures
	if(anglePositionCircle <= -2*mPI)
	{
		anglePositionCircle = anglePositionCircle + 2*mPI - mPI/360;
	}
	else
	{
		anglePositionCircle = anglePositionCircle + mPI/360;
	}

	if(anglePositionSquare <= -2*mPI)
	{
		anglePositionSquare = anglePositionSquare + 2*mPI - mPI/360;
	}
	else
	{
		anglePositionSquare = anglePositionSquare + mPI/360;
	}

	//Calculating new position for the dynamic string
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

	//CALIBRATING JOYSTICKPOSITION
	if (((abs(joystickPosition.x - 127)/2) >= 10) | ((abs(joystickPosition.y - 127)/2) >= 10)) // Avoid noise
	{
		relativeJoystickPositionX = (joystickPosition.x - 127)/2; // relative position for programmer, 2 is the moving speed
		relativeJoystickPositionY = (joystickPosition.y - 127)/2;
	}
	else
	{
		relativeJoystickPositionX = 0;
		relativeJoystickPositionY = 0;
	}

	//SHOW A TRIANGLE
	gdispDrawLine(trianglePointA_X + relativeJoystickPositionX, trianglePointA_Y + relativeJoystickPositionY, trianglePointB_X + relativeJoystickPositionX, trianglePointB_Y + relativeJoystickPositionY, Blue);
	gdispDrawLine(trianglePointB_X + relativeJoystickPositionX, trianglePointB_Y + relativeJoystickPositionY, trianglePointC_X + relativeJoystickPositionX, trianglePointC_Y + relativeJoystickPositionY, Blue);
	gdispDrawLine(trianglePointC_X + relativeJoystickPositionX, trianglePointC_Y + relativeJoystickPositionY, trianglePointA_X + relativeJoystickPositionX, trianglePointA_Y + relativeJoystickPositionY, Blue);

	//DRAW MIDDLE POINT
	gdispDrawPixel(160, 120, Red);

	//SHOW A ROTATING CIRCLE
	double dynamicPositionCircleX = displaySizeX/2 + distanceOfObjects * cos(anglePositionCircle); // relative to centre point of the screen X Axis
	double dynamicPositionCircleY = displaySizeY/2 + distanceOfObjects * sin(anglePositionCircle); // relative to centre point of the scree Y Axis
	gdispFillCircle(dynamicPositionCircleX + relativeJoystickPositionX, dynamicPositionCircleY + relativeJoystickPositionY, circleRadius, Red);
	gdispFillCircle(dynamicPositionCircleX + relativeJoystickPositionX, dynamicPositionCircleY + relativeJoystickPositionY, circleRadius - circleLineWidth , White);

	//SHOW A ROTATING SQUARE
	double dynamicPositionSquareX = displaySizeX/2 - squareLength/2 + distanceOfObjects * cos(anglePositionSquare);  // relative to centre point of the screen X Axis
	double dynamicPositionSquareY = displaySizeY/2 - squareLength/2 + distanceOfObjects * sin(anglePositionSquare);
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
	if(!GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A))
	{
		while(!GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A));
		cntA++;
	}
	if(!GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B))
	{
		while(!GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B));
		cntB++;
	}
	if(!GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C))
	{
		while(!GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C));
		cntC++;
	}
	if(!GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D))
	{
		while(!GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D));
		cntD++;
	}
	if(!GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K))
	{
		while(!GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K));
		cntA = 0;
		cntB = 0;
		cntC = 0;
		cntD = 0;
	}

	// Generate string with current joystick values
	sprintf( str, "A: %d, #: %d |B: %d, #: %d |C %d, #: %d |D: %d, #: %d",
			 GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A), cntA,
			 GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B), cntB,
			 GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C), cntC,
			 GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D), cntD);
	sprintf( str2, "E: %d|K: %d|X: %d, Y: %d",
			GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E),
			GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K),
			joystickPosition.x, joystickPosition.y);
	// Print string of joystick values
	gdispDrawString(0 + relativeJoystickPositionX, 0 + relativeJoystickPositionY, str, font1, Black);
	gdispDrawString(0 + relativeJoystickPositionX, 15 + relativeJoystickPositionY, str2, font1, Black);


	//2.3 JOYSTICK -----------------------------------------------------------------------------------------------------------------------
	// Generate string with current joystick values
	sprintf( str, "Axis 1: %5d|Axis 2: %5d|VBat: %5d",
			 ADC_GetConversionValue(ESPL_ADC_Joystick_1),
			 ADC_GetConversionValue(ESPL_ADC_Joystick_2),
			 ADC_GetConversionValue(ESPL_ADC_VBat) );

	//ADDING THESE VALUES TO MOVE SCREEN
	//joystickPosition.x/2
	//joystickPosition.y/2

	// Print string of joystick values
	gdispDrawString(0 + relativeJoystickPositionX, 30 + relativeJoystickPositionY, str, font1, Black);


//
//		// Clear background
//		gdispClear(White);
//		// Draw rectangle "cave" for circle
//		// By default, the circle should be in the center of the display.
//		// Also, the circle can only move by 127px in both directions (position is limited to uint8_t)
//		gdispFillArea(caveX-10, caveY-10, caveSize + 20, caveSize + 20, Red);
//		// color inner white
//		gdispFillArea(caveX, caveY, caveSize, caveSize, White);
//
//		// Generate string with current joystick values
//		sprintf( str, "Axis 1: %5d|Axis 2: %5d|VBat: %5d",
//				 ADC_GetConversionValue(ESPL_ADC_Joystick_1),
//				 ADC_GetConversionValue(ESPL_ADC_Joystick_2),
//				 ADC_GetConversionValue(ESPL_ADC_VBat) );
//		// Print string of joystick values
//		gdispDrawString(0, 0, str, font1, Black);
//
//		// Generate string with current joystick values
//		sprintf( str, "A: %d|B: %d|C %d|D: %d|E: %d|K: %d",
//				 GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A),
//				 GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B),
//				 GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C),
//				 GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D),
//				 GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E),
//				 GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K) );
//		// Print string of joystick values
//		gdispDrawString(0, 11, str, font1, Black);
//
//		// Draw Circle in center of square, add joystick movement
//		circlePositionX = caveX + joystickPosition.x/2;
//		circlePositionY = caveY + joystickPosition.y/2;
//		gdispFillCircle(circlePositionX, circlePositionY, 10, Green);





		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

		vTaskDelayUntil(&xLastWakeTime, tickFramerate);

	}
}

/**
 * This task polls the joystick value every 20 ticks
 */
void checkJoystick() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct coord joystickPosition = {0, 0};
	const TickType_t tickFramerate = 20;

	while (TRUE) {
		// Remember last joystick values
		joystickPosition.x =
					(uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		joystickPosition.y = (uint8_t) 255 -
						 (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

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
	const uint8_t checksum = position.x^position.y;

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
	struct coord position = {0, 0};
	while (TRUE) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

		// decode package by buffer position
		switch(pos) {
		// start byte
		case 0:
			if(input != startByte)
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
			checksum = buffer[1]^buffer[2];
			if(input == stopByte || checksum == buffer[3]) {
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
	while(TRUE) {
	};
}
