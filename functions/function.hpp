#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>

/* Libraries OPENCV */
#include "highgui.h"
#include "cv.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

// functions declarations
void Sobel(uchar* im_in, uchar* im_out, uint rows, uint columns);
void medianHisto(uchar *im_in, uchar *im_out, int rows, int columns, int pRadiusKernel);
void medianHistoThread(uchar *im_in, uchar *im_out, int rows, int columns, int startAt, int endAt, int pRadiusKernel);
void Threshold(uchar* im_in, uchar* im_out, uint rows, uint columns);
void SkinDetectionRGB(uchar* im_in_red, uchar* im_in_green, uchar* im_in_blue, uchar* im_out, uint rows, uint columns);
void MvtDetectionRGB(uchar* im_in_old, uchar* im_in_current, uchar* im_out, uint rows, uint columns);
void SimpleBackgroundDetectionRGB(uchar* im_in_background, uchar* im_in_current, uchar* im_out, uint rows, uint columns);
void SimpleBackgroundDetectionHSV(uchar* im_in_background_sat, uchar* im_in_current_sat, uchar* im_out, uint rows, uint columns);

#endif
