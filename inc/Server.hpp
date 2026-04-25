#pragma once
#include <string>
#include "FileHandler.hpp"
#include <asio.hpp>
#include <filesystem>
#include <cstdint>

namespace fs = std::filesystem;
using asio::ip::tcp;

// Server acts as the receiver
class Server {
public:
    Server(const std::string &outputDirectory, int port, asio::io_context &io_context);

    void start();

private:
    static constexpr uint32_t MAX_FILENAME_LENGTH{255};

    asio::ip::port_type m_port;
    asio::io_context &m_io_context;
    tcp::acceptor m_acceptor;
    fs::path m_outputDirectory;

    static FileMetadata receiveMetadata(tcp::socket &socket);
};
