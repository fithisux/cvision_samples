#include "bgUtilities.h"
#include <stdio.h>

void createCircle(IplImage *frame,CvPoint *center_ptr,double radius)
{						
		CvPoint pp;
		float dist;
		int l;
		unsigned char * runner=(unsigned char *) frame->imageDataOrigin;

		cvSet(frame,cvScalar(255,255,255,255));
		for(pp.x=0,pp.y=0,l=0;l<(frame->imageSize/3);l++)
		{				
				dist = sqrtf(	(1.0*pp.x - 1.0*center_ptr->x)*
								(1.0*pp.x - 1.0*center_ptr->x)+ 
								(1.0*pp.y - 1.0*center_ptr->y)*
								(1.0*pp.y - 1.0*center_ptr->y));
				if(dist <= radius)
				{
					(*runner)=0;
					(*(runner+1))=255;
					(*(runner+2))=0;				
				}
				runner+=3;		
				pp.x = l % frame->width;
				pp.y = l / frame->width;
		}
		
}

void deInterlace(IplImage *frame)
{

	unsigned char * runner=(unsigned char *) frame->imageData;
	
	for(int l=0;l<frame->imageSize;l++)
	{					
			if( (l / frame->widthStep) % 2 == 1)
			{
				(*runner) = (*(runner-frame->widthStep));
			}
			runner++;
	}
}

unsigned char uabsdiff(unsigned char a,unsigned char b)
{
	if(a>b) return (unsigned char) a-b;
	else return (unsigned char) b-a;
}

int rgb_diff(unsigned char *a,unsigned char *b)
{
	int d=0;

	if(*a> *b) d+=((*a)-(*b)); else d+=((*b)-(*a));
	a++;b++;
	if(*a> *b) d+=((*a)-(*b)); else d+=((*b)-(*a));
	a++;b++;
	if(*a> *b) d+=((*a)-(*b)); else d+=((*b)-(*a));
	
	return d;
}

void cleverDeInterlace(IplImage *frame)
{
	unsigned char * runner;
	int P,Q,theta;
	unsigned char *  C = new unsigned char[3];
	int eff_width=frame->widthStep-6 ;
	int eff_height = ( (frame->height /2 == 0) ? frame->height-2 : frame->height);
	int indy=frame->widthStep;
	int line_step=frame->widthStep;

	for(int i=1;i< eff_height;i+=2)
	{		
		runner = (unsigned char *) &frame->imageData[indy+3];
		indy+=(2 * line_step);

		for(int j=0;j< eff_width;j+=3)
		{			
			P = rgb_diff(runner-line_step,runner+line_step+3);
			P+= rgb_diff(runner-line_step-3,runner+line_step);

			Q = rgb_diff(runner-line_step+3,runner+line_step);
			Q+= rgb_diff(runner-line_step,runner+line_step-3);

			C[0]= rgb_diff(runner-line_step+3,runner+line_step-3);			
			C[1]= rgb_diff(runner-line_step,runner+line_step);				
			C[2]= rgb_diff(runner-line_step-3,runner+line_step+3);
				

			if( P > Q) theta = ( ( C[0] < C[1] ) ? -1 : 0);
			else if (Q < P) theta = ( ( C[1] < C[2] ) ? 0 : 1);
			else
			{
				theta = ( ( C[0] < C[1] ) ? -1 : 0);				
				if( C[2] < C[theta+1] ) theta=1;
			}

			for(int k=0;k<3;k++)
			{
				(*runner)=(*(runner-line_step-3*theta))+(*(runner+line_step+3*theta));
				(*runner) /=2;			
				runner++;
			}			
		}
	}
	delete C;
}

/*returns a rectangle containing the non-zero pixels of the image*/ 
CvRect GetNonZeroRect(IplImage* in)
{
 int imin=10000000, jmin=10000000, imax=-1, jmax=-1;
 CvRect rect;
 
 for (int i = 0; i < in->height; i++ )
  for (int j = 0; j < in->width; j++ )
  {
   if( ((unsigned char *)((in->imageData + in->widthStep*i)))[j] )
   {
    if (i<imin) imin=i;
    if (j<jmin) jmin=j;
    if (i>imax) imax=i;
    if (j>jmax) jmax=j;
   }
  }
 
 rect.x = jmin;
 rect.y = imin;
 rect.height  = imax - imin;
 rect.width   = jmax - jmin;
 
 return rect;
}

