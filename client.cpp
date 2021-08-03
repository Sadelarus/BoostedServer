#include <iostream>
#include <boost/asio.hpp>
#include <boost/ref.hpp>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

namespace {
    typedef uint8_t Buff_t[sizeof(double) * 2];
    typedef boost::shared_ptr<Buff_t> BufferSPtr_t;

    void read_handle(tcp::socket& socket,
                     BufferSPtr_t pBuffer,
                     const boost::system::error_code& err,
                     std::size_t bytes_transferred) {
        if (err) {
            std::cerr << "Error (receive): " << err.message() << std::endl;
            return;
        }

        double res;

        std::memcpy(&res, pBuffer.get(), sizeof(double));

        std::cout << "Received result [" << res << "]" << std::endl;
    }

    void send_handle(tcp::socket& socket,
                     BufferSPtr_t pBuffer,
                     const boost::system::error_code& err,
                     std::size_t bytes_transferred) {
        if (err) {
            std::cerr << "Error (send): " << err.message() << std::endl;
            return;
        }

        socket.async_receive(boost::asio::buffer(pBuffer.get(), sizeof(double)),
                             [&socket, pBuffer](const boost::system::error_code& error,
                                                std::size_t bytes_transferred) {
                                    read_handle(boost::ref(socket), pBuffer, error, bytes_transferred);
                             });
    }

    void connect_handler(tcp::socket& socket,
                         const boost::system::error_code& err) {
        if (err) {
            std::cerr << "Error (connect): " << err.message() << std::endl;
            return;
        }

        const double a1 = 3;
        const double a2 = 5;

        std::cout << "Ready to send the data for processing ["
                  <<  a1 << " + " << a2 << "]" << std::endl;

        BufferSPtr_t pBuffer(new Buff_t);

        memcpy(pBuffer.get(), &a1, sizeof(a1));
        memcpy(pBuffer.get() + sizeof(a1), &a2, sizeof(a2));

        socket.async_send(boost::asio::buffer(pBuffer.get(), sizeof(Buff_t)),
                          [pBuffer, &socket](const boost::system::error_code& error,
                                             std::size_t bytes_transferred) {
                              send_handle(boost::ref(socket), pBuffer, error, bytes_transferred);
                           });
    }
}

int main() {
    boost::asio::io_service io_service;
//socket creation
    tcp::socket socket(io_service);
//connection

    socket.async_connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"),
                                       8080),
                         [&socket](const boost::system::error_code& error) {
                               connect_handler(boost::ref(socket), error);
                         });
    io_service.run();
    return 0;
}
