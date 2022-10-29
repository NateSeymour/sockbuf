#include <iostream>
#include <istream>
#include <ostream>
#include "sockbuf.test.h"

using namespace nys;

TEST_F(UnixSocketTest, ServerClient)
{
    auto server = sockbuf::UnixSocket(this->socket_path, SockMode::Server);
    auto client = sockbuf::UnixSocket(this->socket_path);

    auto server_connection = server.Accept();

    std::istream client_in(&client);
    std::ostream server_out(&server_connection);

    server_out << "Hey, there!\n";

    std::string message;
    client_in >> message;

    std::cout << message << std::endl;
}