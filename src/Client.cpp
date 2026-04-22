#include "Client.hpp"
using asio::ip::tcp;

Client::Client(const std::string &serverIp, int serverPort, asio::io_context &io_context) : m_serverIp(asio::ip::make_address(serverIp)),
    m_serverPort(serverPort), m_io_context(io_context),
    m_socket(m_io_context) {
}

void Client::sendFile(const std::string &filePath) {

    const tcp::endpoint endpoint{m_serverIp, m_serverPort};
    m_socket.connect(endpoint);

    FileHandler fileHandler(filePath);
    fileHandler.openForRead();
    sendMetadata(fileHandler);

    std::vector<char> readBuffer(FileHandler::BUFFER_SIZE);
    size_t bytesRead = 0;

    while ((bytesRead = fileHandler.readChunk(readBuffer)) > 0) {
        // We read only the necessary amount of data (bytesRead) from the buffer
        asio::write(m_socket, asio::buffer(readBuffer.data(), bytesRead));
    }

}

bool Client::sendMetadata(FileHandler &file) {

    // To account for OS-specific byte-ordering, we need to make sure data is sent independent of OS
    //uintmax_t networkFileSize = htonl();
}
