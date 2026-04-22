#include "Server.hpp"
#include "Protocol.hpp"

#include <stdexcept>


Server::Server(const std::string &outputDirectory, int port, asio::io_context &io_context) : m_port(port),
    m_io_context(io_context), m_acceptor(tcp::acceptor(
        m_io_context, tcp::endpoint(tcp::v4(), m_port))),
    m_outputDirectory(outputDirectory) {
    if (fs::exists(m_outputDirectory) && !fs::is_directory(m_outputDirectory)) {
        throw std::invalid_argument("Folder destination is occupied with non-directory element");
    }
}

FileHandler Server::receiveMetadata(tcp::socket &socket) const {
    // First we receive the length of the file name
    uint32_t networkFileNameLength = 0;
    asio::read(socket, asio::buffer(&networkFileNameLength, sizeof(networkFileNameLength)));

    // Convert the fileLength to host endianness
    uint32_t fileNameLength = ntohl(networkFileNameLength);

    // Then we receive the name itself
    std::string fileName{};
    fileName.resize(fileNameLength);
    asio::read(socket, asio::buffer(fileName, fileNameLength));

    // Last thing we receive is the file size
    uint64_t fileSize = 0;
    asio::read(socket, asio::buffer(&fileSize, sizeof(fileSize)));

    FileHandler fileHandler(m_outputDirectory, FileMetadata(fileName, fileSize));
    return fileHandler;
}
