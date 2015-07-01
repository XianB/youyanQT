#include "include/plate.h"
#define PERCENT 0.8

/*
	功能是获得最后一个车牌字符
 */
static void find_character(IplImage * img, List rects);
static void filter_rect_by_area(List src_rects, List dst_rects, int total_area);
/*去除最左最右的可能是边框的矩形*/
static void filter_rect_lr(List rects, IplImage * img_plate);

void make_border_black(IplImage *img);

void get_character(IplImage * img) 
{
	img = cvLoadImage("image/img_plate_after_preprocess.bmp", -1);
	if (img == NULL) {
		fprintf(stderr, "Can not open file img_plate_after_preprocess.bmp");
		exit(-1);
	}

	/*也是两个链表用来存放矩形的*/
	List rects = create_list();
	List rects_final = create_list();

	if (img == NULL) {
		fprintf(stderr, "error in openning file.\n");
		exit(-1);
	}

	CvMemStorage * storage = cvCreateMemStorage(0);
	CvSeq * contours = NULL;

	remove_border_ul(img); /*消除上下边界*/
	IplImage * img_after_removed = cvLoadImage("image/img_after_border_removed.bmp", -1);
	if (img_after_removed == NULL) {
		fprintf(stderr, "Can not open file img_after_border_removed.bmp");
		exit(-1);
	}

	make_border_black(img_after_removed);
	get_contour_rect(img_after_removed, rects, storage, contours);		/*找到所有矩形*/
	filter_rect_by_area(rects->next, rects_final, img_after_removed->width * img_after_removed->height);
	/*现在还要继续去除左右边界:
	 左右边界的特点是:
	 1.几乎全为白色点,注意1也是这样
	 2.面积不在1的范围之内,1的最大最小值定义在头文件中了
	 3.比例比1的大或小,比例也定义在头文件中了
	 */
	rects_final = sort(rects_final);
	filter_rect_lr(rects_final, img_after_removed);
	draw_contour_rect(img_after_removed, rects_final->next);
	find_character(img_after_removed, rects_final);			/*也没有什么好改进的地方了*/
}

