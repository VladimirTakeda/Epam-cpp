#include "src/dataqueue/sharedbuffer.h"
#include "src/network/server/tcpserver.h"
#include "src/task.h"
#include "src/threadpool.h"
#include "src/util.h"

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <functional>
#include <iostream>
#include <thread>

void MeasureTime(const std::function<void()>& func)
{
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end                               = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Queue: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsec" << std::endl;
}

bool ValidateArguments(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "wrong arguments count" << std::endl;
        return false;
    }

    if (!std::filesystem::is_regular_file(argv[1])) {
        std::cerr << "wrong source file path" << std::endl;
        return false;
    }

    return true;
}

void run_client(const std::string& host, const std::string& port)
{
    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, port);
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        char request[Session::max_length];
        std::cout << "Enter message: ";
        std::cin.getline(request, Session::max_length);

        size_t request_length = std::strlen(request);
        boost::asio::write(socket, boost::asio::buffer(request, request_length));

        char reply[Session::max_length];
        size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, request_length));

        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[])
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")("client", "run in client mode")("server", "run in server mode")(
        "port", boost::program_options::value<short>(), "set port")("host", boost::program_options::value<std::string>(),
                                                                    "set host (for client)");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.contains("server")) {
        if (!vm.count("port")) {
            std::cerr << "Port not set for server mode.\n";
            return 1;
        }

        try {
            boost::asio::io_context io_context;

            short port = vm["port"].as<short>();
            Server s(io_context, port);
            io_context.run();
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    } else if (vm.contains("client")) {
        if (!vm.count("port") || !vm.count("host")) {
            std::cerr << "Port or host not set for client mode.\n";
            return 1;
        }

        std::string host = vm["host"].as<std::string>();
        std::string port = std::to_string(vm["port"].as<short>());

        run_client(host, port);
    } else {
        std::cerr << "Mode (client or server) not set.\n";
        return 1;
    }
}
