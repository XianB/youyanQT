#include "include/plate.h"

#if 0
/*不再需要这两个函数了*/
static void gray_strecth(IplImage * src_img, IplImage* dst_img, int exp_max, int exp_min);
static void get_image_max_min_value(IplImage * img, int * max, int * min);
#endif

/*功能:对车辆整体图像进行预处理
 输入:车辆原始图像
 输出:预处理完成后的图像
 过程:
	1.灰度化
	2.灰度拉伸
	3.滤波
	4.边缘检测
	5.二值化
	6.腐蚀膨胀 //这一步可以移动到车牌定位中去,要用循环进行筛选
 作者:唐显斌
 版本:v1.2
 */

void preprocess_car_img(IplImage * img_car)
{
	if (img_car == NULL) {
		fprintf(stderr, "img_car is NULL.\n");
		exit(-1);
	}
	/*********************************准备工作**************************************************************/
	IplImage * img_gray = cvCreateImage(cvGetSize(img_car), IPL_DEPTH_8U, 1);
	IplImage * img_after_stre = cvCreateImage(cvGetSize(img_car), IPL_DEPTH_8U, 1);
	IplImage * img_after_sobel = cvCreateImage(cvGetSize(img_car), IPL_DEPTH_8U, 1);
	IplImage * img_after_filter = cvCreateImage(cvGetSize(img_car), IPL_DEPTH_8U, 1);
	IplImage * img_final = cvCreateImage(cvGetSize(img_car), IPL_DEPTH_8U, 1);

#if 0
	cvNamedWindow("img_car", 1);
	cvNamedWindow("img_after_stre", 1);
	cvNamedWindow("img_after_sobel", 1);
	cvNamedWindow("img_after_filter", 1);
	cvNamedWindow("img_final", 1);
#endif

	/******************************开始进行预处理******************************************************/
	/*可改进程度不大*/
	
	/*一:灰度化 cvCvtColor(IplImage* src, IplImage* dst, int color)*/
	cvCvtColor(img_car, img_gray, CV_RGB2GRAY);
    cvSaveImage("image/img_gray.bmp", img_gray);
	/*二:灰度拉伸*/
	cvNormalize(img_gray, img_after_stre, 0, 255, CV_MINMAX);
    cvSaveImage("image/img_after_stre.bmp", img_after_stre);
	/*gray_strecth(img_gray, img_after_stre, 255, 0);  有自带的函数比这个好用*/
	/*三:滤波*/
	cvSmooth(img_after_stre, img_after_filter, CV_GAUSSIAN);
    cvSaveImage("image/img_after_filter.bmp", img_after_filter);
	/*四:边缘检测*/
	//cvCanny(img_after_filter, img_after_sobel, 50, 150, 3);	/*比用sobel函数好,因为这个不会分方向,把所有的边缘都画了出来了*/
	cvSobel(img_after_filter, img_after_sobel, 1, 0, 3);
    cvSaveImage("image/img_after_sobel.bmp", img_after_sobel);
	/*五:二值化*/
	cvThreshold(img_after_sobel, img_final, 0, 255, CV_THRESH_BINARY| CV_THRESH_OTSU);


	/*********************************显示图像******************************************************************/
#if 0
	cvShowImage("img_car", img_car);
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
	cvSaveImage("image/img_after_preprocess.bmp", dst_img);
	/*********************************释放内存****************************************************************/
	cvReleaseImage(&img_gray);
	cvReleaseImage(&img_after_stre);
	cvReleaseImage(&img_after_sobel);
	cvReleaseImage(&img_after_filter);
	cvReleaseImage(&img_final);

}

#if 0
/*不再需要这两个函数了*/
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

		for (int j = 0; j < img->width; j ++) {

			if (prow[j] >= max_temp)	
			  max_temp = prow[j];

			if (prow[j] <= min_temp)
			  min_temp = prow[j];
		}
	}

	*max = max_temp;
	*min = min_temp;
}

#endif
