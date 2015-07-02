#include "include/plate.h"
#define COUNT_NUMBER 37
#define WIDTH_RESIZE 50
#define HEIGHT_RESIZE 100

/*
功能:通过像素点匹配进行数字的识别
输入:两张规格化为WIDTH_RESIZE*HEIGHT_RESIZE像素点的图片,
思路:一张是待识别的,十张是标准图片,进行匹配,匹配最相近的为数字
 */

char * chinese[31] = {
	"陕", 
	"鲁",
};

static int find_min(int diff[], int size);
static int compare_diff(IplImage *template_img, IplImage *char_img);

int character_recognizing(IplImage * img_char)
{
//	cvNamedWindow("char", 1);
//	cvShowImage("char", img_char);
//	cvWaitKey(0);
	char filename[100];
	IplImage * template_img;
	IplImage * template_img_after_resize;
	IplImage * img_char_after_resize;


	int diff[COUNT_NUMBER] = {0,0,0,0,0,0,0,0,0,0};
//	cvNamedWindow("character_recgnizing");
	int i = 0;
	int number = -1;

      if( freopen("result.txt", "a", stdout) < 0) {
          fprintf(stderr, "can not open file result.txt\n");
      }
	for (i = 0; i < COUNT_NUMBER; i++) {
		/*24和18对应O和I,不可能出现*/
		if (i == 24 || i == 18) {
			continue;
		}
		sprintf(filename, "../image/test_img/number/%d.png", i);
		template_img = cvLoadImage(filename, -1);
		if (template_img == NULL) {
			fprintf(stderr, "Can not open file %s\n", filename);
			exit(-1);
		}
		/*对模版图片进行尺寸归一化*/
		template_img_after_resize = cvCreateImage(cvSize(WIDTH_RESIZE, HEIGHT_RESIZE), template_img->depth, template_img->nChannels);
		cvResize(template_img, template_img_after_resize);
		/*对待识别图片进行尺寸归一化*/

		img_char_after_resize = cvCreateImage(cvSize(WIDTH_RESIZE, HEIGHT_RESIZE), template_img->depth, template_img->nChannels);
		cvResize(img_char, img_char_after_resize);
		diff[i] = compare_diff(template_img_after_resize, img_char_after_resize);
	}

	number = find_min(diff, COUNT_NUMBER);

	for (i = 0; i < COUNT_NUMBER; i++) {
		if (i == 18 || i == 24)
			continue;
		if (i < 10) {
//			printf("number %d: diff: %d\n", i, diff[i]);
		} else {
//			printf("character %c: diff: %d\n", i + 55, diff[i]);
		}
	}
	if (number < 10) {
		printf("the number is : %d\n", number);
	} else if (number < 36){
		printf("the character is : %c\n", number + 55);
	} if ( number >= 36) {
		printf("the chinese is :%s\n", chinese[number - 36]);
	}
	return number;
}


int compare_diff(IplImage *template_img, IplImage *char_img)
{
	int i = 0, j = 0;
	int diff = 0;

	for (i = 0; i < template_img->height; i++) {
		unsigned char * tprow = (unsigned char *)(template_img->imageData + i * template_img->widthStep);
		unsigned char * cprow = (unsigned char *)(char_img->imageData + i * char_img->widthStep);

		for (j = 0; j < template_img->width; j++) {
			if (tprow[j] != cprow[j])
				diff++;
		}
	}
	return diff;
}


int find_min(int diff[], int size)
{
	int min = 100000;
	int i = 0;
	int index = -1;

	for (i = 0; i < size; i++) {
		if (i == 18 || i == 24)
			continue;
		if (diff[i] < min) {
			min = diff[i];
			index = i;
		}
	}

	return index;
}









