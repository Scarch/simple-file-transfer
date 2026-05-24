#pragma once
#include <string>
#include <iostream>
#include <filesystem>
#include "FileHandler.hpp"
#include <asio.hpp>

class Client {
public:
    Client(const std::string &serverIp, int serverPort, asio::io_context &io_context, std::ostream &log = std::cout);

    void sendPath(const std::string &targetPath);

private:
    asio::ip::address m_serverIp;
    asio::ip::port_type m_serverPort;
    asio::io_context &m_io_context;
    asio::ip::tcp::socket m_socket;
    std::ostream &m_log;

    void transmitSingleFile(const fs::path &filePath);
    void sendMetadata(FileHandler& file);
};