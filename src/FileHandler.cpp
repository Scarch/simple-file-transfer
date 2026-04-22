#include "FileHandler.hpp"

#include <stdexcept>

// We give m_fileSize a base value (real value depends on if we're receiving or sending a file and whether the file exists)
FileHandler::FileHandler(const std::string &filePath) : m_filePath(filePath), m_fileMetadata(m_filePath.filename(), 0) {
}

FileHandler::FileHandler(const fs::path &directory, const FileMetadata &fileMetadata) : m_filePath(
        directory / fileMetadata.fileName), m_fileMetadata(fileMetadata) {
}

FileHandler::~FileHandler() {
    m_fileStream.close();
}

void FileHandler::openForRead() {
    if (!fs::exists(m_filePath)) {
        throw std::invalid_argument("File does not exist");
    }

    if (fs::is_directory(m_filePath)) {
        throw std::invalid_argument("File cannot be a directory");
    }

    m_fileStream.open(m_filePath, std::ios::in | std::ios::binary);

    if (!m_fileStream.is_open()) {
        throw std::runtime_error("Failed to open file for reading (possible permission issue or locked file).");
    }

    m_fileMetadata.fileSize = fs::file_size(m_filePath);
    m_currentMode = Mode::READ;
}

void FileHandler::openForWrite() {
    // TODO: add safe overwrite handling
    if (fs::exists(m_filePath)) {
        throw std::invalid_argument("File already exists");
    }

    m_fileStream.open(m_filePath, std::ios::out | std::ios::trunc | std::ios::binary);

    if (!m_fileStream.is_open()) {
        throw std::runtime_error("Failed to open file for writing (check folder permissions).");
    }

    m_currentMode = Mode::WRITE;
}

size_t FileHandler::readChunk(std::vector<char> &buffer) {
    if (m_currentMode != Mode::READ) {
        throw std::logic_error("File not open for reading");
    }

    // We provide buffer.data() since the read function requires a pointer
    // buffer.begin() provides an iterator which is incompatible
    m_fileStream.read(buffer.data(), BUFFER_SIZE);

    // We return the amount of bytes read in the last operation
    return m_fileStream.gcount();
}

bool FileHandler::writeChunk(const std::vector<char> &buffer, size_t bytesToWrite) {
    if (m_currentMode != Mode::WRITE) {
        throw std::logic_error("File not open for writing");
    }

    m_fileStream.write(buffer.data(), static_cast<std::streamsize>(bytesToWrite));

    // We check if writing failed
    if (m_fileStream.fail()) {
        return false;
    }

    return true;
}

std::string FileHandler::getFileName() const {
    return m_fileMetadata.fileName;
}

uint64_t FileHandler::getFileSize() const {
    return m_fileMetadata.fileSize;
}