/*消除上下边界*/
void remove_border_ul(IplImage * img_plate)
{
	int i = 0, j = 0;
	/*这两个变量分别为上下边界的高度*/
	int up_bound = -1, low_bound = -1;
	int white_to_black = 0;
	int black_to_white = 0;
	/*从i从0 到高度一半进行遍历,进行投影,找到上边界*/
	//cvNamedWindow("img_plate", 1);
	//cvShowImage("img_plate", img_plate);
	//cvWaitKey(0);
	for (i = 0; i < (img_plate->height) / 2; i = i + 3) {

		unsigned char * prow = (unsigned char *)(img_plate->imageData + i * img_plate->widthStep);
		white_to_black = 0;
		black_to_white = 0;

		/*记录下每一行的black_to_white和w_to_b的个数*/
		for (j = 0; j < img_plate->width; j = j + 3) {
			if (prow[j] == 0 && prow[j + 3] == 255) {
				black_to_white++;
			} else if (prow[j] == 255 && prow[j + 3] == 0) {
				white_to_black++;
			}
		}

		/*设成6的话对图片的清晰度有很高的要求*/
		if (black_to_white >= 6 && white_to_black >= 6 && up_bound < 0) {
			up_bound = i;
		} else if (black_to_white < 6 && white_to_black < 6 && up_bound > 0) {
//			printf("black_to_white : %d whilte_to_black: %d , up_bound %d\n",black_to_white, white_to_black, up_bound);
			up_bound = -1;
		}
	}

	/*i从最底端到高度的一半进行遍历*/
	for (i = img_plate->height - 1; i > (img_plate->height) / 2; i = i - 3) {
		unsigned char * prow = (unsigned char *)(img_plate->imageData + i * img_plate->widthStep);
		white_to_black = 0;
		black_to_white = 0;

		/*记录下每一行的black_to_white和w_to_b的个数*/
		for (j = 0; j < img_plate->width; j = j + 3) {
			if (prow[j] == 0 && prow[j + 3] == 255) {
				black_to_white++;
			} else if (prow[j] == 255 && prow[j + 3] == 0) {
				white_to_black++;
			}
		}

		if (black_to_white >= 6 && white_to_black >= 6 && low_bound < 0) {
			low_bound = i;
		} else if (black_to_white < 6 && white_to_black < 6 && low_bound > 0) {
			low_bound = -1;
		}
	
		//printf("%d\n", low_bound);
	}
	
#if 0
	cvNamedWindow("img", 1);
	printf("up_bound is %d, low_bound is %d\n", up_bound, low_bound);

	/*画直线操作*/
	/*	void cvLine( CvArr* img, CvPoint pt1, CvPoint pt2, CvScalar color, int thickness=1, int line_type=8, int shift=0 );*/
	cvLine(img_plate, cvPoint(0, up_bound), cvPoint(img_plate->width - 3, up_bound), CV_RGB(0xbF, 0xfd, 0xba), 3, 8, 0);
	cvLine(img_plate, cvPoint(0, low_bound), cvPoint(img_plate->width - 3, low_bound), CV_RGB(0xbF, 0xfd, 0xba), 3, 8, 0);
	cvShowImage("img", img_plate);
	cvWaitKey(0);
#endif
	/*这里容易出错!*/

//printf("%d %d %d %d", 0, up_bound, img_plate->width - 2, low_bound - up_bound - 2);
//	printf("low_bound:%d up_bound:%d\n", low_bound, up_bound);

	assert(low_bound >= 0 && up_bound >= 0);
	cvSetImageROI(img_plate, cvRect(0, up_bound, img_plate->width - 2, low_bound - up_bound - 2));		/*-2保证不要越界*/
	IplImage * tmp_img = cvCreateImage(cvSize(img_plate->width - 2, low_bound - up_bound - 2), img_plate->depth, img_plate->nChannels);
	cvCopy(img_plate, tmp_img);
	cvSaveImage("image/img_after_border_removed.bmp", tmp_img);
	cvResetImageROI(img_plate);
//	printf("setROI in remove bound success\n");
}

/*通过大小比例筛选出满足形状比例的矩形*/
void filter_rect_by_area(List src_rects, List dst_rects, int total_area)
{
	if (src_rects == NULL) {
		fprintf(stderr, "src_rects is empty!\n");
		exit(-1);
	}
	while (src_rects != NULL) {

		double scale = 1.0 *(src_rects->item.height) / (src_rects->item.width);  
		int area_of_rect = (src_rects->item.width) * (src_rects->item.height);

	//	printf("in find rects character %d %d %d %d \n", src_rects->item.x, src_rects->item.y, src_rects->item.x + src_rects->item.width, src_rects->item.y + src_rects->item.height);
		/*车牌有固定的形状比例以及大小比例,先按这个粗略提取出车牌位置*/
		if (area_of_rect > (total_area / 15) || (scale > 5 && area_of_rect > (total_area / 60))) {
//			printf("area %d\n", area_of_rect);
			push_back(dst_rects, src_rects->item);
			dst_rects = dst_rects->next;
		}
		src_rects = src_rects->next;
	}
}


/*功能:找到最后一个字符的图片
  参数:去除边界后的车牌图片,得到的矩形框列表
  思想:过找到rects中的各个矩形中左上角坐标最靠右边的那个矩形即可
 */
