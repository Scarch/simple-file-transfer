#include <iostream>
#include <exception>

#include "Client.hpp"
#include "CliParser.hpp"
#include "Server.hpp"
#include "asio/io_context.hpp"

// for sending a file from your computer back to your computer
// -p 8080 --receive
// --ip 127.0.0.1 -p 8080 --send --source C:/filepath/my_document.pdf

int main(int argc, char *argv[]) {
    try {
        auto argsOpt = parse(argc, argv);

        if (!argsOpt) {
            return EXIT_FAILURE;
        }

        const CliArguments &args = argsOpt.value();
        asio::io_context io_context;

        if (args.mode == Mode::Send) {
            std::cout << "[Client] Starting in Send mode...\n";

            Client client(args.ip, args.port, io_context);
            client.sendFile(args.sourcePath);
        } else if (args.mode == Mode::Receive) {
            std::string outputDirectory = args.outputPath.empty() ? "." : args.outputPath;

            std::cout << "[Server] Starting in Receive mode...\n";

            Server server(outputDirectory, args.port, io_context);
            server.start();
        }
    } catch (const asio::system_error &e) {
        std::cerr << "[Network Error] " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (const std::exception &e) {
        std::cerr << "[Error] " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "[Fatal Error] An unknown error occurred during execution.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
