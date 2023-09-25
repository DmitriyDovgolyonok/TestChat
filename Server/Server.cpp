//
// Created by Дмитрий on 25.09.23.
//
#include "Server.h"
#include <filesystem>

const std::filesystem::path pathToCertificate = std::filesystem::current_path() / "server_certificate.pem";
const std::filesystem::path pathToKey = std::filesystem::current_path() / "server_private_key.pem";
const std::filesystem::path pathToDH = std::filesystem::current_path() / "dhparams.pem";

Server::Server(boost::asio::io_context& io_context, unsigned short port) :
    acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
    ssl_context_(ssl::context::sslv23)
{
    ssl_context_.set_options(ssl::context::default_workarounds |
                                 ssl::context::no_sslv2 |
                                 ssl::context::single_dh_use);
    ssl_context_.set_password_callback([](std::size_t, ssl::context::password_purpose) { return "password"; });
    ssl_context_.use_certificate_chain_file(pathToCertificate.string());
    ssl_context_.use_private_key_file(pathToKey.string(), ssl::context::pem);

    StartAccept();
}

void Server::StartAccept()
{
    auto new_session = std::make_shared<Session>(acceptor_.get_executor(), clients_, ssl_context_,  messages_);
    acceptor_.async_accept(new_session->socket().lowest_layer(),
                           [this, new_session](const boost::system::error_code& error) {
                               if (!error)
                               {
                                   new_session->Start();
                               }
                               StartAccept();
                           });
}