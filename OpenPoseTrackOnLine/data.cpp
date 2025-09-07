//
// Created by gaoyiao on 2025/7/14.
//

#include <cassert>

#include "data.h"
#include "utils.h"

ImageData::ImageData(int id, cv::Mat image) :
    m_id(id),
    image(image),
    rows(image.rows),
    cols(image.cols){

}

ImageData::~ImageData(){

}


std::vector<uint8_t> ImageData::encode() {

    std::vector<uint8_t> id_vec = _32To8(m_id);
    std::vector<uint8_t> rows_8 = _32To8(rows);
    std::vector<uint8_t> cols_8 = _32To8(cols);

    cv::Mat flat = image.reshape(1, 1);

    std::vector<uint8_t> image_vec(flat.begin<uchar>(), flat.end<uchar>());

    std::vector<uint8_t> buffer;
    buffer.reserve(id_vec.size() +
                   rows_8.size() +
                   cols_8.size() +
                   image_vec.size());

    buffer.insert(buffer.end(), id_vec.begin(), id_vec.end());
    buffer.insert(buffer.end(), rows_8.begin(), rows_8.end());
    buffer.insert(buffer.end(), cols_8.begin(), cols_8.end());
    buffer.insert(buffer.end(), image_vec.begin(), image_vec.end());
    return buffer;
}


ImageData ImageData::decode(std::vector<uint8_t>& image_vec) {
    std::vector<uint8_t> id_vec(image_vec.begin(), image_vec.begin() + 4);
    std::vector<uint8_t> rows_vec(image_vec.begin()+4, image_vec.begin()+8);
    std::vector<uint8_t> cols_vec(image_vec.begin()+8, image_vec.begin()+12);
    std::vector<uint8_t> mat_vec(image_vec.begin()+12, image_vec.end());

    uint32_t id = _8To32(id_vec);
    uint32_t rows = _8To32(rows_vec);
    uint32_t cols = _8To32(cols_vec);
    assert(rows * cols * 3 == mat_vec.size());
    cv::Mat image = cv::Mat(rows, cols, CV_8UC3);
    std::memcpy(image.data, mat_vec.data(), mat_vec.size());
    ImageData myImage(id, image);

    return myImage;
}