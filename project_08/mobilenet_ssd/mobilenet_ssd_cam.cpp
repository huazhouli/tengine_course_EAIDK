/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * License); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (c) 2020, OPEN AI LAB
 * Author: sqfu@openailab.com
 */

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sys/time.h>
#include "tengine_operations.h"
#include "tengine_cpp_api.h"
#include "common.hpp"
#include "mipi_cam.hpp"
static std::string gExcName{""};




const string wintitle = "mssd_cam";

struct Box
{
    float x0;
    float y0;
    float x1;
    float y1;
    int class_idx;
    float score;
};

void get_input_data_frame(cv::Mat &image_file, float* input_data, int img_h, int img_w, const float* mean, float scale)
{
    cv::Mat sample = image_file;
    if(sample.empty())
    {
        std::cerr << "Failed to read image file " << image_file << ".\n";
        return;
    }
    cv::Mat img;
    if(sample.channels() == 4)
    {
        cv::cvtColor(sample, img, cv::COLOR_BGRA2BGR);
    }
    else if(sample.channels() == 1)
    {
        cv::cvtColor(sample, img, cv::COLOR_GRAY2BGR);
    }
    else
    {
        img = sample;
    }

    cv::resize(img, img, cv::Size(img_h, img_w));
    img.convertTo(img, CV_32FC3);
    float* img_data = ( float* )img.data;
    int hw = img_h * img_w;
    for(int h = 0; h < img_h; h++)
    {
        for(int w = 0; w < img_w; w++)
        {
            for(int c = 0; c < 3; c++)
            {
                input_data[c * hw + h * img_w + w] = (*img_data - mean[c]) * scale;
                img_data++;
            }
        }
    }
}


/*
*   Post processing for mobilenet_ssd detection
*   including draw box and label targets
*/
void post_process_ssd_frame(cv::Mat img, float threshold, float* outdata, int num)
{
    const char* class_names[] = {"background", "aeroplane", "bicycle",   "bird",   "boat",        "bottle",
                                 "bus",        "car",       "cat",       "chair",  "cow",         "diningtable",
                                 "dog",        "horse",     "motorbike", "person", "pottedplant", "sheep",
                                 "sofa",       "train",     "tvmonitor"};

    int raw_h = img.size().height;
    int raw_w = img.size().width;
    std::vector<Box> boxes;
    int line_width = raw_w * 0.005;
    for(int i = 0; i < num; i++)
    {
        // Compared with threshold, keeping the result if the value larger than threshold, otherwise
        // delete the result
        if(outdata[1] >= threshold)
        {
            Box box;
            box.class_idx = outdata[0];
            box.score = outdata[1];
            box.x0 = outdata[2] * raw_w;
            box.y0 = outdata[3] * raw_h;
            box.x1 = outdata[4] * raw_w;
            box.y1 = outdata[5] * raw_h;
            boxes.push_back(box);
            printf("%s\t:%.0f%%\n", class_names[box.class_idx], box.score * 100);
            printf("BOX:( %g , %g ),( %g , %g )\n", box.x0, box.y0, box.x1, box.y1);
        }
        outdata += 6;
    }

    // drawing boxes and label the target into image
    for(int i = 0; i < ( int )boxes.size(); i++)
    {
        Box box = boxes[i];
        cv::rectangle(img, fcv::Rect(box.x0, box.y0, (box.x1 - box.x0), (box.y1 - box.y0)), fcv::Scalar(255, 255, 0),
                      line_width);
        std::ostringstream score_str;
        score_str << box.score;
        std::string label = std::string(class_names[box.class_idx]) + ": " + score_str.str();
        int baseLine = 0;
        fcv::Size label_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        cv::rectangle(img, cv::Rect(cv::Point(box.x0, box.y0 - label_size.height),
                           fcv::Size(label_size.width, label_size.height + baseLine)),
                           fcv::Scalar(255, 255, 0), CV_FILLED);
        fcv::putText(img, label, fcv::Point(box.x0, box.y0), cv::FONT_HERSHEY_SIMPLEX, 0.5, fcv::Scalar(0, 0, 0));
    }


	
}

void show_usage()
{
    std::cout << "[Usage]: " << gExcName << " [-h]\n"
              << "   [-m model_file] [-i image_file]\n";
}

