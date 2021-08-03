//importing libraries
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <cstdint>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>

#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(s) ((boost::asio::io_context&)(s).get_executor().context())
#else
#define GET_IO_SERVICE(s) ((s).get_io_service())
#endif

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;

class con_handler : public boost::enable_shared_from_this<con_handler>
{
private:
    tcp::socket sock;
    enum { max_length = sizeof(double)*2 };
    std::uint8_t data[max_length];

public:
    typedef boost::shared_ptr<con_handler> pointer;

    con_handler(boost::asio::io_service& io_service)
    : sock(io_service){
    }

    con_handler(const con_handler&) = delete;
    con_handler(con_handler&&) = delete;
    ~con_handler() = default;

    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new con_handler(io_service));
    }

    tcp::socket& socket()
    {
        return sock;
    }

    void start()
    {
        auto p = shared_from_this();
        socket().async_receive(boost::asio::buffer(this->data, max_length),
                                [p](const boost::system::error_code& error,
                                    std::size_t bytes_transferred) {
                                    p->handle_read(error, bytes_transferred);
                                });
    }

    void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (err) {
            std::cerr << "error: " << err.message() << std::endl;
            socket().close();
            return;
        }
        std::cout << "[" << bytes_transferred << "] bytes have been received" << std::endl;
        double a1;
        double a2;

        std::memcpy(&a1, data, sizeof(double));
        std::memcpy(&a2, data + sizeof(double), sizeof(double));

        double res = a1 + a2;

        std::cout << "Result: " << a1 << " + " << a2 << " = " << res << std::endl;

        std::memcpy(data, &res, double(res));

        auto p = shared_from_this();

        socket().async_send(boost::asio::buffer(data, double(res)),
                            [p](const boost::system::error_code& error,
                                    std::size_t bytes_transferred) {
                                p->handle_write(error, bytes_transferred);
                            });
    }

    void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (err) {
            std::cerr << "error: " << err.message() << std::endl;
        } else {
            std::cout << "Processing & data send operation both are over." << std::endl;
        }
        socket().close();
    }
};

class Server
{
private:
   tcp::acceptor acceptor_;
   void start_accept()
   {
    // socket
     con_handler::pointer connection = con_handler::create(GET_IO_SERVICE(acceptor_));

    // asynchronous accept operation and wait for a new connection.
//      acceptor_.async_accept(connection->socket(),
//         boost::bind(&Server::handle_accept, this, connection,
//         boost::asio::placeholders::error));
     // boost::shared_ptr<tcp::socket> pSock(new tcp::socket(GET_IO_SERVICE(acceptor_));

     acceptor_.async_accept(connection->socket(),
                            [this, connection](const boost::system::error_code& error) {
                                handle_accept(connection, error);
                            });
  }

public:
//constructor for accepting connection from client
    Server(boost::asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), 8080))
    {
        start_accept();
    }
    void handle_accept(con_handler::pointer connection, const boost::system::error_code& err)
    {
        if (!err) {
            connection->start();
        }
        start_accept();
    }
};

int main(int argc, char *argv[])
{
    try
    {
        boost::asio::io_service io_service;
        Server server(io_service);
        io_service.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << endl;
    }

    return 0;
}
