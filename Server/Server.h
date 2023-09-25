//
// Created by Дмитрий on 25.09.23.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H
#include "Session.h"

class Server {
public:
    Server(boost::asio::io_context& io_context, unsigned short port);
private:
    tcp::acceptor acceptor_;
    ssl::context ssl_context_;
    std::set<std::shared_ptr<Session>> clients_;
    std::map<std::string, std::vector<std::string>> messages_;

    void StartAccept();

};


#endif //SERVER_SERVER_H
