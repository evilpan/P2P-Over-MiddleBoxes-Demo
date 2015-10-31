#include "server.h"

int main(int argc, char *argv[])
{
    boost::asio::io_service io_service;
    udp_server server(io_service);
    io_service.run();
    return 0;
}
