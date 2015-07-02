#include "include/plate.h"

#define _DEBUG_

#ifdef _DEBUG_
static void draw_rect(IplImage * img, CvRect rect);
#endif
static void dilate_erode_x(IplImage * img_after_threshold, IplImage * img_final); 
static void dilate_erode_y(IplImage * img_final);
static void filter_rect(List src_rects, List dst_rects, IplImage * org_img_car);
static bool identity_by_color(IplImage * img, CvRect rects);
static bool histogram_analysing(CvHistogram *hist, int bins);
/*
输入:预处理完成后的车辆图像
输出:唯一的车牌定位的位置
作者:唐显斌
版本:v1.2
基本思路:
1.找到膨胀腐蚀后的所有外接矩形轮廓
2.根据矩形轮廓的形状比例进行筛选
3.根据颜色以及形状比例进行筛选
4.用循环控制膨胀腐蚀的次数,找到最优矩形即为车牌位置
 */

List get_location(IplImage * img_car, IplImage * org_img_car)
{
	/*分别为用来装预处理后的图像中所有矩形轮廓和筛选过后的轮廓的链表*/
	List rects = create_list();
	List rects_final = create_list();
	assert(rects_final->next == NULL);
//	cvNamedWindow("tmp_image", 1);

	IplImage * img_after_preprocess = cvLoadImage("image/img_after_preprocess.bmp", -1);
	if (img_after_preprocess == NULL) {
		fprintf(stderr, "Can not load image -- img_after_preprocess.bmp!\n");
		exit(-1);
	}

	/*在检测轮廓时需要用到的两个量,storage为容器,contours为指向轮廓的指针*/
	CvMemStorage * storage = cvCreateMemStorage(0);
	CvSeq * contours = NULL;

#if 1
	/*7.17.11.26加上
	 目的:改进车牌定位准确度
	 争取一次定位成功,并且只定位一个位置,不要预选位置
	 */

	dilate_erode_y(img_after_preprocess);
	//rects_final中最后只会保存一个矩形位置的
	while (rects_final->next == NULL) {
		/*主要问题是,如何在第一次就确定那个车牌的位置?*/
		static int count = 0;
		count++;
		assert(count < 20);
		/*重点改进之处,膨胀和腐蚀的次数是很关键的*/
		dilate_erode_x(img_after_preprocess, img_after_preprocess);

		/*找到所有的矩形轮廓*/
		get_contour_rect(img_after_preprocess, rects, storage, contours);/*没什么可以改进的地方*/
		assert(rects->next != NULL);

		/*按照形状进行矩形筛选*/
		/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

		/*重点改进之处,如何筛选出目标矩形是重点,如何保证在经过筛选之后保留的只是一个候选位置是关键,筛选完的结果只有两种,一是空,什么都没有找到,另一种是找到几个候选位置,但是通过颜色判断之后发现肯定不是车牌的位置,
			也就是说如果目前还没有得到候选位置只有可能是这样,在filter中找到的所有矩形颜色都不是蓝色的,也就是说车牌还没膨胀出来,而不会出现有一个位置,但这个位置不是车牌位置的情况
		 */

		draw_contour_rect(img_after_preprocess, rects, "image/img_car_with_rect.bmp");			/*显示未筛选前的矩形位置,看看到底有没有把车牌的位置找到*/
		filter_rect(rects->next, rects_final, org_img_car);				

#if 1
		/*用来逐步显示如果一次没找到候选位置时的操作*/
//		cvShowImage("tmp_img", img_after_preprocess);
//		cvWaitKey(0);
#endif

		/*如果不为一个矩形就继续循环,膨胀腐蚀*/
			/*这一个要满足什么条件呢?*/

		if (count_node(rects_final) == 0) {
			empty_list(rects_final);		/*要保证每次重新循环的时候rects_final都是空的才行*/
			continue;
		}

		/*能执行到这一步说明这肯定是只有一个矩形的*/

		/*注意rects有头结点,所以传进去的时候忽略掉头结点*/
	}//end while
		return rects_final->next;
#endif

#if 0
	/*7.16.11.26注释掉:
	 目的:改进车牌定位准确度
	 当前状态:能正常运行,并且定位准确度在测试中*/
	/*先进行膨胀腐蚀*/
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*重点改进之处,膨胀和腐蚀的次数是很关键的*/
	dilate_erode_x(img_after_preprocess, img_after_preprocess);
	dilate_erode_y(img_after_preprocess);

	/*找到所有的矩形轮廓*/
	get_contour_rect(img_after_preprocess, rects, storage, contours);/*没什么可以改进的地方*/
	assert( rects->next != NULL);

	/*按照形状进行矩形筛选*/
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*重点改进之处,如何筛选出目标矩形是重点*/
	filter_rect_by_shape(rects->next, rects_final);				
	assert(rects_final->next != NULL);

	/*注意rects有头结点,所以传进去的时候忽略掉头结点*/
	draw_contour_rect(img_after_preprocess, rects_final->next);

	return rects_final->next;
#endif
}

