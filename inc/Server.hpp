#pragma once
#include <string>
#include <iostream>
#include "FileHandler.hpp"
#include <asio.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using asio::ip::tcp;

// Server acts as the receiver
class Server {
public:
    Server(const std::string &outputDirectory, int port, asio::io_context &io_context, std::ostream &log = std::cout);

    void start();

private:
    static constexpr uint32_t MAX_FILENAME_LENGTH{255};

    asio::ip::port_type m_port;
    asio::io_context &m_io_context;
    tcp::acceptor m_acceptor;
    fs::path m_outputDirectory;
    std::ostream &m_log;

    FileMetadata receiveMetadata(tcp::socket &socket) const;
};
