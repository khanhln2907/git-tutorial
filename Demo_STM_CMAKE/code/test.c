#include <stdio.h>
#include <math.h>
#define mPI acos(-1.0)

int main()
{
    double anglePositionSquare = 0;
	double anglePositionCircle = mPI;
	double distanceOfObjects = 30; // rotating Radius aroung the triangle
    
    if(anglePositionCircle <= 2*mPI)
		{
			anglePositionCircle = anglePositionCircle + 2*mPI - mPI/360;
		}
		else
		{
			anglePositionCircle = anglePositionCircle + mPI/360;
		}
		
		if(anglePositionSquare <= 2*mPI)
		{
			anglePositionSquare = anglePositionSquare + 2*mPI - mPI/360;
		}
		else
		{
			anglePositionSquare = anglePositionSquare + mPI/360;
		}
		
	while(1)
	{
	    printf("AngleCircle:%d \n AngleSquare:%d \n X:%d \n  Y:%d  \n\n", anglePositionCircle, anglePositionSquare,0, 0);
	}
		
	

    return 0;
}
