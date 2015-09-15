#include "FrameWienerPredictor.h"
#include <stdio.h>
#include <iostream>
#include "bgUtilities.h"

using namespace std;
//
// FrameWiener predictor implementation
//

FrameWienerPredictor::FrameWienerPredictor(confParameters* params)
{
	cout << params->past_length;

	this->size=params->past_length; //size of the array
	this->past_frames = new IplImage* [this->size]; //aray of past frames
	this->coefficients = new IplImage* [this->size]; //array of coefficients
	this->foreground=NULL;//contains the thresholded difference 
						//between prediction and actual value
	this->prediction = NULL; //contains the prediction
	this->temp_storage=NULL; //useful for manipulations	
	this->some_threshold=params->diff_threshold;//threshold	
	this->ready=0; //ready==1 means that this->counter past frames have filled 
					//this->past_frames
	

	int * some_values = new int[4];
	some_values[0]=some_values[1]=some_values[2]=some_values[3]=1;
	this->struct_elem=cvCreateStructuringElementEx(2,2,0,0,CV_SHAPE_RECT,some_values);	
	delete some_values;
}

FrameWienerPredictor::~FrameWienerPredictor()
{
	cvReleaseImage(&this->prediction);
	cvReleaseImage(&this->temp_storage);	
	cvReleaseImage(&this->foreground);
	for(int i=0;i<this->size;i++)
	{
		 if(this->past_frames[i]) cvReleaseImage(&this->past_frames[i]);
	}
	delete this->past_frames;
	for(i=0;i<this->size;i++)
	{
		 if(this->coefficients[i]) cvReleaseImage(&this->coefficients[i]);
	}
	delete this->coefficients;
	cvReleaseStructuringElement(&this->struct_elem);
}

//test if ready
bool FrameWienerPredictor::isReady()
{
	return (this->ready==1);
}

//updates with frames until ready
void FrameWienerPredictor::updateWithFrame(IplImage* some_frame) 
{
	float some_val;
	CvSize asz;
	float *ptr;
	int i,j;
	IplImage *temp;

	static counter=this->size;

	asz=cvSize(some_frame->width,some_frame->height);
	
	
	if(counter > 0)
	{
		this->past_frames[--counter]=cvCloneImage(some_frame);		
	}	
	
	if(counter==0)
	{
		//this is the weight of the lowest term
		//if we multiply with 3.0 we get the next and so on
		some_val = 2.0 * powf(1/3.0,this->size) / (1 - powf(1/3.0,this->size));

		for(i=0;i<this->size;i++)
		{			
			this->coefficients[i]=cvCreateImage(asz,IPL_DEPTH_32F,3);
						
			ptr = (float *) this->coefficients[i]->imageDataOrigin;
			
			for(j=0;j<this->coefficients[i]->imageSize;j+=4)
			{				
				(*ptr)=some_val;ptr++;
			}
			some_val= 3.0*some_val;
		}
		
		//We initialize the remaining variables
		this->prediction=cvCreateImage(asz,IPL_DEPTH_32F,3);
		this->prediction->origin=some_frame->origin;
		this->temp_storage=cvCreateImage(asz,IPL_DEPTH_32F,3);		
		this->temp_storage->origin=some_frame->origin;
		this->foreground=cvCreateImage(asz,IPL_DEPTH_8U,1);			
		this->foreground->origin=some_frame->origin;
		counter--;		
		return;
	}						
			
	if( (counter == -1) && (this->ready == 0)) this->ready=1;
    if(this->ready==1)
    {
		printf("We are inside \n");
       	this->computePrediction();		
		this->computeForeground(some_frame);
		
		//we push the new frame in place 0
		// the others are shifted to the past by 1
		temp = this->past_frames[this->size-1];
       	for(i=this->size-2;i>=0;i--)this->past_frames[i+1]=this->past_frames[i];		
		cvCopy(some_frame,temp);
		this->past_frames[0]=temp;        		
	}
}

//compute the foreground , in this->foreground 255 means fg 0 bg
void FrameWienerPredictor::computeForeground(IplImage * some_frame)
{
		float *frame_ptr, *pred_ptr;
		unsigned char * fg_ptr;
		float temp;
		
		frame_ptr=(float *) some_frame->imageDataOrigin;
		pred_ptr=(float *) this->prediction->imageDataOrigin;
		fg_ptr = (unsigned char *) this->foreground->imageDataOrigin;
		
		//Here we take the euclidean norm for the difference of colors
		//between prediction and current frame and threshold
		for(int i=0;i<this->foreground->imageSize;i++)
		{
			temp = ((*frame_ptr) - (*pred_ptr))*((*frame_ptr) - (*pred_ptr));			
			frame_ptr++;pred_ptr++;
			
			temp += ((*frame_ptr) - (*pred_ptr))*((*frame_ptr) - (*pred_ptr));			
			frame_ptr++;pred_ptr++;
			
			temp += ((*frame_ptr) - (*pred_ptr))*((*frame_ptr) - (*pred_ptr));			
			frame_ptr++;pred_ptr++;
			
			temp=sqrtf(temp);
			
			(*fg_ptr) = ( (temp >= this->some_threshold ) ? 255 : 0);
			fg_ptr++;			
		}	
		//We discard small clusters as noise
		cvErode(this->foreground,this->foreground,this->struct_elem,1);
	
}

//compute the prediction
void FrameWienerPredictor::computePrediction()
{		
	for(int i=0;i<this->size;i++)
	{			
		if(i==0)
		{
			cvMul(this->coefficients[i],this->past_frames[this->size-1-i],this->prediction,1.0);
		}
		else
		{
			cvMul(this->coefficients[i],this->past_frames[this->size-1-i],this->temp_storage,1.0);
			cvAdd(this->temp_storage,this->prediction,this->prediction,NULL);
		}
	}	
}

