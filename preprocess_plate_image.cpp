#include "include/plate.h"

#if 0
static void get_image_max_min_value(IplImage * img, int * max, int * min);
static void gray_strecth(IplImage * src_img, IplImage* dst_img, int exp_max, int exp_min);
#endif

static void dilate_erode_x(IplImage * img_after_threshold, IplImage * img_final); 
static void dilate_erode_y(IplImage * img_final); 
/*功能:对车牌图像进行预处理,一遍后续的字符操作
 输入:尺度归一化后的车牌图像
 输出:预处理过后的车牌图像.bmp
 主要步骤:
 1.灰度化:
 2.灰度拉伸
 3.滤波
 4.边缘检测
 5.二值化
 6.膨胀腐蚀
 */

void preprocess_plate_image(IplImage * img_plate)
{
	if (img_plate == NULL) {
		fprintf(stderr, "img_plate is NULL.!!\n");
		exit(-1);
	}
	/*********************************准备工作**************************************************************/

	IplImage * img_gray = cvCreateImage(cvGetSize(img_plate), IPL_DEPTH_8U, 1);
	IplImage * img_after_stre = cvCreateImage(cvGetSize(img_plate), IPL_DEPTH_8U, 1);
	IplImage * img_after_sobel = cvCreateImage(cvGetSize(img_plate), IPL_DEPTH_8U, 1);
	IplImage * img_after_filter = cvCreateImage(cvGetSize(img_plate), IPL_DEPTH_8U, 1);
	IplImage * img_final = cvCreateImage(cvGetSize(img_plate), IPL_DEPTH_8U, 1);

#if 0
	cvNamedWindow("img_plate", 1);
	cvNamedWindow("img_after_stre", 1);
	cvNamedWindow("img_after_sobel", 1);
	cvNamedWindow("img_after_filter", 1);
	cvNamedWindow("img_final", 1);
#endif

	/******************************开始进行预处理******************************************************/
	/*与车辆整体预处理不同,这里直接开始进行二值化,而不是先进行边缘检测*/
	/*一:灰度化*/
	cvCvtColor(img_plate, img_gray, CV_RGB2GRAY);
	/*二:灰度拉伸*/
	cvNormalize(img_gray, img_after_stre, 0, 255, CV_MINMAX);
	/*三:滤波*/
	cvSmooth(img_after_stre, img_after_filter, CV_GAUSSIAN);
	/*四:二值化*/
	cvThreshold(img_after_filter, img_final, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
#if 0
	/*一:灰度化 cvCvtColor(IplImage* src, IplImage* dst, int color)*/
	cvCvtColor(img_plate, img_gray, CV_RGB2GRAY);
	/*二:灰度拉伸*/
	gray_strecth(img_gray, img_after_stre, 255, 0);
	/*三:滤波*/
	cvSmooth(img_after_stre, img_after_filter, CV_GAUSSIAN);
	/*四:边缘检测*/
	cvCanny(img_after_filter, img_after_sobel, 50, 150, 3);
	/*五:二值化*/
	cvThreshold(img_after_sobel, img_final, 0, 255, CV_THRESH_BINARY| CV_THRESH_OTSU);
	/*六:膨胀腐蚀*/
	dilate_erode_x(img_final, img_final);
	dilate_erode_y(img_final);
#endif

#if 0
	/*一:灰度化 cvCvtColor(IplImage* src, IplImage* dst, int color)*/
	cvCvtColor(img_plate, img_gray, CV_RGB2GRAY);

	/*二:灰度拉伸*/
	cvNormalize(img_gray, img_after_stre, 0, 255, CV_MINMAX);

	/*三:滤波*/
	cvSmooth(img_after_stre, img_after_filter, CV_GAUSSIAN);
//	img_after_filter = cvCloneImage(img_after_stre);

	/*五:二值化*/
	cvThreshold(img_gray, img_final, 0, 255, CV_THRESH_BINARY| CV_THRESH_OTSU);

	/*四:边缘检测*/
	cvCanny(img_after_filter, img_after_sobel, 0, 50, 3);


	/*六:膨胀腐蚀*/
	dilate_erode_x(img_final, img_final);
	dilate_erode_y(img_final);

#endif


	/*********************************显示图像******************************************************************/
#if 0
	cvShowImage("img_plate", img_plate);
	cvShowImage("img_gray", img_gray);
	cvShowImage("img_after_stre", img_after_stre);
	cvShowImage("img_after_sobel", img_after_sobel);
	cvShowImage("img_after_filter", img_after_filter);
	cvShowImage("img_final", img_final);

	cvWaitKey(0);
#endif
	/*********************************最终需要返回img_final图像到dst_img中****************************************/
	IplImage * dst_img;
	dst_img = cvCloneImage(img_final);
	cvSaveImage("image/img_plate_after_preprocess.bmp", dst_img);
	/*********************************释放内存****************************************************************/
	cvReleaseImage(&img_gray);
	cvReleaseImage(&img_after_stre);
	cvReleaseImage(&img_after_sobel);
	cvReleaseImage(&img_after_filter);
	cvReleaseImage(&img_final);

}

#if 0
/*用不着了*/
/*灰度拉伸函数
 参数:源图像,目标图像,最大值,最小值*/
void gray_strecth(IplImage * src_img, IplImage* dst_img, int exp_max, int exp_min)
{

	if (src_img == NULL) {
		fprintf(stderr, "src_img is NULL.\n");
		exit(-1);
	}

	if (exp_max > 256 || exp_min < 0) {
		fprintf(stderr, "Make sure exp_max below 256 and exp_min above 0.\n");
		exit(-1);
	}
	int ori_max, ori_min;

	get_image_max_min_value(src_img, &ori_max, &ori_min);

	for (int i = 0; i < src_img->height; i++) {
		unsigned char * srow = (unsigned char *)(src_img->imageData + i * src_img->widthStep);
		unsigned char * drow = (unsigned char *)(dst_img->imageData + i * dst_img->widthStep);
		for (int j = 0; j < src_img->width; j++){
			if (srow[j] < ori_min)
			  drow[j] = (exp_min / ori_min) * srow[j];
			else if (srow[j] >= ori_min && srow[j] <= ori_max)
			  drow[j] = (exp_max - exp_min) / (ori_max - ori_min) * (srow[j] - ori_min) + exp_min;
			else if (srow[j] > exp_max)
			  drow[j] = (255 - exp_max) / (255 - ori_max) * (srow[j] - ori_max) + exp_max;
		}
	
	}
}


void get_image_max_min_value(IplImage * img, int * max, int * min)
{
	int max_temp = -255, min_temp = 400;

	for (int i = 0; i < img->height; i ++) {

		unsigned char * prow = (unsigned char *)(img->imageData + i * img->widthStep);

		for (int j; j < img->width; j ++) {

			if (prow[j] >= max_temp)	
			  max_temp = prow[j];

			if (prow[j] <= min_temp)
			  min_temp = prow[j];
		}
	}

	*max = max_temp;
	*min = min_temp;
}

/*自定义膨胀腐蚀操作,在循环中使用可以增加定位准确度*/
void dilate_erode_x(IplImage * img_after_threshold, IplImage * img_final) {
	/*自定义1*3的核进行X方向的膨胀腐蚀*/
	IplConvKernel* kernal = cvCreateStructuringElementEx(3,1, 1, 0, CV_SHAPE_RECT);
	cvDilate(img_after_threshold, img_final, kernal, 1);/*X方向膨胀连通数字*/
	cvErode(img_final, img_final, kernal, 1);/*X方向腐蚀去除碎片*/
	cvDilate(img_final, img_final, kernal, 1);/*X方向膨胀回复形态*/
	cvErode(img_final, img_final, kernal, 1);/*X方向腐蚀去除碎片*/
	cvDilate(img_final, img_final, kernal, 1);/*X方向膨胀回复形态*/
}

void dilate_erode_y(IplImage * img_final) {
	/*自定义3*1的核进行Y方向的膨胀腐蚀*/
	IplConvKernel* kernal = cvCreateStructuringElementEx(1, 3, 0, 1, CV_SHAPE_RECT);
	cvDilate(img_final, img_final, kernal, 1);/*回复形态*/
	cvErode(img_final, img_final, kernal, 1);/*Y方向腐蚀去除碎片*/
	cvDilate(img_final, img_final, kernal, 1);/*回复形态*/
	cvErode(img_final, img_final, kernal, 1);/*Y方向腐蚀去除碎片*/
	cvDilate(img_final, img_final, kernal, 1);/*回复形态*/
}


#endif
