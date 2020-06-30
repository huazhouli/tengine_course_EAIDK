#include "bladecv.hpp"
#include <stdio.h>
#include <math.h>
#include "openai_io.hpp"

using namespace fcv;
using namespace std;

int main()
{
    string filename = "a15.jpg";
    Mat src_img = imread(filename);

    Size src_sz = src_img.size();
    Size dst_sz(src_sz.height, src_sz.width);
    printf("image resolution:%d,%d\n",src_img.cols, src_img.rows);

    //use cv::mat
    Mat cvmat2 = imread(filename, 1);

    /* Show fcvMat */
    mark_info info;
    info.type = TEXT;
    strcpy(info.textstr, "OPEN AI LAB");
    info.font_size = 20;
    info.width = 150;
    info.height = 50;
    info.index = 1;
    info.x = 10;
    info.y = 20;
    namedWindow("OPEN AI LAB", WINDOW_NORMAL);
    imshow("OPEN AI LAB", cvmat2, &info);
    waitkey(3000);
    destroyWindow("OPEN AI LAB");

    return 0;
}
