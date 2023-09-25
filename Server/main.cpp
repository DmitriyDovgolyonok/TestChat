#include "Server.h"

int main() {
    boost::asio::io_context io_context;
    Server server(io_context, 443);
    io_context.run();
    return 0;
}
