#ifndef CLIENT_HPP
#define CLIENT_HPP


#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <functional>
#include <iostream>
#include <thread>
#include <atomic>
#include <queue>

#include "external/json/include/nlohmann/json.hpp"

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

class Client: public std::enable_shared_from_this<Client>{
public:
    Client(const std::string &uri);

    void connect();

    void run();

    void send(const std::string &msg);
    void send(const nlohmann::json &buffer);

    void close();

    void async_connect_and_run();

    bool is_connected_to_server();
    bool is_connected_to_playroom();

    void set_start_callback(std::function<void(const nlohmann::json&)> &&func);
    void set_message_callback(std::function<void(const nlohmann::json&)> &&func);
private:
    void async_connect_and_run_handler();

    websocketpp::client<websocketpp::config::asio_tls_client> client;
    websocketpp::client<websocketpp::config::asio_tls_client>::connection_ptr connection_ptr;

    std::string uri;

    std::shared_ptr<std::thread> running_thread;
    std::shared_ptr<std::thread> sending_thread;
    std::shared_ptr<std::thread> connection_thread;

    std::condition_variable cv;
    std::queue<std::variant<std::string, nlohmann::json>> buffer;
    std::mutex mtx;

    std::atomic<bool> is_running;
    std::atomic<bool> connection_failed;
    std::atomic<bool> is_open;

    std::atomic<std::int64_t> client_id;
    std::atomic<std::int64_t> room_id;

    std::function<void(const nlohmann::json&)> on_message_callback;
    std::function<void(const nlohmann::json&)> on_start_callback;

    void running_thread_handler();

    void sending_thread_handler();

    void on_open(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_tls_client> *c);

    void on_message(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg);

    void on_fail(websocketpp::connection_hdl hdl);

    void on_close(websocketpp::connection_hdl hdl);

    static std::shared_ptr<boost::asio::ssl::context> on_tls_init(const char * hostname, websocketpp::connection_hdl);
};


#endif