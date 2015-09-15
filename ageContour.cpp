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
 
		cvShowImage("Museum",frame);
        printf("Frame number %d \n",i);
        printf("Dims %d %d \n",frame->width,frame->height);        
    	i++;
    }	
    	
    cvReleaseCapture(&capture);
	cvDestroyWindow("Museum");
    return 0;
}
