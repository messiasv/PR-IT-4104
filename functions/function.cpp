#include "function.hpp"

/*
 * Sobel Filter to show the edges of objects
 * im_in : the input image in Gray (RGB)
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 */
void Sobel(uchar* im_in, uchar* im_out, uint rows, uint columns) {
  uint i;
  uchar *NW, *N, *NE, *W, *E, *SW, *S, *SE;
  int Gx, Gy, G;

  for (i = 0,
    NW = im_in - columns - 1, // pointers incrementation
    N = im_in - columns,
    NE = im_in - columns + 1,
    W = im_in - 1,
    E = im_in + 1,
    SW = im_in + columns - 1,
    S = im_in + columns,
    SE = im_in + columns + 1;
    i < (columns*rows);
    i++, NW++, N++, NE++, W++, E++, SW++, S++, SE++
  ) {
    if (i < columns || i%columns == 0 || i%columns == columns -1 || i >= (columns -1)*rows) { // if we are on the edges of the image
      im_out[i] = 0; // we put the output image pixel color to black
    }
    else { // we calculate  an approximation of the gradient's norm
      Gx = -(*NW) - (*W<<2) - (*SW) + (*NE) + (*E<<2) + (*SE);
      Gy = -(*NW) - (*N<<2) - (*NE) + (*SW) + (*S<<2) + (*SE);
      G = (abs(Gx) + abs(Gy))>>2;
      if (G > 255) G = 255; // limit the value of the pixels to 255
      im_out[i] = (uchar)G;
    }
  }
  for (i = 0; i < columns*rows; i++) { // filtering the edges
    if (im_out[i] > 50) im_out[i] = 255;
    else im_out[i] = 0;
  }
}

/*
 * Median Filter to smoothen the image and suppress noises
 * im_in : the input image in Gray (RGB)
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 * pRadiusKernel : the number used for the matrix of the algorithm (matrix of (pRadiusKernel*2+1) pixels)
 */
void medianHisto(uchar *im_in, uchar *im_out, int rows, int columns, int pRadiusKernel) {
  int const N = pRadiusKernel;  // kernel radius
  int i, itab, G, pi, pj, mediane;
  int const tailleCote(2 * N + 1);  // kernel's length
  int const tailleKernel(tailleCote*tailleCote);
  int tab[256];
  for (i = 0; i < columns*rows; i++) {
    if (i < columns || i%columns == 0 || i%columns == columns - 1 || i >= (columns - 1)*rows) { // if we are on the edges of the image
      im_out[i] = 0;
    }
    else {
      for (itab = 0; itab != 256; itab++) { // histogram initialization
        tab[itab] = 0;
      }
      pi = 0;
      pj = 0;
      for (pi = 0; pi != tailleCote; pi++) {  // histogram filling with the kernel's values
        for (pj = 0; pj != tailleCote; pj++) {
          tab[int(im_in[i - N*columns - N + pj + pi*columns])]++;
        }
      }
      mediane = 0;
      itab = 0;
      while (mediane < tailleKernel / 2) {  // median search
        mediane += tab[itab];
        itab++;
      }
      G = itab;
      if (G > 255) G = 255; // limit the value of the pixels to 255
      im_out[i] = (uchar)G;
    }
  }
}

/*
 * Median Filter to smoothen the image and suppress noises with multiple threads
 * im_in : the input image in Gray (RGB)
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 * startAt : the first pixel of the image the algorithm starts at
 * endAt : the last pixel of the image the algorithm ends at
 * pRadiusKernel : the number used for the matrix of the algorithm (matrix of (pRadiusKernel*2+1) pixels)
 */
void medianHistoThread(uchar *im_in, uchar *im_out, int rows, int columns, int startAt, int endAt, int pRadiusKernel) {
	int const N = pRadiusKernel; // kernel radius
	int i, itab, G, pi, pj, mediane;
	int const tailleCote((N << 1) + 1);  // kernel's length
	int const tailleKernel(tailleCote*tailleCote);
	int tab[256];
	int k=0;
	uchar value;
	for (i = startAt; i < endAt; i++) {
			for (itab = 0; itab != 256; itab++) {  // histogram initialization
				tab[itab] = 0;
			}
			pi = 0;
			pj = 0;
			for (pi = 0; pi != tailleCote; pi++) { // histogram filling with the kernel's values
				for (pj = 0; pj != tailleCote; pj++) {
					k=i - N*columns - N + pj + pi*columns;
					if(k<0 || k>columns*rows)
						value = 0;
					else
						value = im_in[k];
					tab[value]++;
				}
			}
			mediane = 0;
			itab = 0;
			while (mediane < tailleKernel / 2) { // median search
				mediane += tab[itab];
				itab++;
			}
			G = itab;
			if (G > 255) G = 255;  // limit the value of the pixels to 255
			im_out[i] = (uchar)G;
	}
}

