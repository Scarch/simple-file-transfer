#include "CliParser.hpp"

#include <iostream>
#include <regex>

bool isValidIp(const std::string& ip) {
    static const std::regex pattern(
    R"(^(25[0-5]|2[0-4]\d|1?\d?\d)(\.(25[0-5]|2[0-4]\d|1?\d?\d)){3}$)"
    );
    return std::regex_match(ip, pattern);
}

std::optional<CliArguments> parse(int argc, char* argv[]) {
    CliArguments args;

    if (argc == 1) {
        std::cerr << "Kasutamine:\n"
                  << "--ip <aadress> -p <port> (--send|--receive) [--file <path>]\n";
        return std::nullopt;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // HELP
        if (arg == "--help" || arg == "-h") {
            std::cout << "Kasutamine:\n"
                      << "  --ip <aadress>\n"
                      << "  -p, --port <port>\n"
                      << "  -s, --send\n"
                      << "  -r, --receive\n"
                      << "  -f, --file <path> (vajalik send mode puhul)\n";
            return std::nullopt;
        }

        // IP
        if (arg == "--ip") {
            if (i + 1 >= argc) {
                std::cerr << "Viga: --ip vajab väärtust\n";
                return std::nullopt;
            }
            args.ip = argv[++i];
        }

        // PORT
        else if (arg == "-p" || arg == "--port") {
            if (i + 1 >= argc) {
                std::cerr << "Viga: --port vajab väärtust\n";
                return std::nullopt;
            }
            try {
                args.port = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "Viga: port peab olema number\n";
                return std::nullopt;
            }

            if (args.port <= 0 || args.port > 65535) {
                std::cerr << "Viga: port peab olema vahemikus 1-65535\n";
                return std::nullopt;
            }
        }

        // SEND
        else if (arg == "--send" || arg == "-s") {
            if (args.mode != Mode::Unknown) {
                std::cerr << "Viga: vali kas send VÕI receive\n";
                return std::nullopt;
            }
            args.mode = Mode::Send;
        }

        // RECEIVE
        else if (arg == "--receive" || arg == "-r") {
            if (args.mode != Mode::Unknown) {
                std::cerr << "Viga: vali kas send VÕI receive\n";
                return std::nullopt;
            }
            args.mode = Mode::Receive;
        }

        // FILE
        else if (arg == "--file" || arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "Viga: --file vajab väärtust\n";
                return std::nullopt;
            }
            args.filePath = argv[++i];
        }

        // UNKNOWN
        else {
            std::cerr << "Tundmatu argument: " << arg << "\n";
            return std::nullopt;
        }
    }

    // Valideerimine

    if (args.ip.empty()) {
        std::cerr << "Viga: IP puudub\n";
        return std::nullopt;
    }

    if (!isValidIp(args.ip)) {
        std::cerr << "Viga: vigane IP aadress\n";
        return std::nullopt;
    }

    if (args.port == 0) {
        std::cerr << "Viga: port puudub\n";
        return std::nullopt;
    }

    if (args.mode == Mode::Unknown) {
        std::cerr << "Viga: mode puudub (--send või --receive)\n";
        return std::nullopt;
    }

    if (args.mode == Mode::Send && args.filePath.empty()) {
        std::cerr << "Viga: send mode nõuab faili (--file)\n";
        return std::nullopt;
    }

    return args;
}