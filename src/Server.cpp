#include "Server.hpp"
#include <string>
#include "FileHandler.hpp"
#include <asio.hpp>
using asio::ip::tcp;

// Server acts as the receiver
class Server {
public:
    Server(int port, asio::io_context &io_context);

private:
    asio::ip::port_type m_serverPort;
    asio::io_context &m_io_context;
    tcp::acceptor m_acceptor;

    FileHandler receiveMetadata();
};