int main(int argc, char* argv[])
{
    gExcName = std::string(argv[0]);
    std::string model_file;
    std::string image_file;
    const char* device = nullptr;

    int ret, c, r;
    char v4l2_dev[64], isp_dev[64];
    char index = -1;
    pthread_t id;
    Mat image;


    /* Window -- create */
    fcv::namedWindow(wintitle);
    fcv::moveWindow(wintitle, 720, 480);

    /* MIPI Camera -- default values */
    int mipi = 1;    /* main camera */
    enum CAM_TYPE type = CAM_OV9750; /* HD camera sensor: OV9750 */
    __u32 width = 640, height = 480; /* resolution: 640x480 */
    RgaRotate rotate = RGA_ROTATE_NONE; /* No rotation */
    __u32 cropx = 0, cropy = 0, cropw = 0, croph = 0;
    int vflip = 0, hflip = 0; /* no flip */


    if (type == CAM_IMX258)
    {
        type = CAM_OV9750;
        printf ("IMX258 is not supported currently. Use OV9750 instead!\n");
    }

    /* V4L2 device */
    sprintf(v4l2_dev, "/dev/video%d", 4 * (mipi - 1) + 2);
    sprintf(isp_dev, "/dev/video%d", 4 * (mipi - 1) + 1);

    printf("width = %u, height = %u, rotate = %u, vflip = %d, hflip = %d, crop = [%u, %u, %u, %u]\n",
           width, height, rotate, vflip, hflip, cropx, cropy, cropw, croph);



    /* MIPI Camera -- initialization */
    v4l2Camera v4l2(width, height, rotate, vflip, hflip, cropx, cropy, cropw, croph, V4L2_PIX_FMT_NV12);
    image.create(cv::Size(RGA_ALIGN(width, 16), RGA_ALIGN(height, 16)), CV_8UC3);
    ret = v4l2.init(v4l2_dev, isp_dev, type);
    if(ret < 0)
    {
        printf("v4l2Camera initialization failed.\n");
        return ret;
    }

    /* MIPI Camera -- open stream */
    ret = v4l2.streamOn();
    if(ret < 0)
        return ret;

    int res;
    while((res = getopt(argc, argv, "m:i:hd:")) != -1)
    {
        switch(res)
        {
            case 'm':
                model_file = optarg;
                break;
            case 'i':
                image_file = optarg;
                break;
            case 'd':
                device = optarg;
                break;
            case 'h':
                show_usage();
                return 0;
            default:
                break;
        }
    }

    if( model_file.empty() )
    {
        std::cerr << "Error: model file not specified!" << std::endl;
        show_usage();
        return -1;
    }

    if(image_file.empty())
    {
        std::cerr << "Error: image file not specified!" << std::endl;
        show_usage();
        return -1;
    }

    tengine::Net somenet;
    tengine::Tensor input_tensor;
    tengine::Tensor output_tensor;
	
    if(request_tengine_version("0.9") != 1)
    {
        std::cout << " request tengine version failed\n";
        return 1;
    }
    // check file
    if((!check_file_exist(model_file) or !check_file_exist(image_file)))
    {
        return 1;
    }

    /* load model */
    somenet.load_model(NULL, "tengine", model_file.c_str());

    // input
    int img_h = 300;
    int img_w = 300;
    int img_size = img_h * img_w * 3;

    float mean[3] = {127.5, 127.5, 127.5};
	const float* _channel_mean = mean;
    float scales = 0.007843;

    int repeat_count = 1;
    const char* repeat = std::getenv("REPEAT_COUNT");

    if(repeat)
        repeat_count = std::strtoul(repeat, NULL, 10);

    /* prepare input data */
    input_tensor.create(img_w, img_h, 3);

for(;;)
	{
	if(ret = v4l2.readFrame(V4L2_PIX_FMT_RGB24,image) < 0)
            return ret;
    if(image.empty())
    {
            std::cout << "End of video stream" << std::endl;
            break;
    }	
    get_input_data_frame(image, (float* )input_tensor.data, img_h, img_w, _channel_mean, scales);

    /* forward */
    somenet.input_tensor(0, 0, input_tensor);

    struct timeval t0, t1;
    float total_time = 0.f;
    float min_time = __DBL_MAX__;
    float max_time = -__DBL_MAX__;
    for(int i = 0; i < repeat_count; i++)
    {
        gettimeofday(&t0, NULL);
        somenet.run();
        gettimeofday(&t1, NULL);
        float mytime = ( float )((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec)) / 1000;
        total_time += mytime;
        min_time = std::min(min_time, mytime);
        max_time = std::max(max_time, mytime);
    }
    std::cout << "--------------------------------------\n";
    std::cout << "\nRepeat " << repeat_count << " times, avg time per run is " << total_time / repeat_count << " ms\n" << "max time is " << max_time << " ms, min time is " << min_time << " ms\n";

    /* get result */
    somenet.extract_tensor(0, 0, output_tensor);

    /* after process */

	//fcv::imshow("test", image, NULL);
    float* outdata = ( float* )(output_tensor.data);
    float show_threshold = 0.5;

    post_process_ssd_frame(image, show_threshold, outdata, output_tensor.h);
        // Show each frame
    fcv::imshow(wintitle, image,NULL);
    /* Exit when Esc is detected */
    char c = ( char )fcv::waitKey(30);
    if(c == 27)
        break;


    /* Exit when Esc is detected */


	}

 //   get_input_data(image_file.c_str(), (float* )input_tensor.data, img_h, img_w, mean, scales);

    /* forward */
    //somenet.input_tensor(0, 0, input_tensor);


    //tensor_t out_tensor = get_graph_output_tensor(graph, 0, 0);    //"detection_out");
    /* get result */
    //somenet.extract_tensor(0, 0, output_tensor);



    return 0;
}