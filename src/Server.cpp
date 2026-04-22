#include "Server.hpp"

#include <stdexcept>


Server::Server(const std::string &outputDirectory, int port, asio::io_context &io_context) : m_port(port), m_io_context(io_context), m_acceptor(tcp::acceptor(
        m_io_context, tcp::endpoint(tcp::v4(), m_port))),
                                                         m_outputDirectory(outputDirectory) {
    if (fs::exists(m_outputDirectory) && !fs::is_directory(m_outputDirectory)) {
        throw std::invalid_argument("Folder destination is occupied with non-directory element");
    }
}

FileHandler Server::receiveMetadata(tcp::socket& socket) {

    FileMetadata receivedMetadata{"", 0};

}