/*通过形状比例筛选出满足形状比例的矩形*/
void filter_rect(List src_rects, List dst_rects, IplImage * org_img_car)
{
		static int i = 0;
	/*要保证dst_rects不为空才行!!*/
	if (src_rects == NULL) {
		fprintf(stderr, "src_rects is empty!\n");
		exit(-1);
	}
	while (src_rects != NULL) {
		double scale = 1.0 * (src_rects->item.width) / (src_rects->item.height);
		int area_of_rect = (src_rects->item.width) * (src_rects->item.height);

//		printf("%d area is %d\n",i, area_of_rect);
		/*车牌有固定的形状比例以及大小比例,先按这个粗略提取出车牌位置*/
		if (scale <= SCALE_MAX && scale >= SCALE_MIN && area_of_rect <= AREA_MAX && area_of_rect >= AREA_MIN ) {
			draw_rect(org_img_car, src_rects->item);
		//printf("%d area is %d scale is :%lf\n",i, area_of_rect, 1.0 * src_rects->item.width / src_rects->item.height);

			if (identity_by_color(org_img_car, src_rects->item)) {
				push_back(dst_rects, src_rects->item);
				dst_rects = dst_rects->next;
			}

		}

		src_rects = src_rects->next;
	}
		i++;
}



/*自定义膨胀腐蚀操作,在循环中使用可以增加定位准确度*/
void dilate_erode_x(IplImage * img_after_threshold, IplImage * img_final) {
	/*自定义1*3的核进行X方向的膨胀腐蚀*/
	IplConvKernel* kernal = cvCreateStructuringElementEx(3,1, 1, 0, CV_SHAPE_RECT);
	cvDilate(img_after_threshold, img_final, kernal, 3);/*X方向膨胀连通数字*/
	cvErode(img_final, img_final, kernal, 1);/*X方向腐蚀去除碎片*/
	cvDilate(img_final, img_final, kernal, 3);/*X方向膨胀回复形态*/
}

void dilate_erode_y(IplImage * img_final) {
	/*自定义3*1的核进行Y方向的膨胀腐蚀*/
	IplConvKernel* kernal = cvCreateStructuringElementEx(1, 3, 0, 1, CV_SHAPE_RECT);
	cvErode(img_final, img_final, kernal, 1);/*Y方向腐蚀去除碎片*/
//	cvDilate(img_final, img_final, kernal, 1);/*回复形态*/
}


bool identity_by_color(IplImage * img, CvRect rects)
{
	/*先从车辆中取出这个位置的矩形*/
	cvSetImageROI(img, rects);
	IplImage * src_rgb = cvCreateImage(cvSize(rects.width, rects.height), img->depth, img->nChannels);
	cvCopy(img, src_rgb);
	cvResetImageROI(img);

	IplImage * src = cvCreateImage(cvGetSize(src_rgb), src_rgb->depth, src_rgb->nChannels);

	cvCvtColor(src_rgb, src, CV_RGB2HSV);

	int dims = 1;  // 一维直方图
	int size =10; //bin的个数
	float range[] = {0, 255}; //取值范围
	float* ranges[] = {range};

	CvHistogram* hist;

	//创建直方图
	hist = cvCreateHist(dims , &size , CV_HIST_ARRAY ,  ranges , 1 );

	//清空直方图
	cvClearHist(hist);

	//给B  G  R 三个通道的图像分配空间
	IplImage* img_h = cvCreateImage(cvGetSize(src) , 8 ,1);
	IplImage* img_s = cvCreateImage(cvGetSize(src) , 8 ,1);
	IplImage* img_v = cvCreateImage(cvGetSize(src) , 8 ,1);

	//将图像src  分解成B   G   R 三个通道
	cvSplit(src , img_h , img_s , img_v , NULL);

	//计算R通道 直方图
	cvCalcHist(&img_h , hist , 0 , 0 );

	//绘制R通道直方图  hist_h

	if (histogram_analysing(hist,10) == true) {
		return true;
		//printf("It must be a plate!\n");
	} else {
		return false;
	//	printf("I don't think it is a plate\n");
	}

#if 0
	//将R通道的直方图数据清空
	cvClearHist(hist);

	cvNamedWindow("h");
	cvShowImage("h"  , hist_h);
	char filename_h[20];
	sprintf(filename_h, "%c_h.bmp", argv[1][24]);
	cvSaveImage(filename_h, hist_h);
	cvWaitKey(0);
	//释放资源
	//(1) 释放源图像
	cvReleaseImage(&src);

	//(2) 释放三个通道的图像
	cvReleaseImage(&img_h);


	//(3)释放三个通道直方图

	cvReleaseImage(&hist_h);

	//(4)释放直方图空间
	cvReleaseHist(&hist);

#endif

}

bool histogram_analysing(CvHistogram *hist, int bins)
{
	int i = 0;
	int sum = 0;
	double bin0 = -1;
	double bin1 = -1;
	int *ar = (int *)malloc(sizeof(int) * (bins + 1));
	for (i = 0; i < bins; i++) {
		float histValue = cvQueryHistValue_1D(hist , i);
		ar[i] = (int)histValue;
		sum += ar[i];
//		printf("%d\n", ar[i]);
	}

		bin0 = 1.0 * ar[0] / sum;
		bin1 = 1.0 * ar[1] / sum;

		//printf("bin0: %lf\n", bin0);

	if (bin0 > BIN_MIN)
		return true;
	else 
		return false;
}

#ifdef _DEBUG_
/*在图像上画矩形*/
void draw_rect(IplImage * img, CvRect rect)
{
	IplImage *tmp_img = cvCloneImage(img);
	cvRectangle(tmp_img, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.width, rect.y + rect.height), CV_RGB(0xbf, 0xbd, 0xab), 2, 8, 0);
//	cvNamedWindow("get_location", 1);
//	cvShowImage("get_location", tmp_img);
//	cvWaitKey(0);
}

#endif









