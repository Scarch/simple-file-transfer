#include "Server.hpp"
#include "Protocol.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <chrono>

// Use namespace so that isSafeFileName is only available in Server.cpp and doesn't conflict elsewhere
namespace {
    bool isSafeFileName(const std::string &fileName) {
        if (fileName.empty() || fileName == "." || fileName == "..") {
            return false;
        }

        if (fileName.find('\0') != std::string::npos || fileName.find('\\') != std::string::npos) {
            return false;
        }

        const fs::path candidate(fileName);
        return !candidate.is_absolute() && !candidate.has_parent_path() && candidate.filename() == candidate;
    }
}

Server::Server(const std::string &outputDirectory, int port, asio::io_context &io_context, std::ostream &log) :
    m_port(port),
    m_io_context(io_context),
    m_acceptor(tcp::acceptor(m_io_context, tcp::endpoint(tcp::v4(), m_port))),
    m_outputDirectory(outputDirectory),
    m_log(log) {

    m_log << "[Server] Listening on port: " << m_port << "\n";
    m_log << "[Server] Output directory: " << m_outputDirectory << "\n";

    if (fs::exists(m_outputDirectory) && !fs::is_directory(m_outputDirectory)) {
        throw std::invalid_argument("Folder destination is occupied with non-directory element");
    }
}

void Server::start() {
    m_log << "[Server] Waiting for incoming connections...\n";

    while (true) {
        tcp::socket socket(m_io_context);
        m_acceptor.accept(socket);
        m_log << "[Server] Client connected!\n";

        try {
            while (true) {
                const std::optional<FileMetadata> incomingFile = receiveMetadata(socket);

                if (!incomingFile.has_value()) {
                    m_log << "[Server] Client finished sending files. Connection closed.\n";
                    break;
                }

                fs::path targetPath = m_outputDirectory / incomingFile.value().fileName;
                if (fs::exists(targetPath)) {
                    const std::string errMsg = "File '" + incomingFile.value().fileName + "' already exists.";
                    m_log << "[Server] Error: " << errMsg << " Aborting transfer.\n";

                    sendError(socket, ErrorCode::InvalidFile, errMsg);

                    throw std::runtime_error(errMsg);
                }

                FileHandler fileHandler(m_outputDirectory, incomingFile.value());

                try {
                    fileHandler.openForWrite();
                    sendStatus(socket, Status::Ok);
                } catch (std::exception &e) {
                    sendError(socket, ErrorCode::WriteFailed, e.what());
                    throw;
                }

                std::vector<char> writeBuffer(FileHandler::BUFFER_SIZE);
                uint64_t amountOfDataLeft = fileHandler.getFileSize();

                // For calculating data transfer rate
                const auto start = std::chrono::steady_clock::now();

                while (amountOfDataLeft > 0) {
                    const uint64_t amountOfDataToWrite =
                            std::min(static_cast<uint64_t>(FileHandler::BUFFER_SIZE), amountOfDataLeft);
                    asio::read(socket, asio::buffer(writeBuffer, amountOfDataToWrite));

                    if (!fileHandler.writeChunk(writeBuffer, amountOfDataToWrite)) {
                        const std::string errorMessage = "Failed to write received data to disk";
                        sendError(socket, ErrorCode::WriteFailed, errorMessage);
                        throw std::runtime_error(errorMessage);
                    }

                    amountOfDataLeft = amountOfDataLeft - amountOfDataToWrite;
                }

                // We calculate the data transfer rate
                const auto end = std::chrono::steady_clock::now();
                const auto duration = std::chrono::duration<double>(end - start).count();
                const double bytesPerSecond{
                    duration > 0 ? static_cast<double>(fileHandler.getFileSize()) / duration : 0.0
                };

                m_log << "[Server] File received and saved successfully to '"
                      << (m_outputDirectory / incomingFile->fileName).string()
                      << "' (" << formatSize(bytesPerSecond) << "/s).\n";
            }
        } catch (const asio::system_error &e) {
            if (e.code() == asio::error::eof || e.code() == asio::error::connection_reset) {
                m_log << "[Server] Client finished sending files. Connection closed.\n";
            } else {
                m_log << "[Server] Network error: " << e.what() << "\n";
            }
        } catch (const std::exception &e) {
            m_log << "[Server] Connection error or peer disconnected: " << e.what() << "\n";
        }
    }
}

std::optional<FileMetadata> Server::receiveMetadata(tcp::socket &socket) {
    // First we receive the length of the file name
    uint32_t networkFileNameLength = 0;
    asio::error_code ec;
    asio::read(socket, asio::buffer(&networkFileNameLength, sizeof(networkFileNameLength)), ec);

    if (ec == asio::error::eof || ec == asio::error::connection_reset) {
        return std::nullopt;
    } else if (ec) {
        throw asio::system_error(ec);
    }

    // Convert fileLength to host endianness
    const uint32_t fileNameLength = ntohl(networkFileNameLength);

    // We validate the length before allocating/reading so a peer can't make us
    // resize to multiple GB before we reject the request.
    if (fileNameLength == 0 || fileNameLength > MAX_FILENAME_LENGTH) {
        const std::string errorMessage = "Invalid file name length";
        sendError(socket, ErrorCode::InvalidFile, errorMessage);
        throw std::runtime_error(errorMessage);
    }

    // Then we receive the name itself
    std::string fileName{};
    fileName.resize(fileNameLength);
    asio::read(socket, asio::buffer(fileName.data(), fileNameLength));

    if (!isSafeFileName(fileName)) {
        const std::string errorMessage = "Unsafe file name in metadata";
        sendError(socket, ErrorCode::InvalidFile, errorMessage);
        throw std::runtime_error(errorMessage);
    }

    // Last thing we receive is the file size
    uint64_t networkFileSize = 0;
    asio::read(socket, asio::buffer(&networkFileSize, sizeof(networkFileSize)));

    // Convert fileSize to host endianness
    const uint64_t fileSize = networkToHost64(networkFileSize);

    FileMetadata receivedMetadata(fileName, fileSize);
    m_log << "[Server] Received file metadata from peer: " << receivedMetadata << "\n";
    return receivedMetadata;
}
