#include "CliParser.hpp"
#include <iostream>
#include "asio/ip/address.hpp"

bool isValidIp(const std::string &ip) {
    asio::error_code ec;
    asio::ip::make_address(ip, ec);
    return !ec;
}

std::optional<CliArguments> parse(int argc, char *argv[]) {
    CliArguments args;

    if (argc == 1) {
        std::cerr << "Usage:\n"
                << "  Send:    --ip <address> -p <port> --send --source <path>\n"
                << "  Receive: -p <port> --receive [--output <path>]\n";
        return std::nullopt;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // HELP
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage:\n"
                    << "  --ip <address>          (send mode only)\n"
                    << "  -p, --port <port>\n"
                    << "  -s, --send\n"
                    << "  -r, --receive\n"
                    << "  -f, --source <path>     file or directory to send (send mode only)\n"
                    << "  -o, --output <path>     directory to save into (receive mode, defaults to .)\n";
            return std::nullopt;
        }

        // IP
        if (arg == "--ip") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --ip needs a value\n";
                return std::nullopt;
            }
            args.ip = argv[++i];
        }

        // PORT
        else if (arg == "-p" || arg == "--port") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --port needs a value\n";
                return std::nullopt;
            }
            try {
                args.port = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "Error: port must be a number\n";
                return std::nullopt;
            }

            if (args.port <= 0 || args.port > 65535) {
                std::cerr << "Error: port must be between 1-65535\n";
                return std::nullopt;
            }
        }

        // SEND
        else if (arg == "--send" || arg == "-s") {
            if (args.mode != Mode::Unknown) {
                std::cerr << "Error: choose send OR receive\n";
                return std::nullopt;
            }
            args.mode = Mode::Send;
        }

        // RECEIVE
        else if (arg == "--receive" || arg == "-r") {
            if (args.mode != Mode::Unknown) {
                std::cerr << "Error: choose send OR receive\n";
                return std::nullopt;
            }
            args.mode = Mode::Receive;
        }

        // SOURCE
        else if (arg == "--source" || arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --source needs a value\n";
                return std::nullopt;
            }
            args.sourcePath = argv[++i];
        }

        // OUTPUT
        else if (arg == "--output" || arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --output needs a value\n";
                return std::nullopt;
            }
            args.outputPath = argv[++i];
        }

        // UNKNOWN
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return std::nullopt;
        }
    }

    // -- Validation --
    if (args.port == 0) {
        std::cerr << "Error: port is missing\n";
        return std::nullopt;
    }

    if (args.mode == Mode::Unknown) {
        std::cerr << "Error: mode is missing (--send or --receive)\n";
        return std::nullopt;
    }

    if (args.mode == Mode::Send) {
        if (args.ip.empty()) {
            std::cerr << "Error: IP is missing for send mode\n";
            return std::nullopt;
        }

        if (!isValidIp(args.ip)) {
            std::cerr << "Error: faulty IP address\n";
            return std::nullopt;
        }

        if (args.sourcePath.empty()) {
            std::cerr << "Error: send mode requires a source path (--source)\n";
            return std::nullopt;
        }

        if (!args.outputPath.empty()) {
            std::cerr << "Error: --output is only valid in receive mode\n";
            return std::nullopt;
        }
    } else if (args.mode == Mode::Receive) {
        if (!args.sourcePath.empty()) {
            std::cerr << "Error: --source is only valid in send mode\n";
            return std::nullopt;
        }
    }

    return args;
}
