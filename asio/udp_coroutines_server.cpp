#include <boost/asio.hpp>
#include <cstdio>

using boost::asio::ip::udp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

awaitable<void> echo(udp::socket socket)
{
    try
    {
        char data[1024];
        udp::endpoint sender_endpoint;

        for (;;)
        {
            std::size_t n = co_await socket.async_receive_from(boost::asio::buffer(data), sender_endpoint, use_awaitable);
            co_await socket.async_send_to(boost::asio::buffer(data, n), sender_endpoint, use_awaitable);
        }
    }
    catch (std::exception& e)
    {
        std::printf("echo Exception: %s\n", e.what());
    }
}

int main(int argc, char* argv[])
{
    try
    {
        const char* local_ip = argc < 3 ? "0.0.0.0" : argv[1];
        const char* port = argc < 3 ? argc < 2 ? "8080" : argv[1] : argv[2];

        udp::endpoint endpoint(
            boost::asio::ip::make_address_v4(local_ip),
            std::atoi(port));

        boost::asio::io_context io_context(1);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        udp::socket socket(io_context, endpoint);

        signals.async_wait([&](auto, auto){ io_context.stop(); });
        co_spawn(io_context, echo(std::move(socket)), detached);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::printf("Exception: %s\n", e.what());
    }
}
