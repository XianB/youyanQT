#-------------------------------------------------
#
# Project created by QtCreator 2015-07-01T01:26:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = youyanQT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
	character_recognizing.cpp \
	get_character.cpp  \
	get_plate_image.cpp\
	main_plate.cpp \
	preprocess.cpp \
	resize_image.cpp\
	find_contours.cpp\
	get_location.cpp\
	List.cpp\
	preprocess_plate_image.cpp



HEADERS  += mainwindow.h \
	include/plate.h

FORMS    += mainwindow.ui

INCLUDEPATH += /usr/local/include \
                /usr/local/include/opencv \
                /usr/local/include/opencv2

LIBS += /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_core.so    \
        /usr/local/lib/libopencv_imgproc.so
