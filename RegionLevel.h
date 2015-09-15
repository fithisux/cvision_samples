#include "cxcore.h"
#include  "cv.h" 
#include <stdio.h>
#include "confParameters.h"

class RegionLevel
{
	public :
	
	IplImage *temp_storage,*previous_frame,*current_frame;
	IplImage *previous_separation;
	IplImage *previous_J,*current_J;
	IplImage *current_K,*current_L;
	CvHistogram* hist;
	int kmotion,kmin;
	float myepsilon;
	int hist_sizes[3];
	float *hist_ranges[3];
	IplImage * image_array[3];		    
	IplConvKernel* struct_elem;                               

	public :
	
	RegionLevel(IplImage *raw,IplImage *marked,confParameters *params);				
	~RegionLevel();
	bool updateWithFrameAndMarked(IplImage *raw,IplImage *marked);
	void updateJandK();	
	void totalBackProjectIt();
};
	