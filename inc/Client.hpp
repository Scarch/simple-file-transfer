#pragma once
#include <string>
#include <iostream>
#include "FileHandler.hpp"
#include <asio.hpp>

// Client acts as the sender
class Client {
public:
    Client(const std::string &serverIp, int serverPort, asio::io_context &io_context, std::ostream &log = std::cout);

    void sendFile(const std::string &filePath);

private:
    asio::ip::address m_serverIp;
    asio::ip::port_type m_serverPort;
    asio::io_context &m_io_context;
    asio::ip::tcp::socket m_socket;
    std::ostream &m_log;

    void sendMetadata(FileHandler &file);
};
