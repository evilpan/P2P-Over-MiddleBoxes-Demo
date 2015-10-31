#include "client.h"
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
      std::cerr << "Usage: client <Server IP> <Server Port>" << std::endl;
      return 1;
    }

    boost::asio::io_service io_service;
    udp_client client(io_service, argv[1], argv[2]);
    //run in 2 threads
    //boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));
    io_service.run();


    return 0;
}
