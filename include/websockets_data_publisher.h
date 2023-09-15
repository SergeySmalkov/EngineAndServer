//
// Created by cheytakker on 15.09.23.
//

#ifndef MATCHINGENGINE_WEBSOCKETS_DATA_PUBLISHER_H
#define MATCHINGENGINE_WEBSOCKETS_DATA_PUBLISHER_H

#include <websocketpp/server.hpp>
#include <thread>
#include <websocketpp/config/asio_no_tls.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

void on_open(server* s, websocketpp::connection_hdl hdl) {
    std::thread([s, hdl]() {
        while (true) {
            s->send(hdl, "Fake Data", websocketpp::frame::opcode::text);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

void publish() {
    server print_server;
    print_server.init_asio();
    print_server.set_open_handler(std::bind(&on_open, &print_server, std::placeholders::_1));
    print_server.listen(9002);
    print_server.start_accept();
    print_server.run();
}




#endif //MATCHINGENGINE_WEBSOCKETS_DATA_PUBLISHER_H
