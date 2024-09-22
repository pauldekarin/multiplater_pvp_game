#include "../menu.hpp"
#include <iostream>

Button::Button(const std::string &text, const sf::Font &font) {
    this->texture = sf::Texture();
    this->sprite = sf::Sprite();
    this->text = sf::Text();


    if (!this->texture.loadFromFile("../assets/button.png")) {
        // Обработка ошибки
    }
    


    this->text.setFont(font);
    this->text.setStyle(sf::Text::Bold);
    this->text.setLetterSpacing(0);
    this->text.setFillColor(sf::Color::White);  // Установи желаемый цвет текста
    this->text.setString(text);
    this->text.setCharacterSize(24);
    this->text.setFillColor(Color::Primary);
    this->text.setOrigin(this->text.getGlobalBounds().width / 2, this->text.getGlobalBounds().height / 2 + this->text.getCharacterSize() / 2 - 4.f);


    int padding_x = 20;
    int padding_y = 10;

    this->sprite.setTexture(this->texture);
    this->sprite.setScale(sf::Vector2f(
        (this->text.getLocalBounds().getSize().x + 2 * padding_x) / this->texture.getSize().x,
        (this->text.getLocalBounds().getSize().y + 2 * padding_y) / this->texture.getSize().y
    ));

    this->sprite.setOrigin(
        sf::Vector2f(this->sprite.getLocalBounds().getSize().x / 2, this->sprite.getLocalBounds().getSize().y / 2)
    );

}

void Button::setCallback(std::function<void()> &&func){
    this->callback = std::forward<std::function<void()> >(func);
}
void Button::on_hover_event(sf::RenderWindow &window){
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
    if(this->sprite.getGlobalBounds().contains(sf::Vector2f(mouse_pos))){
        this->is_hovered = true;
    }else{
        this->is_hovered = false;
    }
}
void Button::on_mouse_event(const sf::Event &event){
    if(event.type == sf::Event::MouseButtonReleased){
        if(this->sprite.getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))){
            if(this->callback != nullptr){
                this->callback();
            }
        }
    }
}

void Button::setPosition(const sf::Vector2f &pos){
    this->sprite.setPosition(pos);
    this->text.setPosition(pos);
}
void Button::draw(sf::RenderWindow &window){
    window.draw(this->sprite);
    window.draw(this->text);
}

void Button::update(std::float_t delta){
    this->ellapsed_time += delta;

    if(this->ellapsed_time > 0.05f){
        this->ellapsed_time = 0;

        static sf::Vector2f initial_sprite_scale = this->sprite.getScale();
        static sf::Vector2f initia_text_scale = this->text.getScale();

        std::float_t scale_difference = 0.2f;
        std::float_t scale_factor = 0.1f;
        
        sf::Vector2f sprite_scale = this->sprite.getScale();
        sf::Vector2f text_scale = this->text.getScale();
        
        sf::Vector2f scale_range = sf::Vector2f(scale_factor, scale_factor);

        if(this->is_hovered){
            sprite_scale += scale_range;
            text_scale += scale_range;
        }else{
            sprite_scale -= scale_range;
            text_scale -= scale_range;
        }

        this->text.setScale(utils::clamp(text_scale, initia_text_scale, initia_text_scale + sf::Vector2f(scale_difference,scale_difference)));
        this->sprite.setScale(utils::clamp(sprite_scale, initial_sprite_scale, initial_sprite_scale + sf::Vector2f(scale_difference, scale_difference)));
    
    }
}


MenuScreen::MenuScreen(const std::string &name_screen, std::function<void()> start_game_callback): Screen(name_screen), start_game_callback(start_game_callback){
    this->font = sf::Font();
    this->font.loadFromFile("../fonts/primary.ttf");
    this->font.setSmooth(true);
    
    this->init_menu();
    this->init_background();
    this->init_title();
}

void MenuScreen::init_title(){
    this->title_texture = sf::Texture();
    if(!this->title_texture.loadFromFile("../assets/title.png")){

    }

    this->title_sprite.setTexture(this->title_texture);
    this->title_sprite.setOrigin(this->title_texture.getSize().x / 2, this->title_texture.getSize().y / 2);
    this->title_sprite.setPosition(sf::Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4));
}

void MenuScreen::draw_title(sf::RenderWindow &window){
    window.draw(this->title_sprite);
}

void MenuScreen::init_menu(){
    std::shared_ptr<Button> start_button = std::make_shared<Button>("Start Game", this->font);
    start_button->setPosition(sf::Vector2f(
        WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2
    ));
    start_button->setCallback(std::move(this->start_game_callback));
    this->layer.push_back(start_button);
}
void MenuScreen::init_background(){
    this->background_texture = sf::Texture();
    if(!this->background_texture.loadFromFile("../assets/menu_background.png")){

    }
    this->background_sprite.setScale(sf::Vector2f(TILE_SCALE, TILE_SCALE));
    this->background_sprite.setTexture(this->background_texture);
    this->background_sprite.setTextureRect(sf::IntRect(
        0,0, WINDOW_WIDTH, WINDOW_HEIGHT
    ));
}

void MenuScreen::draw_background(sf::RenderWindow &window){
    window.draw(this->background_sprite);
}

void MenuScreen::draw_menu(sf::RenderWindow &window){
    for(auto &el : this->layer){
        el->on_hover_event(window);
        el->draw(window);
    }
}
void MenuScreen::update(std::float_t delta){
    for(auto &el : this->layer){
        el->update(delta);
    }
}

void MenuScreen::draw(sf::RenderWindow &window){
    this->draw_background(window);
    this->draw_title(window);
    this->draw_menu(window);
}

void MenuScreen::on_key_press_event(const sf::Event &event){

}

void MenuScreen::on_key_release_event(const sf::Event &event){

}

void MenuScreen::on_mouse_pressed(const sf::Event &event){
    for(auto &el : this->layer){
        el->on_mouse_event(event);
    }
}
void MenuScreen::on_mouse_released(const sf::Event &event){
    for(auto &el : this->layer){
        el->on_mouse_event(event);
    }
}

