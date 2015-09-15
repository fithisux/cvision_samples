#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include <stdio.h>

int main( int argc, char** argv )
{
	CvCapture * capture;
    IplImage* frame = NULL;
    int i=0;
	        	
	capture = cvCaptureFromFile("c:\\museum.avi");
	cvNamedWindow("Museum",1);
    
	i=0;
	for(;;)
    {       
        frame = cvQueryFrame(capture);
		printf("Hoop !!!");		
        if(frame == NULL)
        {
        	 printf("Frame is NULL !!!");
        	 break;
        }
 
		printf("Frame number %d \n",i);		
    	i++;
		if(i==418)
		{
			cvSaveImage("mybg.bmp",frame);
			printf("Image saved \n");
			break;
		}
    }	
    	
    cvReleaseCapture(&capture);
	cvDestroyWindow("Museum");
	//cvReleaseImage(&frame);
    return 0;
}
