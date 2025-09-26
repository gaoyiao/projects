//
// Created by gaoyiao on 2025/7/26.
//

#include "server.h"
#include "../processor.h"

int Server::createConnection() {

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1){
        perror("socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_addr_string.c_str());
    server_addr.sin_port = htons(port);

    if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("bind failed");
        return -1;
    }

    listen(server_sock, MAX_CONNECTS);
    setNonBlocking(server_sock);
    std::cout << "Server is listening" << std::endl;
    
    epfd = epoll_create1(0);
    ev.data.fd = server_sock;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &ev);
    
    return 1;
}


void Server::recvFromClient(int sock) {
    while(true){
        if(m_recvInfos[sock].needSize == 0){
            while(m_recvInfos[sock].head_recv_length < headLength){
                m_recvInfos[sock].curRecvHeadSize = recv(sock,
                                                m_recvInfos[sock].head_buffer.data(),
                                                headLength - m_recvInfos[sock].head_recv_length,
                                                0);
                if(m_recvInfos[sock].curRecvHeadSize < 0) continue;
                m_recvInfos[sock].header.insert(m_recvInfos[sock].header.end(), m_recvInfos[sock].head_buffer.begin(), m_recvInfos[sock].head_buffer.begin()+m_recvInfos[sock].curRecvHeadSize);
                m_recvInfos[sock].head_recv_length += m_recvInfos[sock].curRecvHeadSize;
            }
            //std::cout << m_recvInfos[sock].header.size() << std::endl;
            m_recvInfos[sock].needSize = _8To32(m_recvInfos[sock].header);
        }
        else{
            while(m_recvInfos[sock].recvedSize < m_recvInfos[sock].needSize){
                if(m_recvInfos[sock].needSize - m_recvInfos[sock].recvedSize >= CHUNKSIZE) m_recvInfos[sock].chunksize = CHUNKSIZE;
                else m_recvInfos[sock].chunksize = m_recvInfos[sock].needSize - m_recvInfos[sock].recvedSize;

                m_recvInfos[sock].curRecvSize = recv(sock,
                                            m_recvInfos[sock].buffer.data(),
                                            m_recvInfos[sock].chunksize,
                                            0);
                if(m_recvInfos[sock].curRecvSize < 0) continue;
                m_recvInfos[sock].dataTemp.insert(m_recvInfos[sock].dataTemp.end(), m_recvInfos[sock].buffer.begin(), m_recvInfos[sock].buffer.begin()+m_recvInfos[sock].curRecvSize);

                m_recvInfos[sock].recvedSize += m_recvInfos[sock].curRecvSize;
            }
            {
                std::unique_lock<std::mutex> lock(*m_recv_mutexs[sock]);
                m_queues[sock].emplace(std::move(m_recvInfos[sock].dataTemp));
                m_recv_conditions[sock]->notify_one();
            }
            m_recvInfos[sock].clear();
        }
        //std::cout << m_recvInfos[sock].recvedSize << ", " << m_recvInfos[sock].needSize << std::endl;

        //std::cout << "m_queue size is:  " << m_queue.size() << std::endl;
    }
}

/*
void Server::getData() {
    std::vector<uint8_t> data_vec;
    cv::Mat image;
    int count = 0;
    while(true){
        {
            std::unique_lock<std::mutex> lock(m_mutex_vec);
            if(m_queue.empty()) m_condition_lock.wait(lock);
            data_vec = std::move(m_queue.front());
            m_queue.pop();
            ImageData myData = ImageData::decode(data_vec);
            image = myData.getImage().clone();
            cv::imwrite("../saveimages/" + std::to_string(count++) + ".jpg", image);
            //m_queue_mat.emplace(std::move(myData.getImage()));
            //std::cout << "m_queue_mat size is " << m_queue_mat.size() << std::endl;
        }
    }
}
*/

void Server::vecToMat(int sock, std::vector<uint8_t>&& data_vec){
    cv::Mat image;
    ImageData myData = ImageData::decode(data_vec);
    image = myData.getImage().clone();
    {
        std::unique_lock<std::mutex> lock_mat(*m_queue_mats_mutexs[sock]);
        m_queue_mats[sock].emplace(image);
        m_queue_mats_conditions[sock]->notify_one();
    }
    

    cv::imwrite("../saveimages/" +
                 std::to_string(sock) +
                 "_" +
                 std::to_string(m_recv_counts[sock].load()) +
                 ".jpg", image);

    ++m_recv_counts[sock];
}

// void Server::getData() {

//     int sock{};
//     while(true){
//         {
//             std::unique_lock<std::mutex> lock(m_readysock_mutex);
//             m_readysock_condition.wait(lock, [this]() { return !readySocks.empty();});
//             if(readySocks.empty()) continue;
//             sock = std::move(readySocks.front());
//             readySocks.pop();
//             m_pool.submit_void([this, sock]() { vecToMat(sock);});
//         }
//     }
// }


void Server::getData(int sock){
    std::vector<uint8_t> data_vec;
    while(true){
        {
            std::unique_lock<std::mutex> lock(*m_recv_mutexs[sock]);
            m_recv_conditions[sock]->wait(lock, [this, sock]() {return !this->m_queues[sock].empty();});
            if(m_queues[sock].empty()) continue;
            data_vec = std::move(m_queues[sock].front());
            m_queues[sock].pop();
        }
        vecToMat(sock, std::move(data_vec));
    }
}


void Server::disPatcher() {
    while(true){
        int n = epoll_wait(epfd, events, MAX_CONNECTS, -1);
        for(int i = 0; i < n; ++i){
            if(events[i].data.fd == server_sock){
                int client_sock = accept(server_sock, nullptr, nullptr);
                if(conneNums.load() >= MAX_CONNECTS){
                    send(client_sock, msg.c_str(), msg.size(), 0);
                    close(client_sock);
                    continue;
                }
                epoll_event client_event;
                client_event.data.fd = client_sock;
                client_event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &client_event);
                setNonBlocking(client_sock);

                /* 
                TODO:
                    结构体统一初始化 written by gaoyiao 2025/9/26 10:39
                */
                m_recv_mutexs[client_sock] = std::make_unique<std::mutex>();
                m_recv_conditions[client_sock] = std::make_unique<std::condition_variable>();
                m_recvInfos[client_sock] = recvInfo();
                m_recv_flags[client_sock] = false;
                m_queues[client_sock] = std::queue<std::vector<uint8_t>>{};
                m_queue_mats[client_sock] = std::queue<cv::Mat>{};
                m_queue_mats_mutexs[client_sock] = std::make_unique<std::mutex>();
                m_queue_mats_conditions[client_sock] = std::make_unique<std::condition_variable>();
                m_recv_counts[client_sock].store(0);

                std::cout << "new Connection" << client_sock << std::endl;

                //每有一个新连接，就创建一个recv线程
                //m_recv_pool.submit_void([this, client_sock] () { this->getData(client_sock);});
                recvThreads.push_back(std::thread(&Server::getData, this, client_sock));
                openposeThreads.push_back(std::thread(&IProcessor::process, iprocess, std::ref(*this), client_sock));
                std::cout << openposeThreads.size() << std::endl;
                conneNums++;

            } else{
                if(!m_recv_flags[events[i].data.fd]){
                    m_recv_flags[events[i].data.fd] = true;
                    m_pool.submit_void([this, i]() { this-> recvFromClient(events[i].data.fd);});
                }else continue;
            }
        }
    }
}

void Server::realTimeShow() {
    iprocess->show();
}

