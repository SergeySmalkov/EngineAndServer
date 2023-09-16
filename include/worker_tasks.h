//
// Created by cheytakker on 16.09.23.
//

#ifndef MATCHINGENGINE_WORKER_TASKS_H
#define MATCHINGENGINE_WORKER_TASKS_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <chrono>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>

void update_price(double relative_time, double best_bid, double best_ask, std::mutex& price_mutex) {
    std::lock_guard<std::mutex> lock(price_mutex);
//    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(relative_time * 1000)));
    std::cout << "Updating price after " << relative_time << " seconds to: BID=" << best_bid << ", ASK=" << best_ask << "\n";
}

void process_order(const std::string& order_data, zmq::socket_t& update_socket) {
    // Simulate some processing
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string update_msg = "ACCEPTED for " + order_data;
    zmq::message_t update(update_msg.data(), update_msg.size());
    update_socket.send(update, zmq::send_flags::none);
}

#endif //MATCHINGENGINE_WORKER_TASKS_H
