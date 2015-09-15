#include "confParameters.h"

confParameters::confParameters()
{
	//threshold for filter in pixel level
	//difference between prediction/actual frame
	diff_threshold = 20;
	//kmotion
	succ_threshold = 10;
	//number of bins
	bin_num =8;
	//epsilon
	bin_thres = 10;
	//kmin
	region_area=30;
	//number of past frames fo filtering
	past_length=5;
	//delay to reach actual content in video
	frame_delay=200;
	//movie name
	movie_name="c:\\museum.avi";
	//cascades
	csd_name="haarcascade_frontalface_default.xml";
	//flag or face detection
	face_flag=false;
	//flag for body detection
	body_flag=false;

}
