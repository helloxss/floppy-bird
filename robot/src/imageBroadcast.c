#include "imageBroadcast.h"


void initImageBroadcast(ImageBroadcast* flux, IplImage* img, char* windowTitle, CvFont * font)
{
	flux->img = img;
	flux->windowTitle = windowTitle;
	flux->font = font;
	cvNamedWindow(windowTitle, CV_WINDOW_AUTOSIZE);
}

void initFont(ImageBroadcast* flux){
	double hScale=0.4;
	double vScale=0.4;
	int    lineWidth=1;
	
	cvInitFont(flux->font, CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale, vScale, 0, lineWidth, 8);
}

void updateImage(ImageBroadcast* flux,IplImage* img)
{
	flux->img = img;
}

void updateImageFromCapture(ImageBroadcast* flux, RaspiCamCvCapture * capture)
{
	flux->img = raspiCamCvQueryFrame(capture);
}

void showImage(ImageBroadcast* flux)
{
	cvShowImage(flux->windowTitle,flux->img);
}

/*
 * Get the color of the pixel where the mouse has clicked
 * We put this color as model color (the color we want to tracked)
 */
void getObjectColor(int event, int x, int y, int flags, void *param) {
 
    struct TrackedObject* obj = (struct TrackedObject*)param;	/* tracked object is passed through param */
    CvScalar pixel;
    IplImage *hsv;
 
    if(event == CV_EVENT_LBUTTONUP) {
 
        /* Get the hsv image */
        hsv = cvCloneImage(obj->rawFlux->img);
        cvCvtColor(obj->rawFlux->img), hsv, CV_BGR2HSV);
 
        /* Get the selected pixel */
        pixel = cvGet2D(hsv, y, x);
 
        /* Change the value of the tracked color with the color of the selected pixel */
        obj->h = (int)pixel.val[0];
        obj->s = (int)pixel.val[1];
        obj->v = (int)pixel.val[2];
 
        /* Release the memory of the hsv image */
        cvReleaseImage(&hsv);
 
    }
}

void getCurrentPointCoordinates(int event, int x, int y, int flags, void *param){
	struct VolatileRect * workingSpace = (struct VolatileRect *)param;
	if(workingSpace->originDefined == 1 && event == CV_EVENT_MOUSEMOVE){
		CvPoint origin = cvPoint(workingSpace->rect.x,workingSpace->rect.y);
		workingSpace->rect = cvRect(min(x,origin.x),min(y,origin.y),abs(x-origin.x),abs(y-origin.y));
	}
	
	if(event == CV_EVENT_LBUTTONUP){
		printf("click at x=%d \ty=%d\n",x,y);
		if(workingSpace->originDefined){
			CvPoint origin = cvPoint(workingSpace->rect.x,workingSpace->rect.y);
			workingSpace->rect = cvRect(min(x,origin.x),min(y,origin.y),abs(x-origin.x),abs(y-origin.y));
			workingSpace->rectDefined = 1;
		} else {
			workingSpace->rect.x = x;
			workingSpace->rect.y = y;
			workingSpace->originDefined = 1;
		}
	}
}