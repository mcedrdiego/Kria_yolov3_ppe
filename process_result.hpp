/*
 * Copyright 2019 Xilinx Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <string>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
/*
 *   The color loops every 27 times,
 *    because each channel of RGB loop in sequence of "0, 127, 254"
 */
// static cv::Scalar hsvToRgb(h, s, v){

static const char *labels[] = {"Worker", "Worker + Helmet", "Worker + Vest", "Worker + Helmet + Vest"};
static const cv::Scalar clrs[] = {cv::Scalar(183, 123, 21), cv::Scalar(127, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(127, 0, 255)};

static cv::Scalar getColor(int label)
{
  int c[3];
  for (int i = 1, j = 0; i <= 9; i *= 3, j++)
  {
    c[j] = ((label / i) % 3) * 127;
  }
  return cv::Scalar(c[2], c[1], c[0]);
}

static cv::Mat process_result(cv::Mat &image,
                              const vitis::ai::YOLOv3Result &result,
                              bool is_jpeg)
{
  for (int i = 0; i < result.bboxes.size(); i++)
  {
    bool is_duplicate = false;
    for (int j = i + 1; j < result.bboxes.size(); j++)
    {
      if (result.bboxes[i].x == result.bboxes[j].x && result.bboxes[i].y == result.bboxes[j].y){
        is_duplicate = true;
        break;
      }
        
    }
    if (!is_duplicate)
    {
      int label = result.bboxes[i].label;
      float xmin = result.bboxes[i].x * image.cols + 1;
      float ymin = result.bboxes[i].y * image.rows + 1;
      float xmax = xmin + result.bboxes[i].width * image.cols;
      float ymax = ymin + result.bboxes[i].height * image.rows;
      float confidence = result.bboxes[i].score;

      if (xmax > image.cols)
        xmax = image.cols;
      if (ymax > image.rows)
        ymax = image.rows;

      LOG_IF(INFO, is_jpeg) << "RESULT: " << label << "\t" << xmin << "\t" << ymin
                            << "\t" << xmax << "\t" << ymax << "\t" << confidence
                            << "\n";

      cv::rectangle(image, cv::Point(xmin, ymin), cv::Point(xmax, ymax), clrs[label], 2, 1, 0);
      cv::rectangle(image, cv::Point(xmin, ymin), cv::Point(xmax, ymin + 20), clrs[label], -1);
      cv::putText(image, labels[label], cv::Point(xmin, ymin + 15), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
      cv::putText(image, std::to_string(int(confidence * 100)) + " %", cv::Point(xmax - 50, ymin + 35), cv::FONT_HERSHEY_DUPLEX, 0.5, clrs[label]);
    }
  }
  if (is_jpeg)
    cv::imwrite("/home/petalinux/result.jpg", image);
  return image;
}