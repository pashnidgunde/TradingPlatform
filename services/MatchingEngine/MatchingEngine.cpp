#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "WireFormat.h"
#include <algorithm>
#include <MemoryMappedIO.h>
#include <thread>
#include "NewMessageHandler.h"

using boost::asio::ip::udp;
using boost::asio::ip::address;

namespace {

    class MatchingEngine {
    public:
        MatchingEngine() {

            consumerThread = std::thread{&MatchingEngine::forwarder, this};

            _socket.open(udp::v4());
            _socket.bind(udp::endpoint(address::from_string(IPADDRESS), UDP_PORT));
            startReceive();
            io_service.run();
        }

        ~MatchingEngine() {
            consumerThread.join();
        }

    private:
        void startReceive() {
            _next_file_offset = mfile.nextReceive();
            _socket.async_receive_from(boost::asio::buffer(_next_file_offset, sizeof(Message)),
                                       _remoteEndpoint,
                                       boost::bind(&MatchingEngine::handleReceive,
                                                   this,
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred));
        }

        void forwarder() {
            while(true) {
                bool any_more_to_send = mfile.anyMoreToSend();
                if (!any_more_to_send) continue;
                const auto *m = reinterpret_cast<const Message *>(mfile.nextToSend());
                handler.onIncoming(m);
                mfile.advanceSent();
            }

        }

        void handleReceive(const boost::system::error_code &error,
                           std::size_t /*bytes_transferred*/) {
            if (error) {
                std::cout << "Receive failed: " << error.message() << "\n";
                startReceive();
                return;
            }
            mfile.advanceReceive();
            startReceive();
        }


        boost::asio::io_service io_service;
        udp::socket _socket{io_service};
        udp::endpoint _remoteEndpoint;
        char *_next_file_offset = nullptr;
        const std::string IPADDRESS = "127.0.0.1";
        const long UDP_PORT = 1234;

        MemoryMappedFile mfile;
        std::thread consumerThread{};
        NewMessageHandler handler;
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