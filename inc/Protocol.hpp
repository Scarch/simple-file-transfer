#pragma once
#include <asio.hpp>
#include <bit>
#include <string>
#include <cstdint>
#include <stdexcept>

enum class Status : uint8_t {
    Ok,
    Error,
};

enum class ErrorCode : uint8_t {
    Unknown,
    WriteFailed,
    InvalidPath,
};

constexpr uint32_t MAX_ERROR_MESSAGE_LENGTH{1024};

struct ErrorInfo {
    ErrorCode errorCode;
    std::string message;
};

inline void sendStatus(asio::ip::tcp::socket& socket, Status status) {
    // Status is 1 byte so there's no need to use htonl() to convert endianness
    asio::write(socket, asio::buffer(&status, sizeof(status)));
}

inline void sendError(asio::ip::tcp::socket& socket, ErrorCode errorCode, const std::string& errorMessage) {
    sendStatus(socket, Status::Error);

    // errorCode is 1 byte so there's no need to use htonl() to convert endianness
    asio::write(socket, asio::buffer(&errorCode, sizeof(errorCode)));

    const auto errorMessageLength = static_cast<uint32_t>(errorMessage.length());
    uint32_t networkErrorMessageLength = htonl(errorMessageLength);
    asio::write(socket, asio::buffer(&networkErrorMessageLength, sizeof(networkErrorMessageLength)));

    asio::write(socket, asio::buffer(errorMessage));
}

inline Status receiveStatus(asio::ip::tcp::socket& socket) {
    uint8_t rawStatus{};
    asio::read(socket, asio::buffer(&rawStatus, sizeof(rawStatus)));

    // We verify that the status is valid
    if (rawStatus > static_cast<uint8_t>(Status::Error)) {
        throw std::runtime_error("Received invalid status");
    }

    return static_cast<Status>(rawStatus);
}

inline ErrorInfo receiveError(asio::ip::tcp::socket& socket) {
    // We assume the status has already been received

    uint8_t rawErrorCode{};
    asio::read(socket, asio::buffer(&rawErrorCode, sizeof(rawErrorCode)));
    const auto errorCode = static_cast<ErrorCode>(rawErrorCode);

    uint32_t networkErrorMessageLength{};
    asio::read(socket, asio::buffer(&networkErrorMessageLength, sizeof(networkErrorMessageLength)));
    const uint32_t errorMessageLength = ntohl(networkErrorMessageLength);

    // We don't want to naively accept any message length
    if (errorMessageLength > MAX_ERROR_MESSAGE_LENGTH) {
        throw std::runtime_error("Received message length is over limit");
    }

    std::string errorMessage{};
    errorMessage.resize(errorMessageLength);
    asio::read(socket, asio::buffer(errorMessage, errorMessageLength));

    return ErrorInfo{errorCode, errorMessage};
}


// Problem: https://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c

// C++23 does provide byte-swapping, but we're using C++20
// There are also some compiler-specific builins like __builtin_bswap64, but it'd be nice to have something compiler-indpendent
// Therefore we do manual swapping of bytes
inline uint64_t swapBytes64(uint64_t value) {
    return ((value & 0x00000000000000FFULL) << 56) |
           ((value & 0x000000000000FF00ULL) << 40) |
           ((value & 0x0000000000FF0000ULL) << 24) |
           ((value & 0x00000000FF000000ULL) << 8) |
           ((value & 0x000000FF00000000ULL) >> 8) |
           ((value & 0x0000FF0000000000ULL) >> 24) |
           ((value & 0x00FF000000000000ULL) >> 40) |
           ((value & 0xFF00000000000000ULL) >> 56);
}

inline uint64_t hostToNetwork64(uint64_t value) {
    // std::endian is part of C++20 functionality
    // Network endianness should be big-endian
    if constexpr (std::endian::native == std::endian::big) {
        return value; // The host is already big-endian
    } else {
        return swapBytes64(value);
    }
}

inline uint64_t networkToHost64(uint64_t value) {
    if constexpr (std::endian::native == std::endian::big) {
        return value;
    } else {
        return swapBytes64(value);
    }
}
