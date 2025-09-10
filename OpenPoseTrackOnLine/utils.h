//
// Created by gaoyiao on 2025/7/16.
//

#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <cstdint>
#include <vector>
#include <unistd.h>
#include <string>

constexpr int CHUNKSIZE = 1024;
constexpr int MAX_CONNECTS = 10;

std::vector<uint8_t> _32To8(uint32_t index);

uint32_t _8To32(std::vector<uint8_t>& index_vec);

int setNonBlocking(int fd);




// 全局变量只在此声明
extern std::string savepath;
extern std::string protoFile;
extern std::string weightsFile;
extern const int nPoints;

#define MPT
#define PARALLEL_CALLBACK

#ifdef COCO
extern const int POSE_PAIRS[17][2];
#elif defined(MPT)
extern const int POSE_PAIRS[14][2];
#endif


#endif //SERVER_UTILS_H
