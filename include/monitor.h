namespace reba::engine {
class Monitor {
public:
    Monitor();
    run();
    startWatchdog();
private:
    boost::asio::io_context io_context_;
}
}