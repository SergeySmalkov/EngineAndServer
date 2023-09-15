// cpp_server.cpp

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <thread>
#include <chrono>

typedef websocketpp::server<websocketpp::config::asio> server;

void on_open(server* s, websocketpp::connection_hdl hdl) {
    std::thread([s, hdl]() {
        while (true) {
            s->send(hdl, "Fake Data", websocketpp::frame::opcode::text);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

int main() {
    server print_server;

    print_server.init_asio();

    print_server.set_open_handler(std::bind(&on_open, &print_server, std::placeholders::_1));

    print_server.listen(9002);
    print_server.start_accept();

    print_server.run();
}
