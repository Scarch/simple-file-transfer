#pragma once
#include <string>
#include "FileHandler.hpp"
#include <asio.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using asio::ip::tcp;

// Server acts as the receiver
class Server {
public:
    Server(const std::string &outputDirectory, int port, asio::io_context &io_context);

    void start();

private:
    asio::ip::port_type m_port;
    asio::io_context &m_io_context;
    tcp::acceptor m_acceptor;
    fs::path m_outputDirectory;

    FileHandler receiveMetadata(tcp::socket &socket) const;
};
