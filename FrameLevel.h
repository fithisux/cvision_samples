#include "KMeans.h"

class FrameLevel
{
	public :
	
	IplImage **cores;
	int the_size;
	int representative_model;
	float switch_threshold;
	float comparison_threshold;
	IplImage *temp_storage;	
	public :
	
	FrameLevel(IplImage **some_cores,int length,int prefered,float sw_thres,float comp_thres);
	bool switchWithFrame(IplImage* frame,IplImage *initial_backextr);
	void recreateMask(IplImage* frame,IplImage *initial_backextr);
	~FrameLevel();

	private :
	
	float findRatio(int core_no,IplImage* frame,IplImage *initial_backextr);
};
