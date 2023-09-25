//
// Created by Дмитрий on 25.09.23.
//

#ifndef SERVER_SESSION_H
#define SERVER_SESSION_H
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <set>
#include <iostream>
#include <map>

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
using namespace boost::asio;
using ip::tcp;


class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::any_io_executor io_context, std::set<std::shared_ptr<Session>>& clients, ssl::context& ssl_context, std::map<std::string, std::vector<std::string>>& messages);
    void Start();
    void Read();
    void HandleNicknameRead(const boost::system::error_code& error, std::size_t bytes_transferred);
    void ReadMessage();
    void HandleMessageRead(const boost::system::error_code& error, std::size_t bytes_transferred);
    void BroadcastMessage(const std::string& message);
    void Disconnect();
    ssl::stream<tcp::socket>& socket();
private:
    ssl::stream<tcp::socket> socket_;
    std::set<std::shared_ptr<Session>>& clients_;
    boost::asio::streambuf input_buffer_;
    std::string nickname_;
    std::map<std::string, std::vector<std::string>>& messages_;
    enum { max_length = 1024 };
    char data_[max_length];
};


#endif //SERVER_SESSION_H
