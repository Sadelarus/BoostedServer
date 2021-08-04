#include <iostream>
#include <boost/asio.hpp>
#include <boost/ref.hpp>
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

int main() {
    boost::asio::io_service io_service;
//socket creation
    tcp::socket socket(io_service);
//connection
    data.args.a = 3;
    data.args.b = 2;
    socket.async_connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"),
                                       8080),
                         [&socket](const boost::system::error_code& error) {
                               connect_handler(boost::ref(socket), error);
                         });
    io_service.run();
    return 0;
}
