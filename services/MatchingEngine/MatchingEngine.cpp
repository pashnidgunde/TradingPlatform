#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>


using boost::asio::ip::udp;

namespace {

    class MatchingEngine {
    public:
        MatchingEngine(boost::asio::io_service& io_service)
                : _socket(io_service, udp::endpoint(udp::v4(), 1111))
        {
            startReceive();
        }
    private:
        void startReceive() {
            _socket.async_receive_from(
                    boost::asio::buffer(_recvBuffer), _remoteEndpoint,
                    boost::bind(&MatchingEngine::handleReceive, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
        }

        void handleReceive(const boost::system::error_code& error,
                           std::size_t /*bytes_transferred*/) {
            if (!error || error == boost::asio::error::message_size) {
            }
            startReceive();
        }

        udp::socket _socket;
        udp::endpoint _remoteEndpoint;
        std::array<char, 1024> _recvBuffer;
    };

}  // namespace

int main() {
    try {
        boost::asio::io_service io_service;
        MatchingEngine server{io_service};
        io_service.run();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    return 0;
}