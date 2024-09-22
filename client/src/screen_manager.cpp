#include "../screen_manager.hpp"

ScreenManager::ScreenManager(){

}

ScreenManager::~ScreenManager(){

}

void ScreenManager::show(const std::string &name){

}

void ScreenManager::show(size_t index){
    if(index >= this->screens.size()) return;

    for(size_t index = 0; index < this->screens.size(); index++){
        this->screens.at(index)->hide();
    }

    this->screens.at(index)->show();
}

void ScreenManager::append(Screen &screen){
    this->append(screen.get_shared_ptr());
}

void ScreenManager::append(std::shared_ptr<Screen> screen){
    this->screens.push_back(std::move(screen));
    
    this->show(this->screens.size() - 1);
}

std::shared_ptr<Screen> ScreenManager::get_active(){
    for(auto &screen : this->screens){
        if(screen->get_state()){
            return screen;
        }
    }
}

void ScreenManager::setActive(size_t index){
    for(auto &screen : this->screens){
        screen->hide();
    }

    this->screens.at(index)->show();
}
void ScreenManager::setActive(const std::string &name_screen){
    for(size_t i = 0; i < this->screens.size(); i++){
        if(this->screens.at(i)->getName() == name_screen){
            this->setActive(i);
            break;
        }
    }
}