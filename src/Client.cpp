#include <chrono>
#include "Client.hpp"
#include "Protocol.hpp"
using asio::ip::tcp;

Client::Client(const std::string &serverIp, int serverPort,
               asio::io_context &io_context, std::ostream &log) : m_serverIp(asio::ip::make_address(serverIp)),
                                                                  m_serverPort(serverPort), m_io_context(io_context),
                                                                  m_socket(m_io_context), m_log(log) {
}

void Client::sendFile(const std::string &filePath) {
    // We start each sending operation with a "fresh" socket
    if (m_socket.is_open()) {
        m_socket.close();
    }
    m_socket = tcp::socket(m_io_context);

    const tcp::endpoint endpoint{m_serverIp, m_serverPort};
    m_log << "[Client] Connecting to " << m_serverIp << ":" << m_serverPort << "\n";
    m_socket.connect(endpoint);

    FileHandler fileHandler(filePath);
    fileHandler.openForRead();
    m_log << "[Client] Sending file: " << filePath << "\n";

    sendMetadata(fileHandler);

    // We check whether the server failed to open up the FileHandler for writing
    throwIfPeerError(m_socket);

    std::vector<char> readBuffer(FileHandler::BUFFER_SIZE);
    size_t bytesRead = 0;

    // For calculating data transfer rate
    const auto start = std::chrono::steady_clock::now();

    while ((bytesRead = fileHandler.readChunk(readBuffer)) > 0) {
        // We read only the necessary amount of data (bytesRead) from the buffer
        asio::write(m_socket, asio::buffer(readBuffer.data(), bytesRead));

        // We check if the server successfully received the chunk
        throwIfPeerError(m_socket);
    }

    // We calculate the data transfer rate
    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration<double>(end - start).count();
    const double bytesPerSecond{
        duration > 0 ? static_cast<double>(fileHandler.getFileSize()) / duration : 0.0
    };

    m_log << "[Client] File '" << filePath << "' was sent successfully (" <<
            formatSize(bytesPerSecond) << "/s).\n";
}

void Client::sendMetadata(FileHandler &file) {
    FileMetadata fileMetadata(file.getFileName(), file.getFileSize());

    // We send the file name length for the server before the name itself
    const auto fileNameLength = static_cast<uint32_t>(fileMetadata.fileName.length());
    uint32_t networkFileNameLength = htonl(fileNameLength);
    asio::write(m_socket, asio::buffer(&networkFileNameLength, sizeof(networkFileNameLength)));

    // Then we send the actual file name
    asio::write(m_socket, asio::buffer(fileMetadata.fileName));

    // Now we send the file size
    // To account for OS-specific byte-ordering, we need to make sure data is sent independent of OS
    uint64_t networkFileSize = hostToNetwork64(fileMetadata.fileSize);
    asio::write(m_socket, asio::buffer(&networkFileSize, sizeof(networkFileSize)));

    m_log << "[Client] Sent file metadata to peer: " << fileMetadata << "\n";

    // Receive status from the server
    throwIfPeerError(m_socket);
}
