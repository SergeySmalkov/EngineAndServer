//
// Created by cheytakker on 19.09.23.
//

#ifndef MATCHINGENGINE_ZMQLIB_H
#define MATCHINGENGINE_ZMQLIB_H


#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <vector>

class ZMQServer {
private:
    zmq::context_t context_;
    std::vector<zmq::socket_t> sockets_;
    std::vector<zmq::socket_t> update_status_sockets_;
    boost::asio::io_service& io_service_;  // Reference to the io_service

    void handleMessageFromClient1(const std::string& message) {
        // Handle message from client 1
        std::cout << "Received from Client 1: " << message << std::endl;
        std::string update_msg = "NEW";
        zmq::message_t update_new(update_msg.data(), update_msg.size());
        update_status_sockets_[0].send(update_new, zmq::send_flags::none);

        // Post the task directly to the io_service for processing
        io_service_.post(std::bind(process_order, message, std::ref(update_status_sockets_[0])));
    }

    void handleMessageFromClient2(const std::string& message) {
        // Handle message from client 2
        std::cout << "Received from Client 2: " << message << std::endl;
        std::string update_msg = "NEW";
        zmq::message_t update_new(update_msg.data(), update_msg.size());
        update_status_sockets_[1].send(update_new, zmq::send_flags::none);

        // Post the task directly to the io_service for processing
        io_service_.post(std::bind(process_order, message, std::ref(update_status_sockets_[1])));
    }

public:
    ZMQServer(boost::asio::io_service& io_service) : context_(1), sockets_(), io_service_(io_service) {
        // Set up the sockets for the two clients
        sockets_.emplace_back(context_, zmq::socket_type::pull);
        sockets_.emplace_back(context_, zmq::socket_type::pull);
        sockets_[0].bind("tcp://*:5555");  // Client 1
        sockets_[1].bind("tcp://*:5557");  // Client 2
        update_status_sockets_.emplace_back(context_, zmq::socket_type::push);
        update_status_sockets_.emplace_back(context_, zmq::socket_type::push);
        update_status_sockets_[0].connect("tcp://localhost:5556"); // Client 1
        update_status_sockets_[1].connect("tcp://localhost:5558"); // Client 2
    }

    void run() {
        // Prepare poll items
        std::vector<zmq::pollitem_t> poll_items;
        for (auto& socket : sockets_) {
            poll_items.push_back({socket, 0, ZMQ_POLLIN, 0});
        }

        while (true) {
            //Blocking!
            zmq::poll(poll_items, std::chrono::milliseconds(-1));  // Poll indefinitely until an event occurs

            for (int i = 0; i < poll_items.size(); ++i) {
                if (poll_items[i].revents & ZMQ_POLLIN) {
                    zmq::message_t zmq_msg;
                    sockets_[i].recv(zmq_msg, zmq::recv_flags::none);
                    std::string msg(static_cast<char*>(zmq_msg.data()), zmq_msg.size());

                    if (i == 0) {
                        handleMessageFromClient1(msg);
                    } else {
                        handleMessageFromClient2(msg);
                    }
                }
            }
        }
    }

};



#endif //MATCHINGENGINE_ZMQLIB_H
