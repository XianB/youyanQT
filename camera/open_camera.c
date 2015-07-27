#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	CvCapture *capture;
	
	IplImage * frame;
	char filename[50];
	cvNamedWindow("camera", 1);
	int i = 0;

	capture = cvCreateCameraCapture(0);

	if (capture == NULL) {
		fprintf(stderr, "Cannot open camera!\n");
		exit(-1);
	}

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 1920); 
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 1080); 
	while(1) {
		frame = cvQueryFrame(capture);
		if (!frame)
			break;
		cvShowImage("camera", frame);
		char c = cvWaitKey(33);
		if (c == '\n') {
			break;
		}
		if (c == ' ') {
			sprintf(filename, "img%d.png", i);
			cvSaveImage(filename, frame);
			i++;
		}

	}

	printf("Done!\n");
	return 0;
}
