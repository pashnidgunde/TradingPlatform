#include <iostream>
#include <boost/asio.hpp>
#include "CSVFileReader.h"
#include "Tokenizer.h"

namespace client {
    using boost::asio::ip::udp;

    class UDPClient {
    public:
        UDPClient(
                boost::asio::io_service &io_service,
                const std::string &host,
                const std::string &port
        ) : io_service_(io_service), socket_(io_service, udp::endpoint(udp::v4(), 0)) {
            udp::resolver resolver(io_service_);
            udp::resolver::query query(udp::v4(), host, port);
            udp::resolver::iterator iter = resolver.resolve(query);
            endpoint_ = *iter;
        }

        ~UDPClient() {
            socket_.close();
        }

        void send(const std::string &msg) {
            socket_.send_to(boost::asio::buffer(msg, msg.size()), endpoint_);
        }

    private:
        boost::asio::io_service &io_service_;
        udp::socket socket_;
        udp::endpoint endpoint_;
    };

}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>";
        return 1;
    }

    client::CSVFileReader fileReader(argv[1]);
    auto instructions = fileReader.instructions();


    boost::asio::io_service io_service;
    client::UDPClient client(io_service, "localhost", "1234");
    client.send("Hello");
    return 0;
}