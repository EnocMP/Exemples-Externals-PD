#include<stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main (int argc, char **argv) {
  
  char* imageName = argv[1];
  
  if (argc != 2) {
    printf("afegeix l'arxiu!\n");
    return -1;
  }
  Mat image;
  Mat gray_image;
  
  image = imread(imageName, 1);
  
  cvtColor (image, gray_image, CV_BGR2GRAY); //change from RGB to grey scale
  
  namedWindow("Display Image", CV_WINDOW_AUTOSIZE);
  namedWindow(imageName, CV_WINDOW_AUTOSIZE);
  
  imshow(imageName, image);
  imshow("Display Image", gray_image);
  waitKey(0);
  return 0;
}