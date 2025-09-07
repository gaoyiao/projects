#include "client.h"

int Client::createConnect(){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(Address.c_str());
    server_addr.sin_port = htons(port);

    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Connect Failed)");
        return -1;
    }

    return 1;
}

void Client::dataGenerator(){
    std::string file{};
    cv::Mat image;
    int count = 0;
    std::vector<uint8_t> data_vec;
    for(const auto& entry : fs::directory_iterator(homepath)){
        if(!running.load()) return;
        if(entry.is_regular_file()) file = entry.path().string();
        image = cv::imread(file);
        ImageData myImage(count++, image);
        data_vec = myImage.encode();
        std::unique_lock<std::mutex> m_lock(m_mutex);
        m_queue.emplace(data_vec);
        m_condition_lock.notify_one();
    }
}

void Client::readFromCameraAndDataGenerator(){
    
    cv::VideoCapture cap(camera_url);
    int frame_index{0};

    cv::Mat image;
    cv::Mat image_clone;
    std::vector<uint8_t> data_vec;

    if(!cap.isOpened()){
        std::cout << "Failed on Open Camera" << std::endl;
        return;
    }


    while(true){
        cap >> image;
        if(image.empty()){
            std::cout << "Frame Video frames Error!" << std::endl;
            break;
        }

        image_clone = image.clone();
        if(cv::waitKey(1) == 'q') return;
        cv::imshow("camera", image_clone);
        ImageData myImage(frame_index++, image_clone);
        data_vec = myImage.encode();
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(data_vec);
        m_condition_lock.notify_one();
    }
}




void Client::sendData(){
    std::vector<uint8_t> data_vec;
    std::vector<uint8_t> buffer(CHUNKSIZE, 0);
    std::vector<uint8_t> header_buffer(4, 0);
    uint32_t sended_size{};
    uint32_t chunk_size{};
    int count = 0;

    while(true){
        if(!running.load()) return;
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_queue.empty()) m_condition_lock.wait(lock);
        data_vec = std::move(m_queue.front());
        m_queue.pop();

        sended_size = 0;
        chunk_size = 0;


        header_buffer = _32To8(data_vec.size());
        std::cout << "send header size is " << data_vec.size() << std::endl;
        send(sockfd, header_buffer.data(), header_buffer.size(), 0);

        while(sended_size < data_vec.size()){
            if(data_vec.size() - sended_size < CHUNKSIZE) {
                chunk_size = data_vec.size() - sended_size;
                buffer.clear();
            }
            else
                chunk_size = CHUNKSIZE;
            std::memcpy(buffer.data(), data_vec.data() + sended_size, chunk_size);
            send(sockfd, buffer.data(), chunk_size, 0);
            sended_size += chunk_size;
        }
        std::cout << count++ << "already sended!  " << "The Size of Sended data is " << sended_size << std::endl;
    }
}

void Client::recvMessage(){
    
    while(true){
        int meslen = recv(sockfd, buf, 1024, 0);
        if(meslen > 0){
            std::cout << buf << std::endl;
            running.store(false);
            return;
        }
    }
}

    
