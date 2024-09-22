#ifndef SCREEN_HPP
#define SCREEN_HPP
#include <string>
#include <atomic>
#include <vector>
#include <utility>
#include <SFML/Graphics.hpp>

class Screen: public std::enable_shared_from_this<Screen>{
public:
    Screen(const std::string &name = ""):name(name){
        static std::int32_t count_of_created_screens = 1;
        static std::vector<std::string> existing_names_of_screens;

        if(name.empty()){
            this->name = "screen_" + std::to_string(count_of_created_screens++); 
        }

        for(const std::string &existing_name : existing_names_of_screens){
            if(this->name.compare(existing_name) == 0){
                this->name.append("_copy");
            }
        }

        existing_names_of_screens.push_back(this->name);
    }

    std::string getName(){
        return this->name;
    }
    void show(){
        this->is_active.store(true, std::memory_order_relaxed);
    };

    void hide(){
        this->is_active.store(false, std::memory_order_relaxed);
    }

    bool get_state(){
        return this->is_active.load(std::memory_order_relaxed);
    }

    std::shared_ptr<Screen> get_shared_ptr(){
        return shared_from_this();
    }

    virtual void update(std::float_t delta) = 0;
    virtual void draw(sf::RenderWindow &window) = 0;

    virtual void on_key_press_event(const sf::Event &event) = 0;
    virtual void on_key_release_event(const sf::Event &event) = 0;
    virtual void on_mouse_pressed(const sf::Event &event) = 0;
    virtual void on_mouse_released(const sf::Event &event) = 0;

private:
    std::atomic<bool> is_active;
    std::string name;
};

#endif