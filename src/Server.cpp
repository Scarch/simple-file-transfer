#include "Server.hpp"
#include "Protocol.hpp"

#include <stdexcept>
#include <vector>


Server::Server(const std::string &outputDirectory, int port, asio::io_context &io_context) : m_port(port),
    m_io_context(io_context), m_acceptor(tcp::acceptor(
        m_io_context, tcp::endpoint(tcp::v4(), m_port))),
    m_outputDirectory(outputDirectory) {
    if (fs::exists(m_outputDirectory) && !fs::is_directory(m_outputDirectory)) {
        throw std::invalid_argument("Folder destination is occupied with non-directory element");
    }
}

void Server::start() {
    tcp::socket socket(m_io_context);
    m_acceptor.accept(socket);

    const FileMetadata incomingFile = receiveMetadata(socket);
    FileHandler fileHandler(m_outputDirectory, incomingFile);

    // TODO: Add try-except for cases when issues might arise (and warn the client accordingly so they don't start sending data that we can't receive)
    fileHandler.openForWrite();

    std::vector<char> writeBuffer(FileHandler::BUFFER_SIZE);
    uint64_t amountOfDataLeft = fileHandler.getFileSize();

    while (amountOfDataLeft > 0) {
        const uint64_t amountOfDataToWrite = std::min(static_cast<uint64_t>(FileHandler::BUFFER_SIZE), amountOfDataLeft);
        asio::read(socket, asio::buffer(writeBuffer, amountOfDataToWrite));
        fileHandler.writeChunk(writeBuffer, amountOfDataToWrite); // TODO: account for situations where writing might have failed (return value is false)
        amountOfDataLeft = amountOfDataLeft - amountOfDataToWrite;
    }
}

FileMetadata Server::receiveMetadata(tcp::socket &socket) {
    // First we receive the length of the file name
    uint32_t networkFileNameLength = 0;
    asio::read(socket, asio::buffer(&networkFileNameLength, sizeof(networkFileNameLength)));

    // Convert fileLength to host endianness
    const uint32_t fileNameLength = ntohl(networkFileNameLength);

    // Then we receive the name itself
    std::string fileName{};
    fileName.resize(fileNameLength);
    asio::read(socket, asio::buffer(fileName, fileNameLength));

    // Last thing we receive is the file size
    uint64_t networkFileSize = 0;
    asio::read(socket, asio::buffer(&networkFileSize, sizeof(networkFileSize)));

    // Convert fileSize to host endianness
    const uint64_t fileSize = networkToHost64(networkFileSize);

    return FileMetadata(fileName, fileSize);
}
