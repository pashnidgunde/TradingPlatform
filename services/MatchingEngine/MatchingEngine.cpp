#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "WireFormat.h"
#include "MessageQueue.h"
#include <algorithm>

using boost::asio::ip::udp;
using boost::asio::ip::address;

namespace {

    class MatchingEngine {
    public:
        MatchingEngine() {
            _socket.open(udp::v4());
            _socket.bind(udp::endpoint(address::from_string(IPADDRESS), UDP_PORT));
            startReceive();
            io_service.run();
        }

    private:
        void startReceive() {
            _socket.async_receive_from(boost::asio::buffer(_recvBuffer),
                                       _remoteEndpoint,
                                       boost::bind(&MatchingEngine::handleReceive,
                                                   this,
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred));
        }

        void handleReceive(const boost::system::error_code &error,
                           std::size_t bytes_transferred) {
            if (error) {
                std::cout << "Receive failed: " << error.message() << "\n";
                return;
            }

            //std::cout << "Received: '" << std::string(_recvBuffer.begin(), _recvBuffer.begin()+bytes_transferred) << "' (" << error.message() << ")\n";
            std::string str(_recvBuffer.begin(), _recvBuffer.begin() + bytes_transferred);
            const auto *m = reinterpret_cast<const Message *>(str.c_str());
            msgQueue.incoming(m);

            startReceive();
        }

        boost::asio::io_service io_service;
        udp::socket _socket{io_service};
        udp::endpoint _remoteEndpoint;
        std::array<char, 128> _recvBuffer{};
        const std::string IPADDRESS = "127.0.0.1";
        const long UDP_PORT = 13251;

        platform::MessageQueue msgQueue;
    };

}  // namespace

int main() {
    try {
        MatchingEngine server;
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }
    return 0;
}