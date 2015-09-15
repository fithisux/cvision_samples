//#include "FrameWienerPredictor.h"
#include "RegionLevel.h"
#include "FrameLevel.h"
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <iostream>
using namespace std;

int main( int argc, char** argv )
{
	CvCapture * capture;
    //IplImage* frame = cvCreateImage(cvSize(700,500),IPL_DEPTH_8U,3);
	IplImage* frame = NULL;
	IplImage *image;
    unsigned char *runner1;
	int train_size=5;

	KMeans *my_kmeans = new KMeans(train_size);			
	printf("Go On !!");        	
	
	capture = cvCaptureFromFile("c:\\museum.avi");
	printf("Read from file !!");	
	
	char file_name[50];

	cvNamedWindow("Display",CV_WINDOW_AUTOSIZE);

	int counter=0;
	int i=-1,l,k;
	
	for(;;)
    {    
		i++;
		printf("Go !! \n");

        frame = cvQueryFrame(capture);		
		
		if(frame == NULL)
        {
        	 printf("Frame is NULL !!!");			 
        	 break;
        }			

		if(i==0) image=cvCloneImage(frame);
		else cvCopy(frame,image);
		if(i < 419) continue;

		cvShowImage("Display",image);
		runner1=(unsigned char *) frame->imageData;

		for(l=0;l<frame->imageSize;l++)
		{					
			if( (l / frame->widthStep) % 2 == 1)
			{
				(*runner1) = (*(runner1-frame->widthStep));
			}
			runner1++;
		}


		if(!my_kmeans->isReady())
		{
			printf("Training epoch %d \n",counter);
			my_kmeans->updateCore(frame);
			counter++;
		}
		else
		{
			printf("Kmeans is ready \n");
			printf("Training epoch %d \n",counter);
			my_kmeans->updateCore(frame);
			counter++;
		}

		if(counter == 50)
		{			
			printf("Saving files ...\n");
			for(k=0;k<train_size;k++)
			{
				sprintf(file_name,"core%d.bmp",k);
				cvSaveImage(file_name,my_kmeans->cores[k]);
			}
			break;
		}
	}

	printf("Exiting \n");
    cvReleaseCapture(&capture);
	cvReleaseImage(&image);
	cvDestroyWindow("Display");
	delete my_kmeans;
	return 0;
}
