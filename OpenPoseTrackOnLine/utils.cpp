//
// Created by gaoyiao on 2025/7/16.
//

#include "utils.h"
#include <fcntl.h>

std::vector<uint8_t> _32To8(uint32_t index) {

    uint8_t highest = index >> 24;
    uint8_t second = (index >> 16) & 0xFF;
    uint8_t third = (index >> 8) & 0xFF;
    uint8_t lowest = index & 0xFF;

    std::vector<uint8_t> buffer{highest, second, third, lowest};

    return buffer;
}

uint32_t _8To32(std::vector<uint8_t>& vec) {

    uint32_t data = ((uint32_t)vec[0] << 24) |
                    ((uint32_t)vec[1] << 16) |
                    ((uint32_t)vec[2] << 8) |
                    (uint32_t)vec[3];

    return data;
}

int setNonBlocking(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

std::string savepath = "/home/gaoyiao/C_CPP/study/github/learnopencv/openposetrack/images";

#ifdef COCO

const int POSE_PAIRS[17][2] = {
    {1, 2}, {1, 5}, {2, 3},
    {3, 4}, {5, 6}, {6, 7},
    {1, 8}, {8, 9}, {9, 10},
    {1, 11}, {11, 12}, {12, 13},
    {1, 0}, {0, 14}, {14, 16},
    {0, 15}, {15, 17}
};

std::string protoFile = "/home/gaoyiao/C_CPP/learnopencv/OpenPose/pose/coco/pose_deploy_linevec.prototxt";
std::string weightsFile = "/home/gaoyiao/C_CPP/learnopencv/OpenPose/pose/coco/pose_iter_440000.caffemodel";
const int nPoints = 18;

#elif defined(MPT)

const int POSE_PAIRS[14][2] = {
    {0, 1}, {1, 2}, {2, 3},
    {3, 4}, {1, 5}, {5, 6},
    {6, 7}, {1, 14}, {14, 8}, {8, 9},
    {9, 10}, {14, 11}, {11, 12}, {12, 13}
};

std::string protoFile = "/home/gaoyiao/C_CPP/learnopencv/OpenPose/pose/mpi/pose_deploy_linevec_faster_4_stages.prototxt";
std::string weightsFile = "/home/gaoyiao/C_CPP/learnopencv/OpenPose/pose/mpi/pose_iter_160000.caffemodel";
const int nPoints = 15;

#endif