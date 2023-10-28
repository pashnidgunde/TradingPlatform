#include <iostream>
#include <boost/asio.hpp>
#include "CSVFileReader.h"
#include "Tokenizer.h"
#include "Encoder.h"

namespace client {
    using boost::asio::ip::udp;
    using boost::asio::ip::address;

    class UDPClient {
    public:
        UDPClient() : socket_(io_service_) {
            socket_.open(udp::v4());
        }

        ~UDPClient() {
            socket_.close();
        }

        void send(const Message &message) {
            this->send(reinterpret_cast<const char *>(&message), sizeof(Message));
        }

        void send(const char *msg, int len) {
            socket_.send_to(boost::asio::buffer(msg, len), endpoint_);
        }

    private:
        const std::string IPADDRESS = "127.0.0.1";
        const long UDP_PORT = 1234;
        boost::asio::io_service io_service_;
        udp::socket socket_;
        udp::endpoint endpoint_ = udp::endpoint(address::from_string(IPADDRESS), UDP_PORT);
    };
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>";
        return 1;
    }

    client::CSVFileReader fileReader(argv[1]);
    auto instructions = fileReader.instructions();
    client::UDPClient client;

    for (const auto &instruction: instructions) {
        auto encoded = Encoder::encode(instruction);
        if (!encoded.has_value()) {
            std::cerr << "Failed to encode : " << instruction << std::endl;
            continue;
        }
        client.send(encoded.value());
    }

    return 0;
}