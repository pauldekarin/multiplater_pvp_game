#include "../client.hpp"


Client::Client(const std::string &uri):uri(uri), is_running(false), connection_failed(false), is_open(false), client_id(-1), room_id(-1)
{

}

void Client::set_message_callback(std::function<void(const nlohmann::json&)> &&func){
    this->on_message_callback = std::forward<std::function<void(const nlohmann::json&)> >(func);
}
void Client::set_start_callback(std::function<void(const nlohmann::json&)> &&func){
    this->on_start_callback = std::forward<std::function<void(const nlohmann::json&)> >(func);
}

bool Client::is_connected_to_server(){
    return this->client_id >= 0;
}

bool Client::is_connected_to_playroom(){
    return this->room_id >= 0;
}

void Client::connect(){
    try{
        this->client.init_asio();
        
        this->client.set_message_handler(std::bind(&Client::on_message, this, std::placeholders::_1, std::placeholders::_2));
        // this->client.set_open_handler(std::bind(&Client::on_open, this, std::placeholders::_1, &this->client));
        this->client.set_fail_handler(std::bind(&Client::on_fail, this, std::placeholders::_1));
        this->client.set_close_handler(std::bind(&Client::on_close, this, std::placeholders::_1));
        
        websocketpp::lib::error_code ec;
        this->connection_ptr = this->client.get_connection(this->uri, ec);

        if(ec){
            std::cout << "[CLIENT THREAD] error get connection: " << ec.message() << std::endl;
        }
        this->connection_ptr->append_header("User-Agent", "Websocket++");
        this->client.connect(this->connection_ptr); 
    }catch(websocketpp::exception ec){

    }
}

void Client::run(){
    if(this->is_running.load(std::memory_order_relaxed)) return;

    this->is_running.store(true, std::memory_order_relaxed);
    this->running_thread = std::make_shared<std::thread>(&Client::running_thread_handler, this);
    this->sending_thread = std::make_shared<std::thread>(&Client::sending_thread_handler, this);
}

void Client::send(const std::string &msg){
    if(!this->is_open.load(std::memory_order_relaxed)) return;
    std::cout << msg << std::endl;
    std::unique_lock<std::mutex> locker(this->mtx);
    this->buffer.push(msg);
    this->cv.notify_all();
}

void Client::send(const nlohmann::json &buffer){
    if(!this->is_open.load(std::memory_order_relaxed)) return;
    std::cout << buffer.dump(4) << std::endl;

    std::unique_lock<std::mutex> locker(this->mtx);
    this->buffer.push(buffer);
    this->cv.notify_all();
}

void Client::close(){
    if(this->is_running.load(std::memory_order_relaxed)){
        this->is_running.store(false, std::memory_order_relaxed);
        this->cv.notify_all();

        websocketpp::lib::error_code ec;

        if(
            this->connection_ptr->get_state() != websocketpp::session::state::closed &&
            this->connection_ptr->get_state() != websocketpp::session::state::closing
        ){
            this->client.stop_perpetual();
            this->client.close(this->connection_ptr->get_handle(), websocketpp::close::status::going_away, "", ec);
            if(ec){
                std::cerr << "Error closing listening thread: " << ec.message() << std::endl;
            }
        }

        if(this->running_thread && this->running_thread->joinable()){
            this->running_thread->join();
        }
        if(this->sending_thread && this->sending_thread->joinable()){
            this->sending_thread->join();
        }
        if(this->connection_thread && this->connection_thread->joinable()){
            this->connection_thread->join();
        }
    }
}

void Client::async_connect_and_run(){
    if(this->is_running.load(std::memory_order_relaxed)) return;

    this->is_running.store(true, std::memory_order_relaxed);
    this->connection_thread = std::make_shared<std::thread>(&Client::async_connect_and_run_handler, this);
}

std::shared_ptr<boost::asio::ssl::context> Client::on_tls_init(const char * hostname, websocketpp::connection_hdl) {
     std::shared_ptr<boost::asio::ssl::context> ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

     try {
              ctx->set_options(boost::asio::ssl::context::default_workarounds |
                               boost::asio::ssl::context::no_sslv2 |
                               boost::asio::ssl::context::no_sslv3 |
                               boost::asio::ssl::context::single_dh_use);
              ctx->set_verify_mode(boost::asio::ssl::verify_none);
          } catch (std::exception& e) {
              std::cout << "TLS Initialization Error: " << e.what() << std::endl;
          }
     return ctx;
}

