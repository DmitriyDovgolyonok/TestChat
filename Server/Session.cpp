//
// Created by Дмитрий on 25.09.23.
//

#include "Session.h"

Session::Session(boost::asio::any_io_executor io_context, std::set<std::shared_ptr<Session>>& clients, ssl::context& ssl_context, std::map<std::string, std::vector<std::string>>& messages) :
        socket_(io_context, ssl_context),
        clients_(clients),
        messages_(messages)
{

}

void Session::Start()
{
    socket_.async_handshake(ssl::stream_base::server,
                            [self = shared_from_this()](const boost::system::error_code& error) {
                                if (!error)
                                {
                                    self->clients_.insert(self);
                                    self->Read();
                                }
                            });
}

void Session::Read() {
    async_read_until(socket_,
                     input_buffer_,
                     '\n',
                     boost::bind(&Session::HandleNicknameRead,
                                 shared_from_this(),boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred));
}

void Session::HandleNicknameRead(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(!error)
    {
        nickname_ = std::string(boost::asio::buffers_begin(input_buffer_.data()),
                                boost::asio::buffers_begin(input_buffer_.data()) + bytes_transferred - 1);
        input_buffer_.consume(bytes_transferred);
        std::cout << nickname_ + " has joined the chat.\n";
        BroadcastMessage(nickname_ + " has joined the chat.\n");
        ReadMessage();
    }
}
void Session::ReadMessage()
{
    async_read_until(socket_, input_buffer_, '\n',
                     boost::bind(&Session::HandleMessageRead, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Session::HandleMessageRead(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (!error)
    {
        std::string message(boost::asio::buffers_begin(input_buffer_.data()),
                            boost::asio::buffers_begin(input_buffer_.data()) + bytes_transferred - 1);
        input_buffer_.consume(bytes_transferred);

        if (!nickname_.empty())
            messages_[nickname_].push_back(message);

        if (message == "/list")
        {
            std::string userList;
            for(const auto& client : clients_)
            {
                userList += client->nickname_ + "\n";
            }
            boost::asio::write(socket_, boost::asio::buffer(userList));
        }
        else if (message.substr(0, 8) == "/search ")
        {
            std::string targetUser = message.substr(8);
            if(messages_.count(targetUser) > 0)
            {
                for(const std::string& userMessage : messages_[targetUser])
                {
                    boost::asio::write(socket_, boost::asio::buffer(targetUser + ": " + userMessage + '\n'));
                }
            }
        }
        else if (message.substr(0, 5) == "/p2p ")
        {
            std::string command = message.substr(5);
            size_t spacePos = command.find(' ');

            if(spacePos != std::string::npos)
            {
                std::string targetUser = command.substr(0, spacePos);
                std::string p2pMessage = command.substr(spacePos + 1);

                if(targetUser == nickname_)
                    boost::asio::write(socket_, boost::asio::buffer("You cannot send a message to yourself.\n"));
                else
                {
                    bool found = false;
                    for(auto& client : clients_)
                    {
                        if(client->nickname_ == targetUser)
                        {
                            boost::asio::write(client->socket_, boost::asio::buffer(nickname_ + " (P2P): " + p2pMessage + '\n'));
                            found = true;
                            break;
                        }
                    }
                    if(!found)
                        boost::asio::write(socket_, boost::asio::buffer("User " + targetUser + " not found.\n"));
                }
            }
            else
                boost::asio::write(socket_, boost::asio::buffer("Invalid /p2p command format. Usage: /p2p {name} {message}\n"));
        }
        else if (message == "/quit")
        {
            Disconnect();
        }
        else
            BroadcastMessage(nickname_ + ": " + message + '\n');

        ReadMessage();
    }
}
void Session::BroadcastMessage(const std::string& message)
{
    for (auto& client : clients_)
    {
        if (client != shared_from_this())
            boost::asio::write(client->socket_, boost::asio::buffer(message));
    }
}
void Session::Disconnect()
{
    std::cout << nickname_ << " is disconnecting...\n";
    try {
        socket_.shutdown();
    } catch (const std::exception& e)
    {
        std::cerr << "Error during socket shutdown: " << e.what() << std::endl;
    }
    BroadcastMessage(nickname_ + " has left the chat.\n");
    clients_.erase(shared_from_this());
}

ssl::stream<tcp::socket>& Session::socket() {
    return socket_;
}