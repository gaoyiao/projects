//
// Created by gaoyiao on 2025/7/14.
//

#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include <vector>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class ImageData{
private:
    uint32_t m_id;
    cv::Mat image;
    const uint32_t rows;
    const uint32_t cols;

public:
    ImageData(int id, cv::Mat image);
    ~ImageData();
    std::vector<uint8_t> encode();

    uint32_t getId() { return m_id; }
    cv::Mat getImage() { return image; }

//    std::vector<uint8_t> _32To8(uint32_t index);
//    uint32_t _8To32(std::vector<uint8_t> vec);

    static ImageData decode(std::vector<uint8_t>& image_vec);
};

#endif //SERVER_DATA_H
