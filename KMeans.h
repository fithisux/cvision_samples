#include "FrameWienerPredictor.h"

class KMeans
{
		
	public :
	
	IplImage** cores;
	int size;
	int* counter;
	public :

	KMeans(int size);
	~KMeans();
	void updateCore(IplImage *frame);
	int findClass(IplImage *frame);
	bool isReady();
};