bool convertByte2Float(IplImage *byte_img,IplImage *float_img)
{
	if(4*byte_img->imageSize != float_img->imageSize) return false;

	unsigned char * runner1= (unsigned char *)byte_img->imageDataOrigin;        	        	
	float * runner2 = (float *) float_img->imageDataOrigin;
			
	for(int k=0;k< byte_img->imageSize;k++)
	{
		(*runner2) = (float) (*runner1);
		//printf(">> %f \n",(*runner2));				
		runner1++;runner2++;
	}  
	return true;      	        
}

bool convertFloat2Byte(IplImage *float_img,IplImage *byte_img)
{

	if(4*byte_img->imageSize != float_img->imageSize) return false;

	unsigned char * runner1= (unsigned char *)byte_img->imageDataOrigin;        	        	
	float * runner2 = (float *) float_img->imageDataOrigin;
			
	for(int k=0;k< byte_img->imageSize;k++)
	{
		(*runner1) = (unsigned char) (*runner2);
		//printf(">> %f \n",(*runner2));				
		runner1++;runner2++;
	}  
	return true;      	        
}


imageShower::imageShower(char * wnd_name)
{
	cvNamedWindow(wnd_name,1);
	this->the_name=wnd_name;
	this->animage=NULL;
}

imageShower::~imageShower()
{
		if(this->animage != NULL)	cvReleaseImage(&this->animage);
		cvDestroyWindow(this->the_name);
}

void imageShower::myShowImage(IplImage * frame)
{	
	if(frame == NULL) return;

	if(this->animage == NULL)
	{
		if(frame->depth == IPL_DEPTH_32F)
		{
			this->animage = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,frame->nChannels);
			this->animage->origin=frame->origin;
			convertFloat2Byte(frame,this->animage);
		}
		else this->animage = cvCloneImage(frame);	
	} 
	else
	{
		if(this->animage->imageSize != frame->imageSize)
		{
			cvReleaseImage(&this->animage);
			if(frame->depth == IPL_DEPTH_32F)
			{
				this->animage = 
				cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,frame->nChannels);
				this->animage->origin=frame->origin;
				convertFloat2Byte(frame,this->animage);
			}
			else this->animage = cvCloneImage(frame);
		}
		else
		{
			if(this->animage->nChannels != frame->nChannels)
			{
				cvReleaseImage(&this->animage);
				if(frame->depth == IPL_DEPTH_32F)
				{
					this->animage = 
							cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,frame->nChannels);
					this->animage->origin=frame->origin;
					convertFloat2Byte(frame,this->animage);
				}
				else this->animage = cvCloneImage(frame);				
			}
			else
			{
				if(frame->depth == IPL_DEPTH_32F) convertFloat2Byte(frame,this->animage);
				else cvCopy(frame,this->animage);
				this->animage->origin=frame->origin;
			}
		}
	}
	cvShowImage(this->the_name,this->animage);
}

CvHaarClassifierCascade* load_object_detector( const char* cascade_path )
{
    return (CvHaarClassifierCascade*)cvLoad( cascade_path );
}

void detect_and_draw_objects( IplImage* image,
                              CvHaarClassifierCascade* cascade,
                              int do_pyramids )
{
    IplImage* small_image = image;
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* faces;
    int i, scale = 1;

    /* if the flag is specified, down-scale the input image to get a
       performance boost w/o loosing quality (perhaps) */
    if( do_pyramids )
    {
        small_image = cvCreateImage( cvSize(image->width/2,image->height/2), IPL_DEPTH_8U, 3 );
        cvPyrDown( image, small_image, CV_GAUSSIAN_5x5 );
        scale = 2;
    }

    /* use the fastest variant */
    faces = cvHaarDetectObjects( small_image, cascade, storage, 1.2, 2, CV_HAAR_DO_CANNY_PRUNING );

	if (faces->total > 0) 
		printf("found %d faces \n", faces->total);
	else
		printf("no faces detected \n");

    /* draw all the rectangles */
    for( i = 0; i < faces->total; i++ )
    {
        /* extract the rectanlges only */
        CvRect face_rect = *(CvRect*)cvGetSeqElem( faces, i);
        cvRectangle( image, cvPoint(face_rect.x*scale,face_rect.y*scale),
                     cvPoint((face_rect.x+face_rect.width)*scale,
                             (face_rect.y+face_rect.height)*scale),
                     CV_RGB(255,0,0), 3 );
    }

    if( small_image != image )
        cvReleaseImage( &small_image );
    cvReleaseMemStorage( &storage );
}