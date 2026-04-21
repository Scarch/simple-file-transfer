#pragma once
#include <string>
#include "FileHandler.hpp"
#include <asio.hpp>
using asio::ip::tcp;

// Client acts as the sender
class Client {
public:
    Client(const std::string &serverIp, int serverPort, asio::io_context &io_context);

    void sendFile(const std::string &filePath);

private:
    asio::ip::address m_serverIp;
    asio::ip::port_type m_serverPort;
    asio::io_context &m_io_context;
    tcp::socket m_socket;

    bool sendMetadata();
};