void Client::async_connect_and_run_handler(){
    static bool is_first_attempt_connection = true;

    std::function<void()> wait_for_another_attempt = [this](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    };

    while(this->is_running.load(std::memory_order_relaxed)){
        std::cout << "[CONNECTION THREAD] Trying to connect..." << std::this_thread::get_id() << std::endl;
        this->connection_failed.store(false, std::memory_order_relaxed);
        try{
            if(is_first_attempt_connection){
                this->client.set_access_channels(websocketpp::log::alevel::all);
                this->client.clear_access_channels(websocketpp::log::alevel::frame_payload);
                this->client.set_error_channels(websocketpp::log::elevel::all);

                this->client.init_asio();

                this->client.set_message_handler(std::bind(&Client::on_message, this, std::placeholders::_1, std::placeholders::_2));
                this->client.set_tls_init_handler(std::bind(&Client::on_tls_init, "second-quick-armadillo.glitch.me", std::placeholders::_1));
                this->client.set_open_handler(std::bind(&Client::on_open, this, std::placeholders::_1, &this->client));
                this->client.set_fail_handler(std::bind(&Client::on_fail, this, std::placeholders::_1));
                this->client.set_close_handler(std::bind(&Client::on_close, this, std::placeholders::_1));
                
                is_first_attempt_connection = false;
            }else{
                this->client.reset();
            }

            websocketpp::lib::error_code ec;
            this->connection_ptr = this->client.get_connection(this->uri, ec);
            
            if(ec){
                std::cerr << "[CONNECTION THREAD] error connected: " << ec.message() << std::endl;
                wait_for_another_attempt();
                continue;
            }
            this->connection_ptr->append_header("user-agent", "Websocket++");
            this->client.connect(this->connection_ptr);
            this->client.run();

            if(!this->is_running.load(std::memory_order_relaxed)) break;

            wait_for_another_attempt();
        }catch(websocketpp::exception ec){
            std::cerr << "[CONNECTION THREAD] exception: " << ec.what() << std::endl;
            wait_for_another_attempt();
        }
    }

    std::cout << "[CONNECTION THREAD] STOP" << std::endl;
}

void Client::running_thread_handler() {
    std::cout << "[RUNNING THREAD] START" << std::endl;
    try {
        this->client.run();
    } catch (const websocketpp::exception& e) {
        std::cerr << "Exception in running_thread_handler: " << e.what() << std::endl;
    }
    std::cout << "[RUNNING THREAD] STOP" << std::endl;
}

void Client::sending_thread_handler(){
    std::cout << "[SENDING THREAD] START" << std::endl;
    while(this->is_running.load(std::memory_order_relaxed) && this->is_open.load(std::memory_order_relaxed)){
        if(this->connection_ptr->get_state() != websocketpp::session::state::open){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        std::unique_lock<std::mutex> locker(this->mtx);

        this->cv.wait(locker, [this](){
            return (!this->buffer.empty() || !this->is_running.load(std::memory_order_relaxed) || !this->is_open.load(std::memory_order_relaxed));
        });

        while(!this->buffer.empty()){
            std::variant<std::string, nlohmann::json> &data = this->buffer.front();
            nlohmann::json js_data;

            if(std::get_if<std::string>(&data)){
                js_data = nlohmann::json::parse(std::get<std::string>(data));
            }else if(std::get_if<nlohmann::json>(&data)){
                js_data = std::get<nlohmann::json>(data);
            }

            js_data["room_id"] = this->room_id.load(std::memory_order_relaxed);
            js_data["client_id"] = this->client_id.load(std::memory_order_relaxed);
            
            websocketpp::lib::error_code ec = this->connection_ptr->send(js_data.dump(), websocketpp::frame::opcode::text);
            this->buffer.pop();
            if(ec){
                std::cerr << "[SENDING THREAD] error sending message: " << ec.message() << std::endl;
            }else{

            }
        }   
    }
    std::cout << "[SENDING THREAD] STOP" << std::endl;
}

void Client::on_open(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_tls_client> *c){
    std::cout << "Connection passed" << std::endl;

    websocketpp::lib::error_code ec;
    websocketpp::client<websocketpp::config::asio_tls_client>::connection_ptr con = c->get_con_from_hdl(hdl, ec);
    if(ec){
        std::cerr << "Failed to get connection pointer: " << ec.message() << std::endl;
        return;
    }
    this->is_open.store(true, std::memory_order_relaxed);
    this->cv.notify_all();
    this->sending_thread = std::make_shared<std::thread>(&Client::sending_thread_handler, this);
}

void Client::on_message(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg){
    nlohmann::json buffer = nlohmann::json::parse(msg->get_payload());

    if(buffer["type"] == "set_id"){
        this->client_id.store(buffer["id"], std::memory_order_relaxed);
    }else if(buffer["type"] == "set_room"){
        this->room_id.store(buffer["id"], std::memory_order_relaxed);
        buffer["client_id"] = this->client_id.load(std::memory_order_relaxed);
        if(this->on_start_callback){
            this->on_start_callback(buffer);
        }
    }else if(buffer["type"] == "update"){
        if(this->on_message_callback){
            this->on_message_callback(buffer);
        }
    }
}

void Client::on_fail(websocketpp::connection_hdl hdl){
    this->connection_failed.store(true, std::memory_order_relaxed);
    this->cv.notify_all();
    std::cerr << "WebSocket connection failed!" << std::endl;
}

void Client::on_close(websocketpp::connection_hdl hdl){
    this->is_open.store(false, std::memory_order_relaxed);
    this->connection_failed.store(true, std::memory_order_relaxed);
    this->cv.notify_all();

    if(this->sending_thread && this->sending_thread->joinable()){
        this->sending_thread->join();
    }
}
