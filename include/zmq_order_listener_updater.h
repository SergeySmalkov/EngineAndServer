//
// Created by cheytakker on 15.09.23.
//

#ifndef MATCHINGENGINE_ZMQ_ORDER_LISTENER_UPDATER_H
#define MATCHINGENGINE_ZMQ_ORDER_LISTENER_UPDATER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <chrono>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>


void zmq_accept_orders() {
    zmq::context_t context(1);
    // Socket to receive orders
    zmq::socket_t order_socket(context, zmq::socket_type::pull);
    order_socket.bind("tcp://*:5555");
    // Socket to send order updates
    zmq::socket_t update_socket(context, zmq::socket_type::push);
    update_socket.connect("tcp://localhost:5556");

    while (true) {
        zmq::message_t request;

        // Wait for next request from client
        order_socket.recv(&request);
        std::string order_data = std::string(static_cast<char*>(request.data()), request.size());

        std::cout << "Received Order: " << order_data << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::string update_msg = "NEW";
        zmq::message_t update_new(update_msg.data(), update_msg.size());
        update_socket.send(update_new, zmq::send_flags::none);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        update_msg = "PENDING";
        zmq::message_t update_pending(update_msg.data(), update_msg.size());
        update_socket.send(update_pending, zmq::send_flags::none);
    }
}
#endif //MATCHINGENGINE_ZMQ_ORDER_LISTENER_UPDATER_H