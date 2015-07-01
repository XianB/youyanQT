#include "include/plate.h"

//#define LN

static void cut_image(IplImage * img); 
static double plate_resize_scale(IplImage * img_plate);

int main_plate(const char * car_name)
{
	
	/*********************************************准备工作*****************************************/
	IplImage * img_car = NULL;
	IplImage * img_car_after_resize = NULL;
	IplImage * img_after_preprocess = NULL;
	IplImage * img_plate = NULL;
	IplImage * img_after_resize = NULL;
	IplImage * img_character = NULL;

	List rects; /*保存预选车牌位置矩形的列表*/
	double scale = -1; /*在尺寸归一化时要用到*/
	int width = 0, height = 0; /*最开始时候的尺寸归一化的长宽*/
	int number = -1;	/*最后一个字符的数字结果*/
	int count_recog = 0;
	char filename[50];

#if 1
	//cvNamedWindow("img_car", 1);
//	cvNamedWindow("img_car_after_resize", 1);
	//cvNamedWindow("img_after_preprocess", 1);
	//cvNamedWindow("img_plate", 1);
#endif

	if ((img_car = cvLoadImage(car_name, -1)) == NULL) {
		fprintf(stderr, "Can not open car image file in main.c!\n");
		exit(-1);
	}

	/*****************************************开始进行图像处理***************************************/
	/*由于得到的车辆图像中车占的比例太小,所以需要考虑重新截取图像,保证得到的图像中车辆整体占整个图像的比例较大
	 要实现这个目的我们观察发现拍到的照片中车基本都是处于整个图像的中心,所以我们截取整个图像的中心作为新的图片
	 策略:
	 1.先将图片按宽度分成三份,取中间的一份,车牌肯定在这一份中
	 2.将图片上四分之一截取掉,下四分之一截取点,车牌肯定在剩下的二分之一份图片中
	 */
	/*********现在开始进行截取车身操作****************/
#if 0
	IplImage * tmp_img = cvCreateImage(cvSize(1.0 / 3 * img_car->width, 1.0 / 2 * img_car->height), img_car->depth, img_car->nChannels);
	cvSetImageROI(img_car, cvRect(1.0 / 3 * img_car->width, 1.0 / 4 * img_car->height, 1.0 / 3 * img_car->width, 1.0 / 2 * img_car->height));
	cvCopy(img_car, tmp_img);
	cvSaveImage("tmp_img.bmp", tmp_img);
	cvResetImageROI(img_car);
	img_car = cvLoadImage("tmp_img.bmp", -1);					/*img_car现在是新的截取后的图片了*/
	assert(img_car != NULL);

	cvNamedWindow("haha", 1);
	cvShowImage("haha", tmp_img);
	cvWaitKey(0);
#endif

	cut_image(img_car);
	img_car = cvLoadImage("image/tmp_img.bmp", -1);					/*img_car现在是新的截取后的图片了*/

	/********************************************************************************************************/
	/*为了便于对图像进行统一处理,先对图像尺寸进行处理,让图像的尺寸大小合适,
	  一般大概大小为640*480规格的,所以只需要大概按照这个比例进行resize
	 */



	/*用cvResize函数进行处理即可*/
#if 1
	scale = 1.0 * 640 / img_car->width;			/*将长度规整为640即可,宽就按比例伸长就行了*/
	width = scale * img_car->width;
	height = scale * img_car->height;
	img_car_after_resize = cvCreateImage(cvSize(width, height), img_car->depth, img_car->nChannels);
	cvResize(img_car, img_car_after_resize);			/*对尺寸进行归一化,得到宽为640的图像*/
#endif


	/*图像预处理:输入为尺寸归一化后的车牌图像,输出为一张img_after_preprocess.bmp图像*/
	preprocess_car_img(img_car_after_resize);

	/*读取img_after_preprocess.bmp图像*/
	if ((img_after_preprocess = cvLoadImage("image/img_after_preprocess.bmp", -1)) == NULL) {
		fprintf(stderr, "Can not open file img_after_preprocess.bmp in main.c");
		exit(-1);
	}

#if 1
	/*显示预处理完成后的图像*/
	//cvShowImage("img_car", img_after_preprocess);
	//cvShowImage("img_after_preprocess", img_after_preprocess);
#endif
	
	/***************************************预处理完成,开始找车牌位置*****************************************************************/
	rects = get_location(img_after_preprocess, img_car_after_resize);			/*得到车牌的位置,起初设计阶段是可以有多个预选位置,但是后来发现不用,所以rects其实只有一个位置,但是也是用一个链表装着的*/
	/*由于在get_location中返回的是头结点的next节点,所以这里的参数不用rects->next*/
	assert(count_node(rects) == 1);						/*断言这个链表里只有一个待选车牌位置*/
	/****************************************找到车牌位置,开始截取车牌******************************************************************/
	get_plate_image(img_car_after_resize, rects);		/*得到车牌的图像*/

	img_plate = cvLoadImage("image/plate_img0.bmp", -1);		/*上面那个函数中得到的plate_img.bmp图像*/
	if (img_plate == NULL) {
		fprintf(stderr, "Can not open plate image file!\n");
		exit(-1);
	}

	/*******************************************对车牌进行尺寸变化***************************************************************/
	scale = plate_resize_scale(img_plate);
	resize_image(img_plate,img_after_resize, scale);		/*最后一个参数为5表示将原车牌图像变长为原来的五倍*/
	if ((img_after_resize = cvLoadImage("image/plate_img_after_resize.bmp", -1)) == NULL) {
		fprintf(stderr, "Can not open file plate_img_after_resize.bmp in main.c");
		exit(-1);
	}

	/*******************************************对车牌进行预处理***************************************************************/
	preprocess_plate_image(img_after_resize);			/*对车牌图像进行预处理*/
	
	/********************************************获得车牌上的字符信息**************************************************************/
	get_character(img_after_resize);					/*得到每一个字符的图像*/
	//cvShowImage("image_car", img_after_resize);
	//printf("the plate is: \n");
	count_recog = 0;
	while (count_recog < 7) {

		sprintf(filename, "image/character%d.png", count_recog);

		img_character = cvLoadImage(filename, -1);

		if (img_character == NULL) {
			break;
		}

	/*********************************************开始进行字符识别***********************************************************/

		number = character_recognizing(img_character);
		count_recog++;
	}

	cvWaitKey(0);
	printf("Time used = %.2f\n", (double)clock() / CLOCKS_PER_SEC);
	return 0;
}



