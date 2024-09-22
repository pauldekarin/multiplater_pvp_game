#ifndef SCREEN_MANAGER_HPP
#define SCREEN_MANAGER_HPP

#include <vector>
#include <iostream>
#include "screen.hpp"

class ScreenManager{
public:
    ScreenManager();
    ~ScreenManager();

    void show(const std::string &name);
    void show(size_t index);

    void append(Screen &screen);
    void append(std::shared_ptr<Screen> screen);

    std::shared_ptr<Screen> get_active();
    void setActive(size_t index);
    void setActive(const std::string &name_screen);
private:
    std::vector<std::shared_ptr<Screen>> screens;
};

#endif