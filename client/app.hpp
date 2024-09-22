#ifndef APP_HPP
#define APP_HPP

#include "game.hpp"
#include "menu.hpp"
#include "client.hpp"
#include "screen_manager.hpp"
#include <functional>

class App{
public:
    App();
    ~App();

    void loop();
private:
    std::unique_ptr<sf::RenderWindow> window;
    std::unique_ptr<ScreenManager> screen_manager;
    std::unique_ptr<sf::Clock> clock;
    std::shared_ptr<Client> client;

    void on_key_press_event(const sf::Event &event);
    void on_key_release_event(const sf::Event &event);

    void on_mouse_pressed(const sf::Event &event);
    void on_mouse_released(const sf::Event &event);

    void update();
    void draw();

    void start_game();

    std::float_t ellapsed_time = 0;
};

#endif