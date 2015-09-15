#include "RegionLevel.h"
#include "connexe.h"

// page 2 , paper for parameters
RegionLevel::RegionLevel(IplImage *raw,IplImage *marked,confParameters *params)						
{
	this->previous_frame=cvCloneImage(raw);
	this->current_frame=cvCloneImage(raw);
	this->temp_storage=cvCloneImage(raw);

	this->previous_separation=cvCloneImage(marked);		
	this->previous_J=cvCloneImage(marked);
	this->current_J=cvCloneImage(marked);
	this->current_K=cvCloneImage(marked);
	this->current_L=cvCloneImage(marked);

	for(int k=0;k<3;k++)
	{
		this->image_array[k]=cvCloneImage(marked);
		this->hist_ranges[k] = new float [2];	
		this->hist_ranges[k][0]=0;
		this->hist_ranges[k][1]=256;	
		this->hist_sizes[k]=params->bin_num;
	}	
	this->hist= cvCreateHist(3,this->hist_sizes,CV_HIST_ARRAY,this->hist_ranges,1);

	cvSet(this->previous_J,cvScalarAll(255),NULL);
	this->kmotion=params->succ_threshold;
	this->kmin=params->region_area;
	this->myepsilon=params->bin_thres;

	int * some_values = new int[4];
	some_values[0]=some_values[1]=some_values[2]=some_values[3]=1;
	this->struct_elem=cvCreateStructuringElementEx(2,2,0,0,CV_SHAPE_RECT,some_values);	
	delete some_values;
}

RegionLevel::~RegionLevel()
{
	cvReleaseImage(&previous_frame);
	cvReleaseImage(&current_frame);
	cvReleaseImage(&previous_separation);
	cvReleaseImage(&previous_J);
	cvReleaseImage(&current_J);
	cvReleaseImage(&current_K);
	cvReleaseImage(&current_L);
	for(int k=0;k<3;k++)
	{
		delete hist_ranges[k];
		cvReleaseImage(&this->image_array[k]);
	}
	cvReleaseImage(&this->temp_storage);
	cvReleaseHist(&this->hist);
	cvReleaseStructuringElement(&this->struct_elem);
}

void RegionLevel::updateJandK()
{				
	unsigned char * fg_ptr,*frame_ptr, *pred_ptr;
	float temp;
		
	frame_ptr=(unsigned char *) this->current_frame->imageDataOrigin;
	pred_ptr=(unsigned char *) this->previous_frame->imageDataOrigin;
	fg_ptr = (unsigned char *) this->current_J->imageDataOrigin;
	
	//We take the euclidean distance between triples of colors 
	//in the current and previous frames ,threshold it,and put
	//to current_J (thresholded difference)
	for(int i=0;i<this->current_J->imageSize;i++)
	{
		temp = (1.0*(*frame_ptr) - 1.0*(*pred_ptr))*(1.0*(*frame_ptr) - 1.0*(*pred_ptr));
		frame_ptr++;pred_ptr++;
			
		temp += (1.0*(*frame_ptr) - 1.0*(*pred_ptr))*(1.0*(*frame_ptr) - 1.0*(*pred_ptr));
		frame_ptr++;pred_ptr++;
			
		temp += (1.0*(*frame_ptr) - 1.0*(*pred_ptr))*(1.0*(*frame_ptr) - 1.0*(*pred_ptr));
		frame_ptr++;pred_ptr++;
			
		temp=sqrtf(temp);
		
		(*fg_ptr) = ( (temp >= this->kmotion ) ? 255 : 0);
		fg_ptr++;			
	}
	
	//We find the intersection of current and previous J
	cvAnd(this->current_J,this->previous_J,this->current_K,NULL);
	//And filter with the foreground estimation , we put the result in K
	cvAnd(this->current_K,this->previous_separation,this->current_K,NULL);
	//We discard small clusters as noise
	cvErode(this->current_K,this->current_K,this->struct_elem,1);
	//and store current J for future reference
	cvCopy(this->current_J,this->previous_J,NULL);		
}

