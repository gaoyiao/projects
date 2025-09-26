//
// Created by gaoyiao on 2025/7/26.
//

#ifndef LEARNOPENCV_SERVER_H
#define LEARNOPENCV_SERVER_H

#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <queue>
#include <unordered_map>
#include <sys/epoll.h>

#include <mutex>
#include <condition_variable>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../utils.h"
#include "../data.h"
#include "../thread_pool.h"
#include "../OpenPose/openposetrack.h"
//#include "../processor.h"

class IProcessor;

struct recvInfo{
    std::vector<uint8_t> head_buffer;
    std::vector<uint8_t> buffer;

    std::vector<uint8_t> header;
    std::vector<uint8_t> dataTemp;

    uint32_t needSize{0};
    uint32_t recvedSize{0};
    uint32_t chunksize{0};

    uint32_t head_recv_length{0};
    int curRecvHeadSize{0};
    int curRecvSize{0};

    recvInfo() :
        head_buffer(4, 0),
        buffer(1024, 0){

    }


    void clear(){
        needSize = 0;
        recvedSize = 0;
        head_recv_length = 0;
        curRecvHeadSize = 0;
        curRecvSize = 0;

        header.clear();
        dataTemp.clear();
    }
};

class Server
{
private:
    //server info
    int server_sock{}, client_sock{};
    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t client_len = sizeof(client_addr);
    const std::string msg = "The number of connections has exceeded the maximum";

    int epfd{};
    epoll_event ev{}, events[MAX_CONNECTS];

    std::string server_addr_string{};
    int port{};

    //std::queue<std::vector<uint8_t>> m_queue; //接收来自Client发送的数据，并拼接成完整的vector
    //std::queue<cv::Mat> m_queue_mat;

    /*
    TODO: 
        是否将下面使用结构体放在一起更加高效，易读  by gaoyiao 2025/9/26 10:35
    */

    std::unordered_map<int, std::queue<std::vector<uint8_t>>> m_queues;
    std::unordered_map<int ,std::queue<cv::Mat>> m_queue_mats;
    std::unordered_map<int, std::unique_ptr<std::mutex>> m_queue_mats_mutexs;
    std::unordered_map<int, std::unique_ptr<std::condition_variable>> m_queue_mats_conditions;

    //recvInfo recvinfo;
    uint32_t headLength{4};

//    std::mutex m_mutex_vec;
//    std::condition_variable m_condition_lock;

    std::unordered_map<int, recvInfo> m_recvInfos;
    std::unordered_map<int, std::unique_ptr<std::mutex>> m_recv_mutexs;
    std::unordered_map<int, std::unique_ptr<std::condition_variable>> m_recv_conditions;
    std::unordered_map<int, bool> m_recv_flags;
    std::unordered_map<int, std::atomic<int>> m_recv_counts;

    ThreadPool m_pool;
    ThreadPool m_recv_pool;

    std::atomic<int> conneNums{0};
    std::vector<std::thread> recvThreads;
    std::vector<std::thread> openposeThreads;

    IProcessor* iprocess;

public:

    Server(std::string s_addr, int p, IProcessor* iproc) :
        server_addr_string(s_addr),
        port(p),
        m_pool(MAX_CONNECTS),
        m_recv_pool(MAX_CONNECTS),
        iprocess(iproc){
    }

    ~Server(){
        m_pool.shutdown();
        m_recv_pool.shutdown();
        for(auto& recvt : recvThreads) if(recvt.joinable()) recvt.join();
        for(auto& openposet : openposeThreads) if(openposet.joinable()) openposet.join();
    }

    int createConnection();

    // 接收，并将uint8_t buffer拼接成完整的vector
    void recvFromClient(int sock);
    void disPatcher();

    //将vector解码，还原数据结构, 帧号id， Mat
    void vecToMat(int sock, std::vector<uint8_t>&& data_vec);
    void getData(int sock);

    std::queue<cv::Mat>& getMats(int sock) { return m_queue_mats[sock];}

    std::unique_ptr<std::mutex>& getMatLock(int sock) {
        return m_queue_mats_mutexs[sock];
    }

    std::unique_ptr<std::condition_variable>& getMatConditions(int sock){
        return m_queue_mats_conditions[sock];
    }

    void realTimeShow();

};

#endif //LEARNOPENCV_SERVER_H

