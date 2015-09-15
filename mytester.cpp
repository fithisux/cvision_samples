//#include "FrameWienerPredictor.h"
#include "bgUtilities.h"
#include "confParameters.h"
#include "RegionLevel.h"
#include "FrameLevel.h"
#include "cv.h"
#include <stdio.h>
#include <iostream>
using namespace std;

int main( int argc, char** argv )
{
	CvCapture * capture;
    //IplImage* frame = cvCreateImage(cvSize(700,500),IPL_DEPTH_8U,3);
	IplImage* frame = NULL;
	IplImage* image=NULL;
    IplImage* temp_for;
	IplImage ** image_array;
	CvRect whiterect;
				
	confParameters *params = new confParameters();
	printf("Init predictor !!");        	
	FrameWienerPredictor *apred = new FrameWienerPredictor(params);
	
	RegionLevel *region_level=NULL;
	FrameLevel *fr_level=NULL;			
	printf("Go On !!");        	
	
	printf("Read from file !! \n");
	capture = cvCaptureFromFile(params->movie_name);
	CvHaarClassifierCascade* cascade = load_object_detector(params->csd_name);
	printf("File read !! \n");
	
	imageShower* sh1 = new imageShower("Museum");
	imageShower* sh2 = new imageShower("Prediction");
	imageShower* sh3 = new imageShower("UFO");
	imageShower* sh4 = new imageShower("Test");
	
	int i=-1;
	CvPoint pp2 = cvPoint(11,251);

	int counter;

	for(;;)
    {    
		i++;
		printf("Go !! \n");

        frame = cvQueryFrame(capture);
		
		//createCircle(frame,&pp2,20.0);
		//pp2.x += 1;

		if(frame == NULL)
        {
        	 printf("Frame is NULL !!!");			 
        	 break;
        }			

		if(i < params->frame_delay) continue;

		sh1->myShowImage(frame);
		//De-interlacing
		//cleverDeInterlace(frame);
		deInterlace(frame);		
		//sh2->myShowImage(frame);

		if(i==params->frame_delay)
		{
			temp_for = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
			temp_for->origin = frame->origin;
			image = cvCreateImage(cvGetSize(frame),IPL_DEPTH_32F,3);
			image->origin=frame->origin;
			image_array = new IplImage*[5];
			char fname[50];
			IplImage *img_ptr;
			for(int l=0;l<5;l++)
			{
				sprintf(fname,"core%d.bmp",l);
				img_ptr=cvLoadImage(fname);
				image_array[l]=cvCloneImage(img_ptr);
				cvConvertImage(img_ptr,image_array[l],CV_CVTIMG_FLIP);
				cvReleaseImage(&img_ptr);
			}	
		}
		
		if(i > params->frame_delay)
        {
        	cout << "Starting \n";

			//Convert  byte image to float image
			convertByte2Float(frame,image);
			//convertFloat2Byte(image,temp_icon);		
			apred->updateWithFrame(image);
			
			char s[100];
			if( (i>=210) && (i<=211) )
			{
					sprintf(s,"original%d.bmp",i);
					cvSaveImage(s,image);
			}

			//is predictor ready?
        	if(apred->isReady())
        	{
	        	printf("Predicting with frame %d \n",i);
				cvCopy(apred->foreground,temp_for);
        	/*
				printf("Executing frame-level\n");				
				if(fr_level == NULL)
				{
					fr_level=new FrameLevel(image_array,5,0,0.7,my_threshold);
				}
				
				if(fr_level->switchWithFrame(frame,temp_for))
				{
					printf("Switching necessary \n");
					fr_level->recreateMask(frame,temp_for);
				}
			*/				
				printf("Executing region level");
				if(region_level==NULL)
				{					
					region_level=new RegionLevel(frame,temp_for,params);
				}				
				
				region_level->updateWithFrameAndMarked(frame,temp_for);
				sh2->myShowImage(region_level->current_K);
				sh3->myShowImage(region_level->temp_storage);
				
				if(params->face_flag)
				{
					whiterect = GetNonZeroRect(region_level->current_L);
					cvSetImageROI(frame,whiterect);
					detect_and_draw_objects(frame,cascade,0);
					sh4->myShowImage(frame);
				}
					
        	}
        	else	printf("Frame %d before prediction \n",i);
        	
        	
        }        
		
        printf("Frame number %d \n",i);        	
		char c=cvWaitKey(10);
    }	

	delete sh1;
	delete sh2;
	delete sh3;
	delete sh4;
	cvReleaseCapture(&capture);
	cvReleaseHaarClassifierCascade( &cascade );	        
	if(image != NULL) cvReleaseImage(&image);
	delete apred;
	delete region_level;
	delete fr_level;
    return 0;
}
