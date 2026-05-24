#include "CliParser.hpp"
#include <iostream>
#include "asio/ip/address.hpp"

bool isValidIp(const std::string& ip) {
    asio::error_code ec;
    asio::ip::make_address(ip, ec);
    return !ec;
}

std::optional<CliArguments> parse(int argc, char* argv[]) {
    CliArguments args;

    if (argc == 1) {
        std::cerr << "Usage:\n"
                  << "--ip <address> -p <port> (--send|--receive) [--file <path>]\n";
        return std::nullopt;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // HELP
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage:\n"
                      << "  --ip <address>\n"
                      << "  -p, --port <port>\n"
                      << "  -s, --send\n"
                      << "  -r, --receive\n"
                      << "  -f, --file <path> (needed for send mode)\n";
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

        // FILE
        else if (arg == "--file" || arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --file needs a value\n";
                return std::nullopt;
            }
            args.filePath = argv[++i];
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

        if (args.filePath.empty()) {
            std::cerr << "Error: send mode requires a file (--file)\n";
            return std::nullopt;
        }
    }

    return args;
}