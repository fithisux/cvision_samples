#include "FrameLevel.h"
#include <stdio.h>
#include "cxcore.h"
#include "cv.h"

FrameLevel::FrameLevel(IplImage** somecores,int length,int prefered,float sw_thres,float comp_thres)
{
	this->cores=somecores; //an array for core for frame-level comparisons
	this->the_size=length;// the size of the above mentioned array
	this->representative_model=prefered; //the initially prefered representative_model
	this->switch_threshold=sw_thres; //threshold in percentage of false pixels for bg
	this->comparison_threshold=comp_thres;//threshold for comparison between an 
											//image and a core, we use L^{infinity} norm
	this->temp_storage=NULL;//temporary storage for computations
}

FrameLevel::~FrameLevel()
{
	cvReleaseImage(&this->temp_storage);
}


//Switch with frame gives indications wether the frame triggers
//and it corresponding pixel-level bg/fg extraction triggers any switching
bool FrameLevel::switchWithFrame(IplImage* frame,IplImage *initial_backextr)
{
	float someratio,temp;	
	
	if(this->temp_storage == NULL) this->temp_storage=cvCloneImage(frame);
	someratio=
	this->findRatio(this->representative_model,frame,initial_backextr);
	
	printf("ratio found is %f \n",someratio);

	if(someratio >= this->switch_threshold)
	{
		temp=someratio;
		for(int i=0;i<this->the_size;i++)
		{
			someratio=this->findRatio(i,frame,initial_backextr);
			if(someratio< temp)
			{
				temp=someratio;
				this->representative_model=i;
			}
		}
		return true;
	}
	else return false;	
}


//It enriches the initial_backextr by the detection of false bg pixels
void FrameLevel::recreateMask(IplImage* frame,IplImage *initial_backextr)
{	
	cvAbsDiff(frame,this->cores[this->representative_model],this->temp_storage);

	cvThreshold(this->temp_storage,this->temp_storage,
				this->comparison_threshold,255,CV_THRESH_BINARY);

	unsigned char *s=(unsigned char *) initial_backextr->imageDataOrigin;
	unsigned char *runner=(unsigned char *) this->temp_storage->imageDataOrigin;
	
	for(int i=0;i<initial_backextr->imageSize;i++)
	{
//this is a hack for L^{infinity} norm computation
		//check if the pixel is marked as bg
		if((*s)==0) 
		{
			//if it is bg, check if it is "Far" from core in one of the colors
			//if yes, update to 255 ( we use 255 to directly map the image to 
			//black and white		
			if( (*runner) || (*(runner+1)) || (*(runner+2)) ) (*s)=255;
		}
		runner+=3;
		s++;
	}	
		
}

//finds the ratio of false bg pixels according to this>cores[core_no]
float FrameLevel::findRatio(int core_no,IplImage* frame,IplImage *initial_backextr)
{
	int back_pixels,false_pixels;
	
	back_pixels=false_pixels=0;
	
	//We initialize for L^{infinity} distance
	cvAbsDiff(frame,this->cores[core_no],this->temp_storage);

	cvThreshold(this->temp_storage,this->temp_storage,
				this->comparison_threshold,255,CV_THRESH_BINARY);

	unsigned char *s=(unsigned char *) initial_backextr->imageDataOrigin;
	unsigned char *runner=(unsigned char *) this->temp_storage->imageDataOrigin;
	
	for(int i=0;i<initial_backextr->imageSize;i++)
	{
		//this is a hack for L^{infinity} norm computation
		//check if the pixel is marked as bg
		if((*s)==0) 
		{
			//if it is bg, increase back_pixels, 
			//check if it is "Far" from core in one of the colors
			//increase the number of false pixels
			//we do not make any updates according to the wallflower paper
			back_pixels++;			
			if( (*runner) || (*(runner+1)) || (*(runner+2)) ) false_pixels++;
		}
		runner+=3;
		s++;
	}		
	return ((back_pixels==0) ? 1.0 : ((1.0*false_pixels)/(1.0*back_pixels)));
}
