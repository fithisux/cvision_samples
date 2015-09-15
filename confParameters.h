#ifndef confParameters_h

#define confParameters_h


class confParameters
{

public :

	float diff_threshold;
	float succ_threshold;
	int bin_num;
	int bin_thres;
	int frame_delay;
	int region_area;
	int past_length;
	const char *movie_name;
	const char *csd_name;
	bool face_flag;
	bool body_flag;
	confParameters();

};

#endif