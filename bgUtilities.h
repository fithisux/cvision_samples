#include "cxcore.h"
#include "cv.h"
#include "highgui.h"

void createCircle(IplImage *frame,CvPoint *center_ptr,double radius);
void deInterlace(IplImage *frame);
bool convertByte2Float(IplImage *byte_img,IplImage *float_img);
bool convertFloat2Byte(IplImage *float_img,IplImage *byte_img);
CvHaarClassifierCascade* load_object_detector(const char* cascade_path);
void detect_and_draw_objects( IplImage* image,CvHaarClassifierCascade* cascade,
                              int do_pyramids);
void cleverDeInterlace(IplImage *frame);
unsigned char uabsdiff(unsigned char a,unsigned char b);
int rgb_diff(unsigned char *a,unsigned char *b);
CvRect GetNonZeroRect(IplImage* in);

class imageShower
{
	private :
	char *the_name;
	IplImage* animage;

	public :
	imageShower(char * wnd_name);
	~imageShower();
	void myShowImage(IplImage *frame);
};