//
// Created by gaoyiao on 2025/7/24.
//

#include "client.h"
//#include "server.h"

int main()
{
    const std::string Addr = "192.168.10.87";
    const std::string homepath = "../../images/";
    const std::string camera_url = "http://192.168.10.72:8080/video";
    const int port = 8888;
    Client client(Addr, port, homepath, camera_url);


    int flag = client.createConnect();

    if(flag > 0){
        std::thread t1(&Client::readFromCameraAndDataGenerator, &client);
        std::thread t2(&Client::sendData, &client);
        std::thread t3(&Client::recvMessage, &client);

        t1.join();
        t2.join();
        t3.join();
    }
    return 0;

}
