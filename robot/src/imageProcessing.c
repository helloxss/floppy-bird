#include "imageProcessing.h"


/*!
* \brief Transform the image into a two colored image, one color for the color we want to track, another color for the others colors
* From this image, we 
* \param[in] obj : address of the TrackedObject 
*/
void binarisation(TrackedObject* obj) 
{ 
    IplImage *hsv;
    IplConvKernel *kernel;
    
	if(obj->binFlux->img != NULL)
		releaseTrackingImageMemory(obj);
    obj->binFlux->img = cvCreateImage(cvGetSize(obj->rawFlux->img), obj->rawFlux->img->depth, 1);	/* Create the mask &initialize it to white (no color detected) */
	
    /* Create the hsv image */
    hsv = cvCloneImage(obj->rawFlux->img);
    cvCvtColor(obj->rawFlux->img, hsv, CV_BGR2HSV);
	
    /* We create the mask */
    cvInRangeS(hsv, cvScalar(obj->h - Htolerance -1, obj->s - Stolerance, 0,0), cvScalar(obj->h + Htolerance -1, obj->s + Stolerance, 255,0), obj->binFlux->img);
	
    /* Create kernels for the morphological operation */
    kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_RECT,NULL);
 
    /* Morphological opening (inverse because we have white pixels on black background) */
    cvDilate(obj->binFlux->img, obj->binFlux->img, kernel, 2);
    cvErode(obj->binFlux->img, obj->binFlux->img, kernel, 2);  
    
    /* Release memory */
    cvReleaseStructuringElement(&kernel);
    cvReleaseImage(&hsv);
}

/*!
* \brief get the number of pixel detected and the center of gravity of these pixel + display the tracking zone eventualy
* From this image, we get two datas : the number of pixel detected, and the center of gravity of these pixel
* \param[in] obj : address of the TrackedObject 
* \param[in] displayTrackZone : true if you want to display the trakingZone
*/
void findObject(TrackedObject * obj, boolean displayTrackZone)
{
	int x, y;
	int sumX = 0, sumY = 0;
    obj->nbPixels = 0;
	cvSetImageROI(obj->binFlux->img,obj->trackingZone);
	
    /* We go through the mask to look for the tracked object and get its gravity center */
    for(x = obj->binFlux->img->roi->xOffset; x < obj->binFlux->img->roi->width + obj->binFlux->img->roi->xOffset; x++) {
        for(y = obj->binFlux->img->roi->yOffset; y < obj->binFlux->img->roi->height + obj->binFlux->img->roi->yOffset ; y++) { 
 
            /* If its a tracked pixel, count it to the center of gravity's calcul */
            if((obj->binFlux->img->imageData[x+ y*obj->binFlux->img->widthStep]) == 255) {
                sumX += x;
                sumY += y;
                (obj->nbPixels)++;
            }
        }
    }
    
	/* Show the tracking zone in the full colored image*/
	if(displayTrackZone)
		cvRectangleR(obj->rawFlux->img,obj->trackingZone,obj->trackerColor,1,8,0);
 
    // If there is no pixel, we return a center outside the image, else we return the center of gravity
    if(obj->nbPixels > 0){
		obj->origin = cvPoint((int)(sumX / (obj->nbPixels)), (int)(sumY / (obj->nbPixels)));
        /* Getting the width of the tracked object */
        sumX = 0;
        y = obj->origin.y;
        for(x = obj->binFlux->img->roi->xOffset; x < obj->binFlux->img->roi->width + obj->binFlux->img->roi->xOffset; x++){
			if((obj->binFlux->img->imageData[x+ y*obj->binFlux->img->widthStep]) == 255) {
                sumX ++;
            }
		}
		obj->width = sumX;
        /* Getting the height of the tracked object */
        sumY = 0;
        x = obj->origin.x;
        for(y = obj->binFlux->img->roi->yOffset; y < obj->binFlux->img->roi->height + obj->binFlux->img->roi->yOffset ; y++){
			if((obj->binFlux->img->imageData[x+ y*obj->binFlux->img->widthStep]) == 255) {
                sumY ++;
            }
		}
		obj->height = sumY;
        
	} else {
        obj->origin = cvPoint(-1, -1);
	}
	
	cvResetImageROI(obj->binFlux->img);
}


/*!
* \brief Add a shape on the video that follow your colored object
* \param[in] obj : address of the TrackedObject 
*/
void addObjectToVideo(TrackedObject* obj) {
	
    /* Draw an object centered on its origin */
    if (obj->nbPixels > NB_PIXEL_THRESHOLD){
		switch (obj->shape){
			case ELLIPSE:
				cvDrawEllipse(obj->rawFlux->img, obj->origin, cvSize(obj->height/2,obj->width/2), 0, 0, 360, reverse(obj->trackerColor), 1,8,0);	/* Draw a circle around the origin */
				break;
			case RECTANGLE: ;	/*empty statement needed because of the following declaration*/
				CvPoint leftUpCorner = cvPoint(obj->origin.x-(obj->width/2),obj->origin.y-(obj->height/2));
				CvPoint rightDownCorner = cvPoint(obj->origin.x+(obj->width/2),obj->origin.y+(obj->height/2));
				cvRectangle(obj->rawFlux->img,leftUpCorner,rightDownCorner, reverse(obj->trackerColor), 1,8,0);	/* Draw a rectangle frow the origin */
				break;
			case NONE:
				break;
		}
	}
}

/*!
* \brief start the init work space procedure
* \param[in] capture : (optional if tere loading from a file) address of the capture
* \param[in] window : name of the window
* \param[in] loadFile : (optional) the load file
*/
CvRect initWorkSpace(RaspiCamCvCapture * capture, char* window, FILE* loadFile){
	if(loadFile==NULL){
		struct VolatileRect workingSpace;
		struct ImageBroadcast flux;
		
		workingSpace.originDefined = false;
		workingSpace.rectDefined = false;
		initImageBroadcast(&flux, NULL, NULL, window, NULL);
		
		cvSetMouseCallback(window, getCurrentPointCoordinates, &workingSpace);
		printf("Definition of the working space \n");
		while(workingSpace.rectDefined == false) {			/* wait for the definition of the workspace */
			loadImage(&flux,capture);
			if(workingSpace.originDefined) {
				cvRectangleR(flux.img,workingSpace.rect,WORKSPACE_DEF_COLOR,1,8,0);
			}
			showImage(&flux);		
			char keyPressed = cvWaitKey(30);
			switch (keyPressed){
				case 27:				/* ESC to reset the rectangle origin */
					workingSpace.originDefined = false;
					break;
			}
		}
		printf("Working space defined\n");
		cvDestroyWindow(window);
		return workingSpace.rect;
	} else {		/* else loading file */
		CvRect workSpaceRect;
		fread(&workSpaceRect,sizeof(CvRect),1,loadFile);
		return workSpaceRect;
	}
}

/*!
* \brief save the work space in a file
* \param[in] workingSpace : the working space
* \param[in] saveFile : the save file
*/
void saveWorkingSpace(CvRect* workingSpace, FILE* saveFile)
{
	if(saveFile != NULL && workingSpace!=NULL)
		fwrite(workingSpace,sizeof(CvRect),1,saveFile);
}

/*!
* \brief reverse a color
* \param[in] color : a CvScalar of the color to reverse
*/
CvScalar reverse(CvScalar color)
{
	return cvScalar(255-color.val[0],255-color.val[1],255-color.val[2],0);
}
