#include <boost/asio.hpp>
#include <cstdio>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
    boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

awaitable<void> echo(tcp::socket socket)
{
    try
    {
        char data[1024];

        for (;;)
        {
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
            co_await socket.async_send(boost::asio::buffer(data, n), use_awaitable);
        }
    }
    catch (boost::system::system_error& e)
    {
        if (e.code() != boost::asio::error::eof)
        {
            std::printf("System Error: %s\n", e.what());
        }
    }
}

awaitable<void> listener(tcp::endpoint const& endpoint)
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, endpoint);

    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        const char* local_ip = argc < 3 ? "0.0.0.0" : argv[1];
        const char* port = argc < 3 ? argc < 2 ? "8080" : argv[1] : argv[2];

        tcp::endpoint endpoint(
            boost::asio::ip::make_address_v4(local_ip),
            std::atoi(port));

        boost::asio::io_context io_context(1);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

        signals.async_wait([&](auto, auto){ io_context.stop(); });
        co_spawn(io_context, listener(endpoint), detached);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::printf("Exception: %s\n", e.what());
    }
}
