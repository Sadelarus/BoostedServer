#include <iostream>
#include <boost/asio.hpp>
#include <boost/ref.hpp>
#include <boost/program_options.hpp>
#include "includes.h"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

namespace {
    unData data;

    void read_handle(tcp::socket& socket,
                     const boost::system::error_code& err,
                     std::size_t bytes_transferred) {
        if (err) {
            std::cerr << "Error (receive): " << err.message() << std::endl;
            return;
        }

        std::cout << "Received result [" << data.res.res << "]" << std::endl;
    }

    void send_handle(tcp::socket& socket,
                     const boost::system::error_code& err,
                     std::size_t bytes_transferred) {
        if (err) {
            std::cerr << "Error (send): " << err.message() << std::endl;
            return;
        }

        socket.async_receive(boost::asio::buffer(&data.args, sizeof(data.args)),
                             [&socket](const boost::system::error_code& error,
                                                std::size_t bytes_transferred) {
                                    read_handle(boost::ref(socket), error, bytes_transferred);
                             });
    }

    void connect_handler(tcp::socket& socket,
                         const boost::system::error_code& err) {
        if (err) {
            std::cerr << "Error (connect): " << err.message() << std::endl;
            return;
        }

        std::cout << "Ready to send the data for processing ["
                  <<  data.args.a << " + " << data.args.b << "]" << std::endl;

        socket.async_send(boost::asio::buffer(&data.args, sizeof(data.args)),
                          [&socket](const boost::system::error_code& error,
                                             std::size_t bytes_transferred) {
                              send_handle(boost::ref(socket), error, bytes_transferred);
                           });
    }
}

int main(int argc, char** argv) {

    std::string serverAddr = "127.0.0.1";
    std::uint16_t port = 8080;
    namespace po = boost::program_options;
    po::options_description desc("Application options");
    desc.add_options()
    ("help,h", "Show help")
    ("arg-1,a", po::value<double>(&data.args.a), "Argument #1")
    ("arg-2,A", po::value<double>(&data.args.b), "Argument #2")
    ("server-adr,s", po::value<std::string>(&serverAddr), "Server address")
    ("server-port,p", po::value<std::uint16_t>(&port), "Server port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);


    if (vm.count("help") || argc == 1) {
        std::cout << desc << std::endl;
        return 1;
    }

    if (vm.count("arg-1") == 0 || vm.count("arg-2") == 0) {
        std::cout << "Parameters were not passed" << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    boost::asio::io_service io_service;
//socket creation
    tcp::socket socket(io_service);
//connection
    socket.async_connect(tcp::endpoint(boost::asio::ip::address::from_string(serverAddr),
                                       port),
                         [&socket](const boost::system::error_code& error) {
                               connect_handler(boost::ref(socket), error);
                         });
    io_service.run();
    return 0;
}
