#include <opencv2/opencv.hpp>
#include <iostream>
 
using namespace std;
using namespace cv;
 
int main(int argc, char **argv)
{
    Mat img = imread("cat.jpg");
    if (img.empty())
    {
        cout << "打开图像失败！" << endl;
        return -1;
    }
    imshow("image", img);
    waitKey();
 
    return 0;
}
