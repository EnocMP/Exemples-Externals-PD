#include<stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main (int argc, char **argv) {
  
  if (argc != 2) {
    printf("afegeix l'arxiu!\n");
    return -1;
  }
  Mat image;
  image = imread(argv[1], 1);
  namedWindow("Display Image", CV_WINDOW_AUTOSIZE);
  imshow("Display Image", image);
  waitKey(0);
  return 0;
}