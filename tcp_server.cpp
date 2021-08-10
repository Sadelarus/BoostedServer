//importing libraries
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <cstdint>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>
#include <boost/program_options.hpp>
#include "includes.h"

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
    enum { max_length = sizeof(CalcData) };
    unData data;

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
        socket().async_receive(boost::asio::buffer(&data.args, sizeof(data.args)),
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

        double r = data.args.a + data.args.b;

        std::cout << "Result: " << data.args.a << " + " << data.args.b << " = " << r << std::endl;

        data.res.res = r;

        auto p = shared_from_this();

        socket().async_send(boost::asio::buffer(&data.res, sizeof(data.res)),
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
    Server(boost::asio::io_service& io_service, const std::string &addr, const std::uint16_t port)
    : acceptor_(io_service, tcp::endpoint(ip::make_address(addr.c_str()), port))
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
    std::string serverAddr = "127.0.0.1";
    std::uint16_t port = 8080;
    namespace po = boost::program_options;
    po::options_description desc("Application options");
    desc.add_options()
    ("help,h", "Show help")
    ("server-adr,s", po::value<std::string>(&serverAddr), "Server address")
    ("server-port,p", po::value<std::uint16_t>(&port), "Server port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);


    if (vm.count("help") || argc == 1) {
        std::cout << desc << std::endl;
        return 1;
    }

    try
    {
        boost::asio::io_service io_service;
        Server server(io_service, serverAddr, port);
        io_service.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << endl;
    }

    return 0;
}
