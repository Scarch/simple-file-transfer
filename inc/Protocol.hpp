#pragma once
#include <asio.hpp>

// Problem: https://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c

// C++23 does provide byte-swapping, but we're using C++20
// There are also some compiler-specific builins like __builtin_bswap64, but it'd be nice to have something compiler-indpendent
// Therefore we do manual swapping of bytes
inline uint64_t swapBytes64(uint64_t value);

inline uint64_t hostToNetwork64(uint64_t value);

inline uint64_t networkToHost64(uint64_t value);
