#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "include/plate.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
using namespace std;

void showImage(char *wname, char * filename);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_7_clicked()
{
    QString Qfilename = ui->lineEdit->text();
    string filename = Qfilename.toStdString();
    main_plate(filename.c_str());

    FILE * fp = fopen("result.txt", "r");

    char buf[1000];
    while (feof(fp) == 0)
    {
    fread(buf, 1000, sizeof(char), fp);
    fprintf(stderr, "%s", buf);
	ui->label_2->setText(buf);
    }
    fclose(fp);
}

void MainWindow::on_pushButton_4_clicked()
{
    showImage("img_after_preprocess", "image/img_after_preprocess.bmp");
}

void showImage(char *wname, char * filename)
{
    cvNamedWindow(wname, 1);
    IplImage * img = cvLoadImage(filename);
    cvShowImage(wname, img);

}

void MainWindow::on_pushButton_clicked()
{
    showImage("img_plate_after_preprocess", "image/img_plate_after_preprocess.bmp");
}

void MainWindow::on_pushButton_9_clicked()
{
    showImage("img_car_with_rect", "image/img_car_with.bmp");
}

void MainWindow::on_pushButton_10_clicked()
{
    showImage("img_plate_with_rect", "image/img_plate_with_rect.bmp");
}

void MainWindow::on_pushButton_6_clicked()
{
    showImage("img_plate_with_rect", "image/img_car_after_resize.bmp");

}

void MainWindow::on_pushButton_8_clicked()
{
    showImage("img_plate_with_rect", "image/img_car_after_resize.bmp");
}

void MainWindow::on_pushButton_2_clicked()
{
    showImage("img_plate_with_rect", "image/img_car_with_rect.bmp");

}

void MainWindow::on_pushButton_3_clicked()
{
    showImage("img_plate_with_rect", "image/plate_img_after_resize.bmp");

}

void MainWindow::on_pushButton_5_clicked()
{

}
