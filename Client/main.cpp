#include "Client.h"

int main() {
    boost::asio::io_context io_context;
    Client client(io_context, "127.0.0.1", 443);

    try {
        client.Connect();
        client.Send();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