/*
 * Threshold Filter to dynamically determine the background with a histogram of colours (based on Otsu's algorithm)
 * im_in : the input image in Gray (RGB)
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 */
void Threshold(uchar* im_in, uchar* im_out, uint rows, uint columns) {
  uint N = rows * columns;
  uint max_intensity = 255, value = 0;
  int sum = 0, sumB = 0, var_max = 0, q1 = 0, q2 = 0, u1 = 0, u2 = 0, threshold = 255, V = 0;
  int* histogram = (int*) calloc(256 ,sizeof(int));

  for(uint i = 0; i< N;i++) {
    value = im_in[i];
    histogram[value]++;
    sum += value;
  }

  for(uint i = 0; i <= max_intensity; i++) {
    q1 += histogram[i];   // q1 -> cumulative pixels number to the intensity i
    if(q1 == 0) {
      continue;
    }
    q2 = N - q1; // q2 -> remaining pixels number
    sumB += i * histogram[i];
    if(q2 == 0) {
      q2 = 1;
    }
    u1 = sumB / q1;  // u1 -> intensity average to i
    u2 = (sum - sumB) / q2; // u2 -> remaining intensity average

    V = q1 * q2 * ((u1 - u2) * (u1 - u2)); // V -> variance

    if(V > var_max) {
      threshold = i; // -> intensity treshold with the highest value of the variance
      var_max = V;
    }
  }

  for(uint i = 0; i < N; i++) {
    if(im_in[i] <= threshold) {
      im_out[i] = 255;
    }
  }
  free(histogram);
}

/*
 * Skin Detection Filter in RGB to show the skin of people
 * im_in_red : the input image in the Red channel (RGB)
 * im_in_green : the input image in the Green channel (RGB)
 * im_in_blue : the input image in the Blue channel (RGB)
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 */
void SkinDetectionRGB(uchar* im_in_red, uchar* im_in_green, uchar* im_in_blue, uchar* im_out, uint rows, uint columns){
  uint N = rows * columns;
  uchar R, G, B, max, min;
  for (uint i = 0; i < N; i++) {
    R = im_in_red[i];
    G = im_in_green[i];
    B = im_in_blue[i];
    max = B; // arbitrary
    min = G; // arbitrary
    if (R > B) {
      max = R;
      min = B;
      if (G > R){
        max = G;
      } else if (B > G){
        min = G;
      }
    }
    if ((R > 95) && (G > 40) && (B>20) &&
    (abs(R - G) > 15) && (R > G) &&
    (G > B)  && ((max - min) > 15)) {
      im_out[i] = 255;
    }
  }
}

/*
 * Movement Detection Filter in RGB to show the movement path of objects
 * im_in_old : the former input image in Gray (RGB) used as the background
 * im_in_current : the current input image in Gray (RGB)
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 */
void MvtDetectionRGB(uchar* im_in_old, uchar* im_in_current, uchar* im_out, uint rows, uint columns) {
  uint N = rows * columns;
  uint NbImages = 4;
  int threshold = 35;
  uchar* background = (uchar*)calloc(N, sizeof(uchar));
  for (uint i = 0; i < N; i++) {
    background[i] = ((NbImages - 1) * im_in_old[i] + im_in_current[i]) / NbImages;
    if (abs(background[i] - im_in_current[i]) > threshold) {
      im_out[i] = 255;
    }
  }
  free(background);
}

/*
 * Background Detection Filter in RGB to show the objects of the foreground with a fixed background
 * im_in_background : the background input image in Gray (RGB) saved at the beginning
 * im_in_current : the current input image in Gray (RGB) with objects in the foreground
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 */
void SimpleBackgroundDetectionRGB(uchar* im_in_background, uchar* im_in_current, uchar* im_out, uint rows, uint columns) {
  uint N = rows * columns;
  uint NbImages = 20;
  int threshold = 40;
  uchar* background = (uchar*)calloc(N, sizeof(uchar));
  for (uint i = 0; i < N; i++) {
    background[i] = ((NbImages - 1) * im_in_background[i] + im_in_current[i]) / NbImages;
    if (abs(im_in_background[i] - im_in_current[i]) > threshold) {
      im_out[i] = 255;
    }
  }
  free(background);
}

/*
 * Background Detection Filter in HSV to show the objects of the foreground with a fixed background
 * im_in_background_sat : the background input image in the Saturation channel (HSV) saved at the beginning
 * im_in_current_sat : the current input image in the Saturation channel (HSV) with objects in the foreground
 * im_out : the output image
 * rows : the number of rows  of the image (image height)
 * columns : the number of columns of the image (image width)
 */
void SimpleBackgroundDetectionHSV(uchar* im_in_background_sat, uchar* im_in_current_sat, uchar* im_out, uint rows, uint columns) {
  uint N = rows * columns;
  uint NbImages = 20;
  int threshold_sat = 30;
  uchar* im_inter_out = (uchar*)calloc(N, sizeof(uchar));
  uchar* im_inter2_out = (uchar*)calloc(N, sizeof(uchar));
  uchar* background_out = (uchar*)calloc(N, sizeof(uchar));
  for (uint i = 0; i < N; i++) {
    background_out[i] = ((NbImages - 1) * im_in_background_sat[i] + im_in_current_sat[i]) / NbImages;
    if (abs(background_out[i] - im_in_current_sat[i]) > threshold_sat) {
      im_inter_out[i] = 255;
    }
  }
  // medianHisto(im_inter_out, im_out, rows, columns, 3); // -> MedianHisto running on one thread

  // support for multithreading
  #pragma omp parallel for num_threads(4)
  for(int n=0;n<4;n++){
    int startAt = n*columns*rows/4;
    int endAt = (n+1)*columns*rows/4-1;
    medianHistoThread(im_inter_out, im_inter2_out, rows, columns, startAt, endAt, 3);
  }

  #pragma omp parallel for num_threads(4)
  for(int n=0;n<4;n++){
    int startAt = n*columns*rows/4;
    int endAt = (n+1)*columns*rows/4-1;
    medianHistoThread(im_inter2_out, im_out, rows, columns, startAt, endAt, 3);
  }
  free(im_inter_out);
  free(im_inter2_out);
  free(background_out);
  for (uint i = 0; i < N; i++) {
    if (im_out[i] <= 30) im_out[i] = 0;
    else im_out[i] = 255;
  }
}
