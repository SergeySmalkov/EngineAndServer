//
// Created by cheytakker on 19.09.23.
//

#ifndef MATCHINGENGINE_WEBSOCKETLIB_H
#define MATCHINGENGINE_WEBSOCKETLIB_H
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <chrono>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include "worker_tasks.h"
#include <fstream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>
#include <mutex>
#include <list>
#include <set>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

class Broadcaster {
    std::set<std::shared_ptr<websocket::stream<tcp::socket>>> subscribers_;
    std::mutex mutex_;

public:
    Broadcaster(const Broadcaster&) = delete;
    Broadcaster& operator=(const Broadcaster&) = delete;
    Broadcaster() = default;

    void register_session(std::shared_ptr<websocket::stream<tcp::socket>> session) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.insert(session);
    }

    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& subscriber : subscribers_) {
            subscriber->async_write(boost::asio::buffer(message), [](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << "connection closed by the client\n";
                }
            });
        }
    }
};


class Session : public std::enable_shared_from_this<Session> {
    std::shared_ptr<websocket::stream<tcp::socket>> ws_;
    std::shared_ptr<Broadcaster> broadcaster_;

public:
    explicit Session(tcp::socket socket, std::shared_ptr<Broadcaster> broadcaster)
            : ws_(std::make_shared<websocket::stream<tcp::socket>>(std::move(socket))),
              broadcaster_(broadcaster)  {}

    void start() {
        ws_->async_accept([sp = shared_from_this()](boost::system::error_code ec) {
            if (!ec) {
                sp->broadcaster_->register_session(sp->ws_);

                std::string message = "Starting Market Data Feed \n";
                sp->broadcaster_->broadcast(message);
            }
        });
    }
};

class Server {
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::shared_ptr<Broadcaster> broadcaster_;

public:
    Server(boost::asio::io_context& io_context, const tcp::endpoint& endpoint, std::shared_ptr<Broadcaster> broadcaster)
            : acceptor_(io_context, endpoint), socket_(io_context), broadcaster_(broadcaster) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket_), broadcaster_)->start();
                socket_ = tcp::socket(static_cast<boost::asio::io_context&>(acceptor_.get_executor().context()));
                do_accept();
            }
        });
    }
};

#endif //MATCHINGENGINE_WEBSOCKETLIB_H
