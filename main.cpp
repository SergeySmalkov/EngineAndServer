// cpp_server.cpp

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <chrono>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include "websockets_data_publisher.h"
#include "zmq_order_listener_updater.h"

int main() {

    std::thread ws_thread([&](){
        publish();
    });
    ws_thread.detach();

    zmq::context_t context(1);
    // Socket to send order updates
    zmq::socket_t update_socket(context, zmq::socket_type::push);
    update_socket.connect("tcp://localhost:5556");


    std::thread zmq_thread([&]() {
        zmq_accept_orders(context, update_socket);
    });
    zmq_thread.detach();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Join the zmq_thread if needed
    zmq_thread.join();
    ws_thread.join();

    return 0;

}
