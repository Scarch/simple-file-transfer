#pragma once
#include <string>
#include <optional>

enum class Mode {
    Send,
    Receive,
    Unknown
};

struct CliArguments {
    std::string ip;
    int port{};
    Mode mode = Mode::Unknown;
    std::string filePath;
};

std::optional<CliArguments> parse(int argc, char* argv[]);

