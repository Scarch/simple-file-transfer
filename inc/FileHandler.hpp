#pragma once
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

struct FileMetadata {
    std::string fileName;
    uint64_t fileSize;
};

class FileHandler {
public:
    enum class Mode {
        CLOSED,
        READ,
        WRITE
    };

    static constexpr size_t BUFFER_SIZE{8192};

    explicit FileHandler(const std::string &filePath);

    explicit FileHandler(const fs::path &directory, const FileMetadata &fileMetadata);

    void openForRead();

    void openForWrite();

    size_t readChunk(std::vector<char> &buffer);

    bool writeChunk(const std::vector<char> &buffer, size_t bytesToWrite);

    [[nodiscard]] std::string getFileName() const;

    [[nodiscard]] uint64_t getFileSize() const;

private:
    fs::path m_filePath;
    FileMetadata m_fileMetadata;
    std::fstream m_fileStream{};
    Mode m_currentMode{Mode::CLOSED};
};
