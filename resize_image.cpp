#include "include/plate.h"

/*
 功能:对图像进行尺寸归一化处理
 输入:拉长/压缩比例(0.1 <= scale <= 10)
 输出:归一化完成后的图像
 */

void resize_image(IplImage * img_to_resize, IplImage * img_after_resize, float scale)
{
	if (scale < 0.1 || scale > 10) {
		fprintf(stderr, "Scale must be between 0.1 and 10.\n");
		exit(-1);
	}
	if (img_to_resize == NULL) {
		fprintf(stderr, "img_to_resize is NULL!\n");
		exit(-1);
	}
	CvSize dst_size;

	dst_size.width = (int)(img_to_resize->width * scale);
	dst_size.height = (int)(img_to_resize->height * scale);

	img_after_resize = cvCreateImage(dst_size, img_to_resize->depth, img_to_resize->nChannels);

	cvResize(img_to_resize, img_after_resize);
	
	cvSaveImage("image/plate_img_after_resize.bmp", img_after_resize);
}

