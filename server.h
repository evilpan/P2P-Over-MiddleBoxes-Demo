#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::udp;

class udp_server
{
public:
    udp_server(boost::asio::io_service &io_service):
        _sock(io_service, udp::endpoint(udp::v4(), 2333))
    {
        //boost::thread(boost::bind(&udp_server::session_send, this));
        session_receive();
    }
    void session_receive();
    void handle_receive(const boost::system::error_code &ec, std::size_t len);
    void session_send(udp::endpoint &ep, std::string write_message);
    void handle_send(const boost::system::error_code &ec, std::size_t len);

private:
    udp::socket _sock;
    boost::array<char, 256> _recv_buffer;
    std::string _write_message;
    std::list<udp::endpoint> _endpoint_list;
    udp::endpoint _remote_endpoint;//current endpoint
};

void udp_server::session_receive()
{
    _sock.async_receive_from(
            boost::asio::buffer(_recv_buffer),
            _remote_endpoint,
            boost::bind(&udp_server::handle_receive,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}

void udp_server::handle_receive(const boost::system::error_code &ec, std::size_t len)
{
    std::string receive_message(_recv_buffer.data(), len);
    if(strcmp(receive_message.c_str(), "login")==0)
    {
        int isLogged = 0;
        for(std::list<udp::endpoint>::iterator iter=_endpoint_list.begin(); iter!=_endpoint_list.end(); ++iter)
        {
            if(*iter==_remote_endpoint)
            {
                session_send(_remote_endpoint, "You have already logged in.\n");
                isLogged = 1;
                break;
            }
        }
        if(!isLogged)
        {
            std::cout << "User login.\nAddress : " << _remote_endpoint.address().to_string() << std::endl;
            std::cout << "Port : " << _remote_endpoint.port() << std::endl;
            _endpoint_list.push_back(_remote_endpoint);
            session_send(_remote_endpoint, "login success.\n");
        }
    }
    else if(strcmp(receive_message.c_str(), "logout")==0)
    {
        int isLogged = 0;
        for(std::list<udp::endpoint>::iterator iter=_endpoint_list.begin(); iter!=_endpoint_list.end(); ++iter)
        {
            if(*iter == _remote_endpoint)
            {
                isLogged = 1;
                _endpoint_list.erase(iter);
                std::cout << "User logout.\nAddress : " << _remote_endpoint.address().to_string() << std::endl;
                std::cout << "Port : " << _remote_endpoint.port() << std::endl;
                session_send(_remote_endpoint, "Logout success.\n");
                break;
            }
        }
        if(!isLogged)
            session_send(_remote_endpoint, "Logout failed, you have not logged in.\n");
    }
    else if(strcmp(receive_message.c_str(), "list")==0)
    {
        std::ostringstream message;
        int i = 0;
        for(std::list<udp::endpoint>::iterator iter=_endpoint_list.begin(); iter!=_endpoint_list.end(); ++iter)
        {
            if(*iter == _remote_endpoint)
                message << "[" << i << "]" << iter->address().to_string() << ":" << iter->port() << " (yourself)" << std::endl;
            else
                message << "[" << i << "]" << iter->address().to_string() << ":" << iter->port() << std::endl;
            i++;
        }
        session_send(_remote_endpoint, message.str());
    }
    else if(strncmp(receive_message.c_str(), "punch", 5)==0)
    {
        int punched_client = atoi(receive_message.c_str() + 6);
        std::list<udp::endpoint>::iterator iter = _endpoint_list.begin();
        for(int i=0; i < punched_client && iter!=_endpoint_list.end() ; ++i, ++iter)
            ;
        std::ostringstream message;
        if(iter == _endpoint_list.end())
            message << "Punch failed, no such client.";
        else
        {
            std::ostringstream peer_message;
            //udp::endpoint peer_endpoint(iter->address(), iter->port);
            peer_message << "PUNCH_REQUEST " << _remote_endpoint.address().to_string() << ":" << _remote_endpoint.port() << std::endl;
            session_send(*iter, peer_message.str());

            message << "PUNCH_SUCCESS " << iter->address().to_string() << ":" << iter->port() << std::endl;
        }
        session_send(_remote_endpoint, message.str());
    }
    else if(strcmp(receive_message.c_str(), "help")==0)
    {
        session_send(_remote_endpoint, "Commands:\
                \n\thelp : Show this infomation.\
                \n\tlogin : Login p2p server to make you punchable.\
                \n\tlogout : Logout p2p server so that other client(s) won't find you.\
                \n\tlist: List all client(s) that have logined.\
                \n\tpunch <client_number>: send punch request to remote client and start a p2p session\n");
    }
    else
    {
        session_send(_remote_endpoint, "Unknown command, please type 'help' to see more options.\n");
    }

    session_receive();
}

void udp_server::session_send(udp::endpoint &ep, std::string write_message)
{
    //std::getline(std::cin, _write_message);
    _sock.async_send_to(
            boost::asio::buffer(write_message),
            ep,
            boost::bind(&udp_server::handle_send,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}

void udp_server::handle_send(const boost::system::error_code &ec, std::size_t len)
{
    //session_send();
}
