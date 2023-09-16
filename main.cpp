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
#include "worker_tasks.h"
#include <fstream>

int main() {

    std::thread ws_thread([&](){
        publish();
    });
    ws_thread.detach();


    // Global THREADPOOL
    boost::asio::io_service io_service;
    // Use io_service with multiple worker threads
    const int NUM_WORKERS = 2;
    boost::asio::io_service::work work(io_service);  // This ensures io_service doesn't exit if it has no more work

    for (int i = 0; i < NUM_WORKERS; i++) {
        std::thread worker([&]() {
            io_service.run();
        });
        worker.detach();
    }


    zmq::context_t context(1);
    // Socket to send order updates
    zmq::socket_t update_socket(context, zmq::socket_type::push);
    update_socket.connect("tcp://localhost:5556");


    std::thread zmq_thread([&]() {
        zmq_accept_orders(context, update_socket, io_service);
    });
    zmq_thread.detach();

    std::ifstream infile("../price_updates.txt");
    std::string line;
    // Skip the header line
    std::getline(infile, line);

    std::mutex price_mutex;
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::universal_time();

    while (std::getline(infile, line)) {
        double relative_time, best_bid, best_ask;
        sscanf(line.c_str(), "%lf\t%lf\t%lf", &relative_time, &best_bid, &best_ask);

        boost::posix_time::time_duration td = boost::posix_time::milliseconds(static_cast<int64_t>(relative_time * 1000));
        boost::posix_time::ptime scheduled_time = start_time + td;

        // Schedule using boost::asio::deadline_timer
        std::shared_ptr<boost::asio::deadline_timer> timer = std::make_shared<boost::asio::deadline_timer>(io_service, scheduled_time);
        timer->async_wait([&price_mutex, relative_time, best_bid, best_ask, timer](const boost::system::error_code& /*e*/) {
            update_price(relative_time, best_bid, best_ask, std::ref(price_mutex));
        });
    }

    while (true) {
        std::cout << "main is live \n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Join the zmq_thread if needed
    zmq_thread.join();
    ws_thread.join();

    return 0;

}
