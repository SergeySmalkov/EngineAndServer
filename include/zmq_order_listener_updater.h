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
#include "worker_tasks.h"


void zmq_accept_orders(zmq::context_t& context, zmq::socket_t& update_socket, boost::asio::io_service& io_service) {

    // Socket to receive orders
    zmq::socket_t order_socket(context, zmq::socket_type::pull);
    order_socket.bind("tcp://*:5555");

    while (true) {
        zmq::message_t request;
        // Wait for next request from client
        order_socket.recv(&request);
        std::string order_data = std::string(static_cast<char*>(request.data()), request.size());

        std::cout << "Received Order: " << order_data << std::endl;

//        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::string update_msg = "NEW";
        zmq::message_t update_new(update_msg.data(), update_msg.size());
        update_socket.send(update_new, zmq::send_flags::none);

        // Post the task directly to the io_service for processing
        io_service.post(std::bind(process_order, order_data, std::ref(update_socket)));

    }
}
#endif //MATCHINGENGINE_ZMQ_ORDER_LISTENER_UPDATER_H
