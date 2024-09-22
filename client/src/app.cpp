#include "../app.hpp"

App::App(){
    this->window = std::make_unique<sf::RenderWindow>(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Angry Rabbits");
    this->client = std::make_shared<Client>("wss://second-quick-armadillo.glitch.me/");
    this->clock = std::make_unique<sf::Clock>();
    this->screen_manager = std::make_unique<ScreenManager>();

    std::shared_ptr<GameScreen> game_screen = std::make_shared<GameScreen>();
    this->screen_manager->append(game_screen);

    std::shared_ptr<MenuScreen> menu_screen = std::make_shared<MenuScreen>("menu_screen", std::bind(&App::start_game, this));
    this->screen_manager->append(menu_screen);

    this->client->set_message_callback(std::bind(&World::apply_info, World::Instance(), std::placeholders::_1));
    this->client->set_start_callback(std::bind(&World::connect_room, World::Instance(), std::placeholders::_1));
}

App::~App(){
    this->client->close();
}

void App::start_game(){
    this->screen_manager->setActive("game_screen");

    World::Instance()->start_game();
}
void App::loop(){
    this->client->async_connect_and_run();

    while(this->window->isOpen()){
        sf::Event event;

        while(this->window->pollEvent(event)){
            if(event.type == sf::Event::Closed){
                this->window->close();
            }

            if(event.type == sf::Event::KeyPressed){
                this->on_key_press_event(event);
            }

            if(event.type == sf::Event::KeyReleased){
                this->on_key_release_event(event);
            }

            if(event.type == sf::Event::MouseButtonPressed){
                this->on_mouse_pressed(event);
            }

            if(event.type == sf::Event::MouseButtonReleased){
                this->on_mouse_released(event);
            }

        }

        this->update();
        this->draw();
    }
}

void App::update(){
    std::float_t delta = this->clock->restart().asSeconds();
    this->ellapsed_time += delta;
    this->screen_manager->get_active()->update(delta);

    if(this->ellapsed_time >= 0.2f){
        this->ellapsed_time = 0;

        if(this->client->is_connected_to_playroom()){
            nlohmann::json game_info = World::Instance()->get_info();
            game_info["type"] = "update";

            this->client->send(game_info);
        }
    }
}

void App::draw(){
    this->window->clear();
    this->screen_manager->get_active()->draw(*this->window);
    this->window->display();
}   

nlohmann::json convert_event_to_json(const sf::Event &event){
    nlohmann::json js = {
        {"type", event.type},
        {"key_code" , event.key.code}
    };

    return js;
}

void App::on_key_press_event(const sf::Event &event){
    this->screen_manager->get_active()->on_key_press_event(event);
}

void App::on_key_release_event(const sf::Event &event){
    this->screen_manager->get_active()->on_key_release_event(event);

    // if(this->client->is_connected_to_playroom()){
    //     nlohmann::json game_info = World::Instance()->get_info();
    //     game_info["type"] = "update";
    //     game_info["event"] = convert_event_to_json(event);

    //     this->client->send(game_info);
    // }
}

void App::on_mouse_pressed(const sf::Event &event){
    this->screen_manager->get_active()->on_mouse_pressed(event);
}
void App::on_mouse_released(const sf::Event &event){
    this->screen_manager->get_active()->on_mouse_released(event);
    
}


int main() {
    App app = App();
    app.loop();
    return 0;
}