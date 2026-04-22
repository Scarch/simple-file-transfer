#include "Protocol.hpp"

// Manual swapping of bytes
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
