//
// Created by gaoyiao on 2025/7/24.
//

//#include "client.h"
#include "server.h"

int main()
{
    std::string Addr = "127.0.0.1";
    //std::string homepath = "/home/gaoyiao/C_CPP/resign/learnopencv/server/images_back/images/";
    int port = 12346;
    //Client client(Addr, port, homepath);

    /*
    int flag = client.createConnect();

    std::thread t1(&Client::dataGenerator, &client);
    std::thread t2(&Client::sendData, &client);

    t1.join();
    t2.join();
    */

    Server server(Addr, port);
    int flag = server.createConnection();
    std::thread t1(&Server::recvFromClient, &server);
    std::thread t2(&Server::getData, &server);

    t1.join();
    t2.join();
    std::cout << flag << std::endl;
    return 0;

}