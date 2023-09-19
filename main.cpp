// cpp_server.cpp


#include <iostream>
#include <chrono>
#include <zmq.hpp>
#include <string>
#include <thread>
#include "worker_tasks.h"
#include <fstream>
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include "WebsocketLib.h"
#include "ZMQLib.h"

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

int main() {
//WEBSOCKETS BROADCAST OF MARKET DATA, ACCEPTING ANY AMOUNT OF NEW CONNECTIONS VIA CONTEXT:
    // Asio setup
    boost::asio::io_context io_context;
    tcp::endpoint endpoint(tcp::v4(), 8080);
    std::shared_ptr<Broadcaster> broadcaster = std::make_shared<Broadcaster>();
    Server server(io_context, endpoint, broadcaster);

    // Run io_context in a separate thread
    std::thread io_context_thread([&io_context]() {
        io_context.run();
    });

//
//    // Simulating sending messages in intervals
//    for (int i = 0; i < 10; ++i) { // sending 10 messages
//        std::this_thread::sleep_for(std::chrono::seconds(3));  // sleep for 5 seconds
//        std::string message = "Market data update " + std::to_string(i + 1);
//        broadcaster->broadcast(message);
//    }



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

//ZMQ SERVER TO ACCEPT ORDERS AND REGISTER THEM IN THREADPOOL FOR EXECUTION
    ZMQServer zmqserver(io_service);
    std::thread serverThread(&ZMQServer::run, &zmqserver);


//READ TXT FILE AND SCHEDULE UPDATE OF PRICES TO THE THREADPOOL
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
        timer->async_wait([&price_mutex, relative_time, best_bid, best_ask, timer, broadcaster](const boost::system::error_code& /*e*/) {
            update_price(broadcaster, relative_time, best_bid, best_ask, std::ref(price_mutex));
        });
    }

    while (true) {
        std::cout << "main is live \n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Join the zmq_thread if needed
    serverThread.join();
    // Stopping and joining the thread running io_context
    io_context.stop();
    if (io_context_thread.joinable()) {
        io_context_thread.join();
    }
    return 0;

}