void find_character(IplImage * img, List rects)
{
	rects = rects->next;
	if (rects == NULL) {
		fprintf(stderr, "rects is NULL in find_rects->item_character function.\n");
		exit(-1);
	}
//	cvNamedWindow("character", 1);
	int max = -1;
	int count = 0;
	char filename[50];
	/*先对字符链表按左上角坐标排序*/
	/*rects是带头结点的*/
//	printf("here!\n");
	CvRect crect = cvRect(-1, -1, -1, -1);
	IplImage * img_character;

	/*遍历链表寻找x坐标最大的矩形*/
	while (rects != NULL) {
#if 0
		if(rects->item.x > max) {
			max = rects->item.x;
			rects->item = rects->item;
		}
//		printf("in find rects character %d %d %d %d \n", rects->item.x, rects->item.y, rects->item.x + rects->item.width, rects->item.y + rects->item.height);
		rects = rects->next;

#endif
		assert(rects->item.x > 0 && rects->item.y > 0 && rects->item.width > 0 && rects->item.height > 0);

	//printf("in find rects->item character %d %d %d %d \n", rects->item.x, rects->item.y, rects->item.x + rects->item.width, rects->item.y + rects->item.height);
	//printf("in find rects->item character %d %d %d %d \n", 0, 0, img->width, img->height);
		cvSetImageROI(img, rects->item);
		img_character = cvCreateImage(cvSize(rects->item.width, rects->item.height), img->depth, img->nChannels);
//		printf("setROI in find_rects->item_character success\n");

		sprintf(filename, "image/character%d.png", count);
		cvCopy(img, img_character, 0);
//		printf("area of character:%d scale:%lf\n", img_character->width * img_character->height, 1.0 * img_character->width / img_character->height);
		cvSaveImage(filename, img_character);
//		cvShowImage("character", img_character);
//		cvWaitKey(0);
		rects = rects->next;
		count++;
	}
}


/*输入:经过面积筛选的矩形
 输出:去除左右矩形的矩形链表
 筛选方法:
 1.里面的像素点几乎全是白的
 2.由于和1一样,所以面积要不在1的范围之内
 3.比例不能一样*/
void filter_rect_lr(List rects, IplImage * img_plate)
{
	List head = rects;
	IplImage *tmp = cvCloneImage(img_plate);
	rects = rects->next;
	int count_white = 0;	/*计算矩形内白点个数*/
	int i = 0, j = 0;
	int area = 0;
	int width, height;
	int x, y;
	double scale = 0;
	int count = 0;

	while(rects != NULL) {
		count_white = 0;

		area = rects->item.width * rects->item.height;
		scale = 1.0 * rects->item.width / rects->item.height;
		width = rects->item.width;
		height = rects->item.height;
		x = rects->item.x;
		y = rects->item.y;

//		printf("area: %d scale :%lf\n", area, scale);
		cvRectangle(tmp, cvPoint(rects->item.x, rects->item.y), cvPoint(rects->item.x + rects->item.width, rects->item.y + rects->item.height), CV_RGB(0xbF, 0xbd, 0xab), 1, 8, 0);
	
		//cvNamedWindow("tmp", 1);
		//cvShowImage("tmp", tmp);
		//cvWaitKey(0);

		for (i = y; i < height + y; i++) {
			unsigned char * prow = (unsigned char *)(img_plate->imageData + i * img_plate->widthStep);
			for (j = x; j < width + x; j++) {
				if (prow[j] == 255) {
					count_white++;
				}
			}

		}
	//	printf("count_white: %d\n", count_white);

		if ((1.0 * count_white / area > PERCENT) && (area > AREA_MAX_1 || area < AREA_MIN_1) && (scale > SCALE_MAX_1 || scale < SCALE_MIN_1) ) {
			delete_node(head, rects->item);
		}
		if ((1.0 * count_white / area > PERCENT) && count == 0) {
			delete_node(head, rects->item);
		}
		rects = rects->next;
		count++;
	}
}

/*由于检测不到1的轮廓,我觉得是因为1上下都挨着边界了,所以我要手动把上下两行设成黑色的*/

#if 1
void make_border_black(IplImage *img)
{
	int j = 0;
	unsigned char * prow = (unsigned char * )(img->imageData + 0 * img->widthStep);

	for (j = 0; j < img->width; j++) {
		prow[j] = 0;
	}
	prow = (unsigned char * )(img->imageData + (img->height - 1) * img->widthStep);
	for (j = 0; j < img->width; j++) {
		prow[j] = 0;
	}

}
#endif





