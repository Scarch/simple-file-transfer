#include "Client.hpp"
using asio::ip::tcp;

Client::Client(const std::string &serverIp, int serverPort, asio::io_context &io_context) : m_serverIp(asio::ip::make_address(serverIp)),
    m_serverPort(serverPort), m_io_context(io_context),
    m_socket(m_io_context) {
}

void Client::sendFile(const std::string &filePath) {

    const tcp::endpoint endpoint{m_serverIp, m_serverPort};
    m_socket.connect(endpoint);

    FileHandler file(filePath);
    file.openForRead();
    sendMetadata(file);

    std::vector<char> readBuffer(FileHandler::BUFFER_SIZE);

}