//Backprojects and thresholds in current_L
void RegionLevel::totalBackProjectIt()
{
	int img_area,i,j;
	unsigned char temp,*img_ptr1,*img_ptr2;

	//we backproject here from previous frame to next and put results in L
	cvClearHist(this->hist);
	
	//We split and calculate the histogram into an array (we work in RxGxB space)
	cvSplit(this->previous_frame,this->image_array[0],
			this->image_array[1],this->image_array[2],NULL);
	//The claculation is done with mask on the previous frame
	//In order to get the colours of the K pixels which are supposed
	//to belong to the foreground object
	cvCalcHist(this->image_array,hist,0,this->current_K);
	
	cvNormalizeHist(this->hist,255);

	//We backproject to the previous frame since it makes no great difference
	cvCalcBackProject(this->image_array,this->current_L,hist);	
    //We threshold the probability mapping with a hope to get rid of background pixels
	//which should conribute slightly to the histogram. We assume that K consists mainly
	//of foreground pixels 
	cvThreshold(this->current_L,this->current_L,this->myepsilon,255,CV_THRESH_BINARY);
	//cvMerge(this->current_L,this->current_L,this->current_L,NULL,this->temp_storage);
	
	//we grow 4-connected regions from pixels in K in order to eliminate 
	//non-foreground regions

	int bufferDims[3];

	bufferDims[0]=this->current_L->width;
	bufferDims[1]=this->current_L->height;
	bufferDims[2]=1;

	//We label connected components in this->current_J and copy to this->current_L
	//We eleminate regions with less than this->kmin pixels
	cvZero(this->current_J);
	Connexe_SetMinimumSizeOfComponents(this->kmin);
	temp = CountConnectedComponents(this->current_L->imageDataOrigin,UCHAR,
									this->current_J->imageDataOrigin,UCHAR,bufferDims);
	cvCopy(this->current_J,this->current_L);


	if(temp >=254) printf("Probable error \n");

	//We scan the border in K for labels and use them to create Connected components
	// we mark them with 255 , the maximum available
	img_ptr1 = (unsigned char *)this->current_K->imageDataOrigin;
	
	for(i=0;i<this->current_K->imageSize;i++)
	{
		if((*img_ptr1))
		{
			img_ptr2 = (unsigned char *)this->current_J->imageDataOrigin;
			temp = (unsigned char) this->current_J->imageDataOrigin[i];
			if(temp > 0)
			{
				for(j=0;j<this->current_J->imageSize;j++)
				{
					if(*img_ptr2 == temp) (*img_ptr2)=255; else (*img_ptr2)=0;
					img_ptr2++;
				}
			}
		}		
		img_ptr1++;
	}

	//Some labeled regions have remained so we must eliminate them. We exploit 
	//the fact that their labels are less than 255	
	cvThreshold(this->current_J,this->current_L,254,255,CV_THRESH_BINARY);
	
	return;
}

//Puts region separation in current_K
bool RegionLevel::updateWithFrameAndMarked(IplImage *raw,IplImage *marked)
{
	cvCopy(this->current_frame,this->previous_frame);

	cvCopy(raw,this->current_frame);

	printf("Updating J and K \n");
	this->updateJandK();
		
	printf("Backprojecting \n");

	//Current J can be reused , it is of no use anymore	
	cvZero(this->temp_storage);
	this->totalBackProjectIt();

	unsigned char *runner1,*runner2,*runner3;
	runner1= (unsigned char *) this->previous_frame->imageDataOrigin;
	runner2= (unsigned char *) this->current_K->imageDataOrigin;
	runner3= (unsigned char *) this->temp_storage->imageDataOrigin;

	for(int i=0;i<this->current_K->imageSize;i++)
	{
		if( (*runner2) )
		{
			(*runner3)=(*runner1);runner1++;runner3++;
			(*runner3)=(*runner1);runner1++;runner3++;
			(*runner3)=(*runner1);runner1++;runner3++;
		}
		else
		{
			(*runner3)=255;runner1++;runner3++;
			(*runner3)=255;runner1++;runner3++;
			(*runner3)=255;runner1++;runner3++;
		}
		runner2++;
	}

	cvCopy(marked,this->previous_separation);	
	return true;
}
