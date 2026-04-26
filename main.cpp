#include <iostream>
#include <exception>

#include "Client.hpp"
#include "CliParser.hpp"
#include "Server.hpp"
#include "asio/io_context.hpp"

// for sending a file from your computer back to your computer
// --ip 127.0.0.1 -p 8080 --receive
// --ip 127.0.0.1 -p 8080 --send -f C:/filepath/my_document.pdf

int main(int argc, char* argv[]) {
    try {
        auto argsOpt = parse(argc, argv);

        if (!argsOpt) {
            return EXIT_FAILURE;
        }

        const CliArguments& args = argsOpt.value();
        asio::io_context io_context;

        if (args.mode == Mode::Send) {
            std::cout << "[Client] Starting in Send mode...\n";
            std::cout << "[Client] Target: " << args.ip << ":" << args.port << "\n";
            std::cout << "[Client] File to send: " << args.filePath << "\n";

            Client client(args.ip, args.port, io_context);
            client.sendFile(args.filePath);

            std::cout << "[Client] File '" << args.filePath << "' was sent successfully.\n";
        } else if (args.mode == Mode::Receive) {
            std::string outputDirectory = args.filePath.empty() ? "." : args.filePath;

            std::cout << "[Server] Starting in Receive mode...\n";
            std::cout << "[Server] Listening on port: " << args.port << "\n";
            std::cout << "[Server] Output directory: " << outputDirectory << "\n";
            std::cout << "[Server] Waiting for incoming connections...\n";

            Server server(outputDirectory, args.port, io_context);
            server.start();

            std::cout << "[Server] File received and saved successfully to '" << outputDirectory << "'.\n";
        }
    } catch (const asio::system_error& e) {
        std::cerr << "[Network Error] " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "[Fatal Error] An unknown error occurred during execution.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
