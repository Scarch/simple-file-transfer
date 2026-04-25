#include "Client.hpp"
#include "Protocol.hpp"
using asio::ip::tcp;

Client::Client(const std::string &serverIp, int serverPort,
               asio::io_context &io_context) : m_serverIp(asio::ip::make_address(serverIp)),
                                               m_serverPort(serverPort), m_io_context(io_context),
                                               m_socket(m_io_context) {
}

void Client::sendFile(const std::string &filePath) {
    // We start each sending operation with a "fresh" socket
    if (m_socket.is_open()) {
        m_socket.close();
    }
    m_socket = tcp::socket(m_io_context);

    const tcp::endpoint endpoint{m_serverIp, m_serverPort};
    m_socket.connect(endpoint);

    FileHandler fileHandler(filePath);
    fileHandler.openForRead();

    // TODO: account for situations wheere sendMetadata might return false
    sendMetadata(fileHandler);

    std::vector<char> readBuffer(FileHandler::BUFFER_SIZE);
    size_t bytesRead = 0;

    while ((bytesRead = fileHandler.readChunk(readBuffer)) > 0) {
        // We read only the necessary amount of data (bytesRead) from the buffer
        asio::write(m_socket, asio::buffer(readBuffer.data(), bytesRead));
    }
}

bool Client::sendMetadata(FileHandler &file) {
    std::string fileName = file.getFileName();

    // We send the file name length for the server before the name itself
    const uint32_t fileNameLength = fileName.length();
    uint32_t networkFileNameLength = htonl(fileNameLength);
    asio::write(m_socket, asio::buffer(&networkFileNameLength, sizeof(networkFileNameLength)));

    // Then we send the actual file name
    asio::write(m_socket, asio::buffer(fileName));

    // Now we send the file size
    // To account for OS-specific byte-ordering, we need to make sure data is sent independent of OS
    uint64_t networkFileSize = hostToNetwork64(file.getFileSize());
    asio::write(m_socket, asio::buffer(&networkFileSize, sizeof(networkFileSize)));

    // TODO: add some sort of functionality so that client receives feedback from server whether server is ready to receive file data
    // ...
    // if (...) {
    //     return false;
    // }

    return true;
}
