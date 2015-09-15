#define FrameWienerPredictor_h true
#include "cxcore.h"
#include  "cv.h" 
#include "confParameters.h"
class FrameWienerPredictor
{
	public :

	IplImage **past_frames;
	IplImage **coefficients;
	IplImage *prediction;
	IplImage *foreground;
	IplImage *temp_storage;
	float some_threshold;	
	int size;
	int ready;
	FrameWienerPredictor(confParameters* params);
	~FrameWienerPredictor();
	void updateWithFrame(IplImage* some_frame);
	bool isReady();
	IplConvKernel* struct_elem;
	private:
	
	void computeForeground(IplImage* some_frame);
	void computePrediction();
};
