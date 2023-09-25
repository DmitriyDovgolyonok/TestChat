//
// Created by Дмитрий on 25.09.23.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>


using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
using namespace boost::asio;
using ip::tcp;

class Client {
public:
    Client(boost::asio::io_context& io_context, const std::string& server_host, unsigned short server_port);
    void ReplaceSmiles(std::string& message);
    void Connect();
    void Send();
    void ReadMessages(ssl::stream<tcp::socket>& socket);

private:
    boost::asio::io_context& io_context_;
    ssl::context ssl_context_;
    ssl::stream<tcp::socket> socket_;
    std::string server_host_;
    unsigned short server_port_;
    std::string client_name_;
};


#endif //CLIENT_CLIENT_H
