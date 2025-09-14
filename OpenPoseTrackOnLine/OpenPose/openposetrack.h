//
// Created by 高一奥 on 2025/6/8.
//

#ifndef OPENPOSETRACK_OPENPOSETRACK_H
#define OPENPOSETRACK_OPENPOSETRACK_H

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "../thread_pool.h"
#include "../utils.h"
#include "../processor.h"

class OpenPoseTrack : public IProcessor
{
public:
    OpenPoseTrack(int h, int w, int tnum, float th, std::string device, std::string camurl);
    void handleRawImages(Server& server, int sock);
    void shows();
    ~OpenPoseTrack();

    void printDatasSize(){
        std::cout << raw_images.size() << std::endl;
    }

    cv::Mat handle(cv::Mat image, cv::dnn::Net net);

    std::vector<std::future<cv::Mat>> printOutput(){
        return std::move(keyPointfutures);
    }

    void process(Server& server, int sock) override {
        handleRawImages(server, sock);
    }

    void show() override;

private:
    int inputNetHeight;
    int inputNetWidth;
    int thread_nums;
    float threshold;
    std::string deviceName;
    std::string camera_url;


    //threadpool
    ThreadPool pool;

    //data stream
    std::deque<cv::Mat> raw_images;
    std::deque<cv::Mat> keypoint_images;
    std::deque<cv::Mat> show_images;
    std::vector<std::future<cv::Mat>> keyPointfutures;
    std::vector<cv::dnn::Net> Nets;

    //mutex
    std::mutex lock_raw;
    std::mutex lock_keypoint;
    std::mutex lock_shows;

    std::condition_variable m_condition_lock;

    std::mutex m_lock_handled;
    std::condition_variable m_condition_handled;
    std::queue<cv::Mat> m_handled_images;

    //flag
    std::atomic<bool> isRuning{true};

    //Nets
    //params
    //cv::dnn::Net net;


    //function
    cv::dnn::Net generateNet();
    cv::Mat transInputSize(cv::Mat frame);
    std::vector<cv::Point2f>  keyPointsDetection(cv::Mat netOutput, cv::Mat oriInput);
    cv::Mat generateKeypointMap(cv::Mat image, std::vector<cv::Point2f> keyPoints);
    void netInit();

    void callBack(cv::Mat& image, int count);
};

#endif //OPENPOSETRACK_OPENPOSETRACK_H
