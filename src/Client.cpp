#include <chrono>
#include "Client.hpp"
#include "Protocol.hpp"
using asio::ip::tcp;

Client::Client(const std::string &serverIp, int serverPort,
               asio::io_context &io_context, std::ostream &log) : m_serverIp(asio::ip::make_address(serverIp)),
                                               m_serverPort(serverPort), m_io_context(io_context),
                                               m_socket(m_io_context), m_log(log) {
}

void Client::sendPath(const std::string &targetPath) {
    fs::path path(targetPath);

    if (!fs::exists(path)) {
        throw std::invalid_argument("Provided path does not exist: " + targetPath);
    }
    if (!fs::is_regular_file(path) && !fs::is_directory(path)) {
        throw std::invalid_argument("Provided path is neither a regular file nor a directory");
    }

    if (m_socket.is_open()) {
        m_socket.close();
    }
    m_socket = tcp::socket(m_io_context);

    const tcp::endpoint endpoint{m_serverIp, m_serverPort};
    m_log << "[Client] Connecting to " << m_serverIp << ":" << m_serverPort << "\n";
    m_socket.connect(endpoint);

    if (fs::is_regular_file(path)) {
        transmitSingleFile(path);
    } else if (fs::is_directory(path)) {
        // Does not include subdirectories; only files in the larger directory
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                transmitSingleFile(entry.path());
            }
        }
    } else {
        throw std::invalid_argument("Provided path is neither a file nor a directory");
    }

    asio::error_code ec;
    m_socket.shutdown(tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
}

void Client::transmitSingleFile(const fs::path &filePath) {
    FileHandler fileHandler(filePath.string());
    fileHandler.openForRead();
    m_log << "[Client] Sending file: " << filePath << "\n";

    sendMetadata(fileHandler);

    // We check whether the server failed to open up the FileHandler for writing
    throwIfPeerError(m_socket);

    std::vector<char> readBuffer(FileHandler::BUFFER_SIZE);
    size_t bytesRead = 0;

    while ((bytesRead = fileHandler.readChunk(readBuffer)) > 0) {
        asio::write(m_socket, asio::buffer(readBuffer.data(), bytesRead));
    }
}

void Client::sendMetadata(FileHandler &file) {
    std::string fileName = file.getFileName();
    const uint32_t fileNameLength = fileName.length();
    uint32_t networkFileNameLength = htonl(fileNameLength);
    asio::write(m_socket, asio::buffer(&networkFileNameLength, sizeof(networkFileNameLength)));
    asio::write(m_socket, asio::buffer(fileName));

    uint64_t networkFileSize = hostToNetwork64(file.getFileSize());
    asio::write(m_socket, asio::buffer(&networkFileSize, sizeof(networkFileSize)));
}