//
// Created by gaoyiao on 2025/7/24.
//

//#include "client.h"
#include "server.h"

int main()
{
    std::string Addr = "192.168.10.87";
    //std::string homepath = "/home/gaoyiao/C_CPP/resign/learnopencv/server/images_back/images/";
    int port = 8888;

    std::string camUrl = "http://192.168.10.72:8080/video";
#ifdef PARALLEL_CALLBACK
    cv::setNumThreads(1);
#endif
    //OpenPoseTrack opt(368, 368, 12, 0.1, "GPU", camUrl);
    //Client client(Addr, port, homepath);

    /*
    int flag = client.createConnect();

    std::thread t1(&Client::dataGenerator, &client);
    std::thread t2(&Client::sendData, &client);

    t1.join();
    t2.join();
    */

    auto iproc = std::make_unique<OpenPoseTrack>(368, 368, 12, 0.1, "GPU", camUrl);
    
    Server server(Addr, port, std::move(iproc.get()));
    int flag = server.createConnection();
    server.disPatcher();
    
    //std::thread t1(&Server::disPatcher, &server);
    //std::thread t2(&Server::getData, &server);

    //t1.join();
    //t2.join();
    //std::cout << flag << std::endl;
    return 0;

}