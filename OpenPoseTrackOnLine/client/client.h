#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <filesystem>
#include <atomic>

#include <mutex>
#include <thread>
#include <condition_variable>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../utils.h"
#include "../data.h"

namespace fs = std::filesystem;

class Client
{
private:
    const std::string Address;
    const std::string camera_url{""};
    const int port;
    struct sockaddr_in server_addr{};
    int sockfd;
    std::string homepath;

    std::queue<std::vector<uint8_t>> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition_lock;
    char buf[1024];
    std::atomic<bool> running{true};

public:
    Client(const std::string addr, const int p, std::string hp, const std::string cam_url) :
        Address(addr),
        port(p),
        homepath(hp),
        camera_url(cam_url){
    }

    int createConnect();

    void dataGenerator();

    void sendData();

    void recvMessage();

    void readFromCameraAndDataGenerator();


};

#endif
