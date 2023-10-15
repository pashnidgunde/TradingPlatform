#include <iostream>
#include <boost/asio.hpp>
#include "CSVFileReader.h"
#include "Tokenizer.h"
#include "Encoder.h"

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

        void send(const Message& message) {
            this->send(reinterpret_cast<const char *>(&message), sizeof(Message));
        }

        void send(const char* msg, int len) {
            socket_.send_to(boost::asio::buffer(msg, len), endpoint_);
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
    Encoder encoder;
    boost::asio::io_service io_service;
    client::UDPClient client(io_service, "localhost", "1234");

    for (const auto& instruction : instructions) {
        Message encoded = encoder.encode(instruction);
        client.send(encoded);
    }

    return 0;
}