//由于车牌的位置不好找到，所以假设车牌处于图像的正中央，这个函数就是对图像进行了裁剪得到的
//输出是tmp_img.bmp图像
static void cut_image(IplImage * img_car) 
{
#ifndef LN
	IplImage * tmp_img = cvCreateImage(cvSize(1.0 / 3 * img_car->width, 1.0 / 2 * img_car->height), img_car->depth, img_car->nChannels);
	//设置感兴趣区域，饭后将该区域保存下来到tmp_img.bmp中
	cvSetImageROI(img_car, cvRect(1.0 / 3 * img_car->width, 1.0 / 4 * img_car->height, 1.0 / 3 * img_car->width, 1.0 / 2 * img_car->height));
	cvCopy(img_car, tmp_img);
	cvSaveImage("image/tmp_img.bmp", tmp_img);
	cvResetImageROI(img_car);
	assert(img_car != NULL);
#endif

#ifdef LN
	IplImage * tmp_img = cvCreateImage(cvSize(1.0 / 2 * img_car->width, 1.0 / 1 * img_car->height), img_car->depth, img_car->nChannels);
	cvSetImageROI(img_car, cvRect(1.0 / 4 * img_car->width, 0 / 1 * img_car->height, 1.0 / 2 * img_car->width, 1.0 / 1 * img_car->height));
	cvCopy(img_car, tmp_img);
	cvSaveImage("image/tmp_img.bmp", tmp_img);
	cvResetImageROI(img_car);
	assert(img_car != NULL);
#endif

#if 0
	cvNamedWindow("haha", 1);
	cvShowImage("haha", tmp_img);
	cvWaitKey(0);
#endif

}


static double plate_resize_scale(IplImage * img_plate)
{
	return 1.0 * RESIZED_HEIGHT / img_plate->height;
}








