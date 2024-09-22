#ifndef MENU_HPP
#define MENU_HPP

#include "screen.hpp"
#include <SFML/Graphics.hpp>
#include "globals.hpp"

class Button{
public:
    Button(const std::string& text = "Button", const sf::Font &font = sf::Font());

    void draw(sf::RenderWindow &window);
    void update(std::float_t delta);
    void on_hover_event(sf::RenderWindow &window);
    void on_mouse_event(const sf::Event &event);
    void setPosition(const sf::Vector2f &pos);
    void setCallback(std::function<void()> &&func);

private:
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Text text;

    std::function<void()> callback = nullptr;
    std::float_t ellapsed_time = 0;
    bool is_hovered = false;
};

class MenuScreen: public Screen{
public:
    MenuScreen(const std::string &name_screen = "menu_screen", std::function<void()> start_game_callback = nullptr);

    void draw(sf::RenderWindow &window);
    void update(std::float_t delta);

    void on_key_press_event(const sf::Event &event);
    void on_key_release_event(const sf::Event &event);

    void on_mouse_pressed(const sf::Event &event);
    void on_mouse_released(const sf::Event &event);

private:
    std::function<void()> start_game_callback;
    
    sf::Font font;

    sf::Texture background_texture;
    sf::Sprite background_sprite;

    sf::Texture title_texture;
    sf::Sprite title_sprite;

    void init_background();
    void draw_background(sf::RenderWindow &window);

    void init_title();
    void draw_title(sf::RenderWindow &window);
    
    void init_menu();
    void draw_menu(sf::RenderWindow &window);


    std::vector<std::shared_ptr<Button>> layer;
};

#endif