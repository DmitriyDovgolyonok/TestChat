//
// Created by Дмитрий on 25.09.23.
//

#include "Client.h"
#include <mutex>
#include <map>
std::mutex socketMutex;

std::map<std::string, std::string> smileys = {
        {":)", "😀"},
        {":(", "😞"} //Смайлов можно добавить
};

Client::Client(boost::asio::io_context& io_context, const std::string& server_host, unsigned short server_port) :
    io_context_(io_context),
    ssl_context_(ssl::context::sslv23_client),
    socket_(io_context, ssl_context_),
    server_host_(server_host),
    server_port_(server_port),
    client_name_("")
{

}

void Client::ReplaceSmiles(std::string& message)
{
    for(const auto& entry : smileys)
    {
        size_t pos = 0;
        while ((pos = message.find(entry.first, pos)) != std::string::npos)
        {
            message.replace(pos, entry.first.length(), entry.second);
            pos += entry.second.length();
        }
    }
}

void Client::Connect()
{
    tcp::resolver resolver(io_context_);
    tcp::resolver::results_type endpoints = resolver.resolve(server_host_, std::to_string(server_port_));

    boost::asio::connect(socket_.lowest_layer(), endpoints);

    socket_.handshake(ssl::stream_base::client);
}

void Client::Send()
{
    std::cout << "Enter your name: ";
    std::getline(std::cin, client_name_);
    {
        std::lock_guard<std::mutex> lock(socketMutex);
        boost::asio::write(socket_, boost::asio::buffer(client_name_ + "\n"));
    }


    std::thread([&]() {
        ReadMessages(socket_);
    }).detach();

    while (true)
    {
        std::string input;
        std::getline(std::cin, input);
        if (input== "/quit")
        {
            {
                std::lock_guard<std::mutex> lock(socketMutex);
                boost::asio::write(socket_, boost::asio::buffer("/quit\n"));
            }

            socket_.async_shutdown([this](const boost::system::error_code& error) {
                if (!error)
                {
                    boost::system::error_code closeError;
                    socket_.lowest_layer().close(closeError);

                    if (!closeError)
                        io_context_.stop();
                    else
                        std::cerr << "Error while closing socket: " << closeError.message() << std::endl;

                }
                else
                    std::cerr << "Error during SSL shutdown: " << error.message() << std::endl;
            });
            break;
        }
        else if (input.substr(0, 8) == "/search ")
        {
            {
                std::lock_guard<std::mutex> lock(socketMutex);
                boost::asio::write(socket_, boost::asio::buffer(input + "\n"));
            }
        }
        else if (input.substr(0, 5) == "/p2p ")
        {
            std::string command = input.substr(5);
            size_t spacePos = command.find(' ');
            if(spacePos != std::string::npos)
            {
                std::string targetUser = command.substr(0, spacePos);
                std::string message = command.substr(spacePos + 1);

                std::string p2pCommand = "/p2p " + targetUser + " " + message;
                {
                    std::lock_guard<std::mutex> lock(socketMutex);
                    boost::asio::write(socket_, boost::asio::buffer(p2pCommand + "\n"));
                }
            }
            else
                std::cout << "Invalid /p2p command format. Usage: /p2p {name} {message}\n";
        }
        else
        {
            {
                std::lock_guard<std::mutex> lock(socketMutex);
                boost::asio::write(socket_, boost::asio::buffer(input + "\n"));
            }
        }
    }
}

void Client::ReadMessages(ssl::stream<tcp::socket>& socket)
{
    try {
        while (true)
        {
            boost::asio::streambuf response;
            boost::system::error_code error;
            boost::asio::read_until(socket, response, '\n', error);

            if (error == boost::asio::error::eof)
                break;
            else if (error)
            {
                std::cerr << "Error while reading from socket: " << error.message() << std::endl;
                break;
            }

            std::string message(
                    boost::asio::buffers_begin(response.data()),
                    boost::asio::buffers_end(response.data())
            );
            response.consume(message.length());
            ReplaceSmiles(message);
            std::cout << message;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in ReadMessages: " << e.what() << std::endl;
    }
}
