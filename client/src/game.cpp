#include "../game.hpp"

sf::Vector2f operator*(const sf::Vector2f &vec, float_t i){
    return sf::Vector2f(vec.x * i, vec.y * i);
}



sf::Uint64 Frame::operator++(){
    return ++(this->current_frame);
}

sf::Uint64 Frame::operator++(int){
    return (this->current_frame)++;
}

void Frame::set_state(Facing facing_direction, bool is_moving){
    this->current_state.facing_direction = facing_direction;
    this->current_state.is_moving = is_moving;
}

Object::Object(){
    this->id = this->counter++;
    Object::existed_id.insert(this->id);
}
bool Object::exist(std::int64_t check_id){
    return Object::existed_id.count(check_id) != 0;
}
std::int64_t Object::getCounter(){
    return Object::counter;
}
void Object::markupForDelete(){
    this->hasMarkupForDelete.store(true, std::memory_order_relaxed);
}

bool Object::deletedInFuture(){
    return this->hasMarkupForDelete.load(std::memory_order_relaxed);
}

std::int64_t Object::getID(){
    return this->id;
}
void Object::setID(std::int64_t new_id){
    auto it = Object::existed_id.find(new_id);
    if(it != Object::existed_id.end()){
        Object::existed_id.erase(it);
    }
    this->id = new_id;

    Object::existed_id.insert(new_id);
}
sf::IntRect Frame::operator()(){
    sf::IntRect frame_rect;
    frame_rect.width = this->tile_rect.width;
    frame_rect.height = this->tile_rect.height;

    int tile_height = this->tile_rect.top * 2 + frame_rect.height;
    int tile_width = this->tile_rect.left * 2 + frame_rect.width;

    sf::Vector2u frame_pos(0,0);

    if(this->current_state.facing_direction == Facing::LEFT){
        if(this->current_state.is_moving){
            frame_pos = this->run_left;
        }else{
            frame_pos = this->idle_left;
        }

    }else if(this->current_state.facing_direction == Facing::RIGHT){
        if(this->current_state.is_moving){
            frame_pos = this->run_right;
        }else{
            frame_pos = this->idle_right;
        }
    }else if(this->current_state.facing_direction == Facing::UP){
        if(this->current_state.is_moving){
            frame_pos = this->run_up;
        }else{
            frame_pos = this->idle_up;
        }

    }else if(this->current_state.facing_direction == Facing::DOWN){
        if(this->current_state.is_moving){
            frame_pos = this->run_down;
        }else{
            frame_pos = this->idle_down;
        }

    }
    this->current_frame %= frame_pos.y;
    frame_rect.top = this->tile_rect.top + tile_height * frame_pos.x;
    frame_rect.left = this->tile_rect.left + tile_width * this->current_frame;
    return frame_rect;
}


HealthBar::HealthBar
(
    float_t max_health, 
    float_t width, 
    float_t height
):max_health(max_health), health(max_health){
    this->health_bar.setSize(sf::Vector2f(width, height));
    this->health_bar.setFillColor(sf::Color::Green);
    this->health_bar.setOrigin(0, height * 2);

    this->health_bar_background.setSize(sf::Vector2f(width,height));
    this->health_bar_background.setOutlineColor(sf::Color::Yellow);
    this->health_bar_background.setFillColor(sf::Color::Transparent);
    this->health_bar_background.setOrigin(0, height * 2);
}

void HealthBar::draw(sf::RenderWindow &window){
    window.draw(this->health_bar);
    window.draw(this->health_bar_background);
}

void HealthBar::setHealth(std::int64_t health){
    this->health = health <= 0 ? 0 : health;

    if(this->health >= 75){
        this->health_bar.setFillColor(sf::Color::Green);
    }else if(this->health >= 25){
        this->health_bar.setFillColor(sf::Color::Yellow);
    }else{
        this->health_bar.setFillColor(sf::Color::Red);
    }

    
    this->health_bar.setSize(sf::Vector2f(this->health / this->max_health * this->health_bar_background.getSize().x ,this->health_bar.getSize().y));

}

std::int64_t HealthBar::getHealth(){
    return this->health;
}

void HealthBar::setOffset(const sf::Vector2f offset){
    this->health_bar.setOrigin(offset);
    this->health_bar_background.setOrigin(offset);
}
void HealthBar::setPosition(const sf::Vector2f position){
    this->health_bar.setPosition(position);
    this->health_bar_background.setPosition(position);
}

Backpack::Backpack(std::size_t size):size(size){
    this->slots.resize(size);
}

std::size_t Backpack::findFirstEmpty(){
    for(std::size_t i = 0; i < this->slots.size(); i++){
        if(this->slots.at(i).ptr == nullptr){
            return i;
        }
    }
    return -1;
}


void Backpack::append(std::shared_ptr<Weapon> weapon){
    std::size_t idx;
    if((idx = this->findFirstEmpty()) != -1){
        this->slots.at(idx).ptr = weapon;
        this->slots.at(idx).ellapsed_time = 0;
        this->slots.at(idx).has_cooldown = true;
        this->active_index = idx;
    }
}

std::shared_ptr<Weapon> Backpack::getActive(){
    return this->slots.at(this->active_index).ptr;
}

bool Backpack::isActiveSlotEmpty(){
    return this->slots.at(this->active_index).ptr == nullptr;
}
bool Backpack::canUseActive(){
    if(this->isActiveSlotEmpty()) return false;

    BackpackSlot &active_slot = this->slots.at(this->active_index);
    if(!active_slot.has_cooldown || active_slot.ellapsed_time >= active_slot.ptr->getCooldown()){
        return true;
    }
    return false;
}
void Backpack::useActive(const std::shared_ptr<Player> player){
    sf::Vector2f player_position = player->getPosition();
    sf::FloatRect player_rect = player->getGlobalBounds();
    Facing player_direction = player->getDefaultFacingDirection();

    this->slots.at(this->active_index).ellapsed_time = 0;
    this->slots.at(this->active_index).ptr->use(
        sf::Vector2f( player_direction == Facing::RIGHT ? player_rect.left + player_rect.width * 2 : player_rect.left - player_rect.width, player_position.y ),
        sf::Vector2f( player_direction == Facing::RIGHT ? 1 : -1, 0 )
    );
}
void Backpack::update(std::float_t delta){
    for(auto it = this->slots.begin(); it != this->slots.end(); it++){
        if(it->has_cooldown){ it->ellapsed_time += delta; }
    }
}

Weapon::Weapon(
    const std::string &path_to_texture, 
    const sf::Vector2f position,
    std::int64_t damage
):Object(),damage(damage), animation_threshold(ANIMATION_THRESHOLD){
    this->texture = std::make_unique<sf::Texture>();
    this->texture->loadFromFile(path_to_texture);

    this->sprite = std::make_unique<sf::Sprite>();
    this->sprite->setTexture(*this->texture);
}


void Weapon::update(std::float_t delta){
    for(const auto &obj : World::Instance()->getObjects()){
        if(obj->getID() == this->getID()) continue;

        if(obj->getClassName().compare("Player") == 0){
            std::shared_ptr<Player> player = std::static_pointer_cast<Player>(obj);
            
            if(player->getGlobalBounds().intersects(this->getGlobalBounds())){
                player->takeDamage(*this);
                this->markupForDelete();
                return;
            }
        }
    }
}
sf::FloatRect Weapon::getGlobalBounds() const{
    return this->sprite->getGlobalBounds();
}
void Weapon::draw(sf::RenderWindow &window){
    window.draw(*this->sprite);
}


bool Weapon::checkCollision(const sf::FloatRect &bounding_rect){
    return this->sprite->getGlobalBounds().intersects(bounding_rect);
}


std::int64_t Weapon::getDamage() const{
    return this->damage;
}

sf::Vector2f Weapon::getPosition() const{
    return this->sprite->getPosition();
}

sf::Vector2f Weapon::getTileSize() const{
    return sf::Vector2f(this->sprite->getLocalBounds().width, this->sprite->getLocalBounds().height);
}


ThrowingWeapon::ThrowingWeapon
(
    const std::string &path_to_texture, 
    const sf::Vector2f position,
    sf::Vector2f direction,
    std::int64_t damage,
    float_t speed,
    float_t rotation_angle
):
Weapon(path_to_texture, position, damage), 
rotation_angle(rotation_angle)
{
    sf::FloatRect bounding_rect = this->sprite->getLocalBounds();
    sf::Vector2f origin_vector = sf::Vector2f(bounding_rect.width / 2, bounding_rect.height / 2);

    this->sprite->setOrigin(origin_vector);
    this->sprite->setPosition(position + origin_vector);
    this->sprite->setScale(sf::Vector2f(TILE_SCALE, TILE_SCALE));

    this->velocity = utils::normalize(direction) * speed;

    this->cooldown = 0.3f;
}

std::string ThrowingWeapon::getClassName(){
    return "ThrowingWeapon";
}
void ThrowingWeapon::use(const sf::Vector2f &position, const sf::Vector2f &force){
    World::Instance()->append(ThrowingWeapon::generate(position, force));
}
void ThrowingWeapon::update(float delta){
    Weapon::update(delta);

    this->ellapsed_time += delta;

    if(this->ellapsed_time >= this->animation_threshold){
        this->ellapsed_time = 0;

        this->sprite->move(this->velocity);
        this->sprite->rotate(this->rotation_angle);
    }        
}

nlohmann::json ThrowingWeapon::getInfo(){
    nlohmann::json info = {
        {"class_name" , this->getClassName()},
        {"position", nlohmann::json::array({
            getPosition().x, getPosition().y,
        })},
        {"force" , nlohmann::json::array({
            getForce().x, getForce().y
        })},
        {"id" , getID()},
        {"deleted" , this->deletedInFuture()},
        
    };
    return info;
}
std::float_t ThrowingWeapon::getCooldown(){
    return this->cooldown;
}

std::shared_ptr<ThrowingWeapon> 
ThrowingWeapon::generate(
    const sf::Vector2f position,
    const sf::Vector2f direction
)
{
    return std::make_shared<ThrowingWeapon>(
        "../assets/axe.png",
        position,
        direction
    );
}

sf::Vector2f ThrowingWeapon::getForce() const{
    return this->velocity;
}


Sprite::Sprite
(
    const std::string &path_to_tilemap, 
    const Frame &frame, 
    const sf::Vector2f position
):
    Object(),
    speed(16),
    ellapsed_time(0),
    animation_threshold(ANIMATION_THRESHOLD),
    frame(frame),
    health_bar(HealthBar(0,0,0)),
    force(sf::Vector2f(0,0)),
    velocity(sf::Vector2f(0,0))
    {
        this->texture = sf::Texture();
        this->texture.loadFromFile(path_to_tilemap);

        this->sprite.setTexture(this->texture);
        this->sprite.setScale(sf::Vector2f(TILE_SCALE, TILE_SCALE));
        this->sprite.setPosition(position);

        this->health_bar = HealthBar(100, this->getTileSize().x ,10);
}

bool Sprite::isMoving(){
    return (this->velocity + this->force == sf::Vector2f(0,0));
}

void Sprite::update(float delta){
    this->ellapsed_time += delta;
    this->health_bar.setPosition(this->getPosition());

    if(this->ellapsed_time >= this->animation_threshold){
        this->update_frame();
        this->force = utils::interpolate(this->force, sf::Vector2f(0,0), 0.4f);
        
        this->ellapsed_time = 0;
    }
}

bool Player::takeDamage(const Weapon &weapon){
    this->health_bar.setHealth(this->health_bar.getHealth() - weapon.getDamage());
    this->applyForce(weapon.getForce(), utils::len(weapon.getForce()));

    if(this->health_bar.getHealth() <= 0) this->die();

    return this->health_bar.getHealth() <= 0;
}

void Sprite::applyForce(const sf::Vector2f &force_direction, float_t force_strength){
    this->force = utils::normalize(force_direction) * force_strength;  
}

void Sprite::draw(sf::RenderWindow &window){
    window.draw(this->sprite);
    this->health_bar.draw(window);
}

sf::FloatRect Sprite::getGlobalBounds() const{
    return this->sprite.getGlobalBounds();
}

sf::Vector2f Sprite::getPosition() const{
    return this->sprite.getPosition();
}
sf::Vector2u Sprite::getTileSize() const{
    return sf::Vector2u(this->frame.tile_rect.width * TILE_SCALE, this->frame.tile_rect.height * TILE_SCALE);
}

void Sprite::setPosition(const sf::Vector2f &position){
    this->sprite.setPosition(position);
}

std::int64_t Sprite::getHealth(){
    return this->health_bar.getHealth();
}

void Sprite::update_frame(){
    this->sprite.setTextureRect(this->frame());
    this->frame++;
}

void Sprite::update_moving(){
    this->sprite.move(this->force + this->velocity);
    this->force = utils::interpolate(this->force, sf::Vector2f(0,0), 0.4f);
}



Player::Player(
    const std::string &path_to_tilemap, 
    const Frame &frame, 
    const sf::Vector2f position,
    Facing default_facing_direction):
        Sprite(path_to_tilemap, frame, position),
        default_facing_direction(default_facing_direction)
{
    this->frame.set_state(this->default_facing_direction);
}
void Player::setHealth(std::int32_t health){
    this->health_bar.setHealth(health);
}
std::vector<Facing> Player::getMoves() const{
    return this->moves;
}

std::string Player::getClassName(){
    return "Player";
}

nlohmann::json Player::getInfo(){
    nlohmann::json moves = nlohmann::json::array();
    for(const Facing move : getMoves()){
        moves.push_back(move);
    }

    nlohmann::json pos = nlohmann::json::array({
        getPosition().x, getPosition().y
    });

    nlohmann::json info = {
        {"position", pos},
        {"moves", moves},
        {"attacked", false},
        {"health" , this->health_bar.getHealth()}
    };

    return info;
}
void Player::update(float delta){
    Sprite::update(delta);

    this->velocity = sf::Vector2f(0,0);
    this->frame.set_state(this->default_facing_direction);

    if(!this->moves.empty()){
        auto iterator_vertical_move = std::find_if(this->moves.rbegin(), this->moves.rend(), [](const Facing &move){
            return move == Facing::UP || move == Facing::DOWN;
        });
        auto iterator_horizontal_move = std::find_if(this->moves.rbegin(), this->moves.rend(), [](const Facing &move){
            return move == Facing::LEFT || move == Facing::RIGHT;
        });


        if(iterator_horizontal_move != this->moves.rend()){
            if(*iterator_horizontal_move == Facing::RIGHT){
                this->velocity.x = this->speed;
                this->frame.set_state(Facing::RIGHT, true);
            }else{
                this->velocity.x = -this->speed;
                this->frame.set_state(Facing::LEFT, true);
            }
        }
        if(iterator_vertical_move != this->moves.rend()){
            if(*iterator_vertical_move == Facing::UP){
                this->velocity.y = -this->speed;
                this->frame.set_state(Facing::UP, true);
            }else{
                this->velocity.y = this->speed;
                this->frame.set_state(Facing::DOWN, true);
            }
        }
        
    }
}

void Player::move(const Facing _direction){
    if(this->is_dead.load(std::memory_order_relaxed)) return;

    if(std::find(this->moves.begin(), this->moves.end(), _direction) != this->moves.end()) return;
    this->moves.push_back(_direction);
}

void Player::setMoves(std::vector<Facing> &&moves){
    this->moves = std::move(moves);
}

void Player::release(const Facing _direction){
    auto it = std::find(this->moves.begin(), this->moves.end(), _direction);
    if(it != this->moves.end()){
        this->moves.erase(it);
    }
}

void Player::die(){
    if(this->is_dead.load(std::memory_order_relaxed)) return;
    this->moves.clear();

    this->force = sf::Vector2f(0,0);
    this->velocity = sf::Vector2f(0,0);

    this->health_bar.setHealth(100);
    this->setPosition(sf::Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2));

}
std::shared_ptr<Player> 
Player::generate(const sf::Vector2f position, Facing default_facing_direction){
    return std::make_shared<Player>(
        "../assets/player.png",
        Frame{
            .idle_left = sf::Vector2u(2,2),
            .run_left = sf::Vector2u(2,4),

            .idle_right = sf::Vector2u(3,2),
            .run_right = sf::Vector2u(3,4),

            .idle_down = sf::Vector2u(0,2),
            .run_down = sf::Vector2u(0,4),

            .idle_up = sf::Vector2u(1,2),
            .run_up = sf::Vector2u(1,4),
            .tile_rect = sf::IntRect(17,16,14,16)
        },
        position,
        default_facing_direction
    );
}

Facing Player::getDefaultFacingDirection() const{
    return this->default_facing_direction;
}

bool Player::is_alive() const{
    return !this->is_dead.load(std::memory_order_relaxed);
}

void Player::setDefaultFacingDirection(Facing facing){
    this->default_facing_direction = facing;
}

World* World::Instance(){
    static World* instance = nullptr;

    if(instance == nullptr){
        instance = new World();
    }
    return instance;
}

void World::destroy(std::shared_ptr<Object> object){
    auto it = std::find_if(this->objects.begin(), this->objects.end(), [&](std::shared_ptr<Object> const &obj){
        return obj->getID() == object->getID();
    });

    if(it != this->objects.end()){
        this->objects.erase(it);
    }
}

std::vector<std::shared_ptr<Object>> World::getObjects() const{
    return this->objects;
}

void World::throwWeapon(const Player &player){
    if(!player.is_alive()) return;

    Facing default_facing_direction = player.getDefaultFacingDirection();
    sf::Vector2f player_position = player.getPosition();
    sf::FloatRect player_rect = player.getGlobalBounds();

    std::shared_ptr<ThrowingWeapon> throwing_weapon = ThrowingWeapon::generate(
        sf::Vector2f( default_facing_direction == Facing::RIGHT ? player_rect.left + player_rect.width * 2 : player_rect.left - player_rect.width, player_position.y ),
        sf::Vector2f( default_facing_direction == Facing::RIGHT ? 1 : -1, 0 )
    );

    this->objects.push_back(throwing_weapon);
}   

void World::handle_keyboard_event(const sf::Event &event, bool main_loop){
    std::shared_ptr<Player> character((main_loop ? this->player : this->enemy));

    if(event.type == sf::Event::KeyPressed){
        if(event.key.code == sf::Keyboard::W){
            character->move(Facing::UP);
        }else if(event.key.code == sf::Keyboard::S){
            character->move(Facing::DOWN);
        }else if(event.key.code == sf::Keyboard::A){
            character->move(Facing::LEFT);
        }else if(event.key.code == sf::Keyboard::D){
            character->move(Facing::RIGHT);
        }else if(event.key.code == sf::Keyboard::Space){
            if(this->backpack->canUseActive()){
                this->backpack->useActive(this->player);
            }
        }else if(event.key.code == sf::Keyboard::P){
        }
    }

    if(event.type == sf::Event::KeyReleased){
        if(event.key.code == sf::Keyboard::W){
            character->release(Facing::UP);
        }else if(event.key.code == sf::Keyboard::S){
            character->release(Facing::DOWN);
        }else if(event.key.code == sf::Keyboard::A){
            character->release(Facing::LEFT);
        }else if(event.key.code == sf::Keyboard::D){
            character->release(Facing::RIGHT);
        }
    }
}

void World::append(std::shared_ptr<Object> object){
    this->objects.push_back(object);
}

void World::check_collision(Sprite &sprite){
    
    sf::Vector2f sprite_position = sprite.getPosition();
    sf::Vector2u sprite_tile_size = sprite.getTileSize();

    sf::Vector2f boundaries = sf::Vector2f(0, 800 - sprite_tile_size.y * TILE_SCALE);
    sprite_position.y = std::clamp(sprite_position.y, boundaries.x, boundaries.y);
    sprite.setPosition(sprite_position);

    const auto &obstacles = this->map->getObstacles();

    sf::FloatRect sprite_bounds = sprite.getGlobalBounds();

    for(const auto &obstacle : obstacles){
        if(sprite_bounds.intersects(obstacle)){
            sprite.setPosition(this->resolveCollision(sprite, obstacle));
        }
    }

}

sf::Vector2f World::resolveCollision(Sprite &sprite, const sf::FloatRect &obstacle) {
    sf::FloatRect playerBounds = sprite.getGlobalBounds();
    sf::Vector2f resolution = sprite.getPosition();

    if(sprite.velocity.y != 0){

    }
    // Вертикальная коллизия
    if (playerBounds.top < obstacle.top && playerBounds.top + playerBounds.height > obstacle.top) {
        // Игрок движется вниз и сталкивается с верхней частью препятствия
        resolution.y = obstacle.top - playerBounds.height;  // Поднимаем игрока над препятствием
        sprite.velocity.y = 0;  // Останавливаем движение по вертикали
    } else if (playerBounds.top > obstacle.top) {
        // Игрок движется вверх и сталкивается с нижней частью препятствия
        resolution.y = obstacle.top + obstacle.height;  // Опускаем игрока под препятствие
        sprite.velocity.y = 0;  // Останавливаем движение по вертикали
    }

    // Горизонтальная коллизия
    if (playerBounds.left < obstacle.left && playerBounds.left + playerBounds.width > obstacle.left) {
        // Игрок движется вправо и сталкивается с левой стороной препятствия
        resolution.x = obstacle.left - playerBounds.width;  // Останавливаем его слева от препятствия
        sprite.velocity.x = 0;  // Останавливаем движение по горизонтали
    } else if (playerBounds.left > obstacle.left + obstacle.width) {
        // Игрок движется влево и сталкивается с правой стороной препятствия
        resolution.x = obstacle.left + obstacle.width;  // Останавливаем его справа от препятствия
        sprite.velocity.x = 0;  // Останавливаем движение по горизонтали
    }

    return resolution;
}


void World::move(Player &character){
    sf::Vector2f velocity = character.velocity + character.force;

    if(velocity == sf::Vector2f(0,0)){
        return;
    }
    const auto &obstacles = this->map->getObstacles();

    sf::FloatRect move_y = character.getGlobalBounds();
    sf::FloatRect move_x = character.getGlobalBounds();

    move_y.top += velocity.y;
    move_x.left += velocity.x;
    
    std::int8_t checked = velocity.x != 0 && velocity.y != 0 ? 0 : 1;

    for(const auto &obstacle : obstacles){
        if(velocity.y != 0 && obstacle.intersects(move_y)){
            move_y.top = character.getGlobalBounds().top;
            checked++;
        }
        if(velocity.x != 0 && obstacle.intersects(move_x)){
            move_x.left = character.getGlobalBounds().left;
            checked++;
        }

        if(checked >= 2){
            break;
        }
    }

    character.setPosition(sf::Vector2f(move_x.left, move_y.top));
}
void World::update(float delta){
    this->update_all(delta);
    
    this->ellapsed_time += delta;

    if(this->ellapsed_time >= 0.1f){
        this->move(*this->player);
        this->move(*this->enemy);

        this->ellapsed_time = 0;
    }
    
    this->objects.erase(
        std::remove_if(this->objects.begin(), this->objects.end(),[](const std::shared_ptr<Object> &obj){
            return obj->deletedInFuture();
        }),
        this->objects.end()
    );
}

void World::update_all(float delta){
    this->backpack->update(delta);

    for(const auto &obj : this->objects){
        if(!obj->deletedInFuture()) { obj->update(delta); }
    }
    
}
void World::draw(sf::RenderWindow &window){
    this->map->draw(window);

    for(const auto &obj : this->objects){
        if(!obj->deletedInFuture()){ obj->draw(window);}
    }
}

World::World(){
    this->player = Player::generate(sf::Vector2f(200,180), Facing::RIGHT);
    this->enemy = Player::generate(sf::Vector2f(200,180), Facing::LEFT);
    this->map = std::make_shared<TileMap>("../maps/default");

    this->backpack = std::make_unique<Backpack>();
    this->backpack->append(ThrowingWeapon::generate());

    this->objects.push_back(this->player);
    this->objects.push_back(this->enemy);
}
nlohmann::json World::get_info(){
    nlohmann::json objects_data = nlohmann::json::array();
    for(const auto & object : this->objects){
        if(object->getClassName().compare("Player") != 0){
            objects_data.push_back(object->getInfo());
        }
    };
    nlohmann::json info = {
        {
            "player", this->player->getInfo()
        },
        {
            "objects", objects_data
        }
    };
    return info;
};
void World::apply_info(const nlohmann::json &data){
    const std::function<void(std::vector<std::float_t>&&)> apply_pos = [this](std::vector<std::float_t> &&pos){
        this->enemy->setPosition(sf::Vector2f(
            pos.front(), pos.back()
        ));
    };

    const std::function<void(std::vector<std::int32_t>&&)> apply_moves = [this](std::vector<std::int32_t> &&moves){
        if(!moves.empty()){
            
        }
        std::vector<Facing> m(moves.size());
        std::transform(moves.begin(), moves.end(), m.begin(),[](std::int32_t n){return static_cast<Facing>(n);});
        this->enemy->setMoves(std::move(m));
    };
    const std::function<void(std::int32_t)> apply_health = [this](std::int32_t health){
        this->enemy->setHealth(health);
    };
    try{
        apply_pos(data["player"]["position"].get<std::vector<std::float_t>>());
        apply_moves(data["player"]["moves"].get<std::vector<std::int32_t>>());
        apply_health(data["player"]["health"].get<std::int32_t>());

        for(const auto &weapon_data : data["objects"]){
            if(weapon_data["deleted"].get<bool>() || Object::exist(weapon_data["id"])) continue;

            auto it = std::find_if(this->objects.begin(), this->objects.end(), [&](const std::shared_ptr<Object> el){
                return weapon_data["id"] == el->getID();
            });

            if(it == this->objects.end()){
                if(weapon_data["class_name"].get<std::string>().compare("ThrowingWeapon") == 0){
                    std::vector<std::float_t> weapon_pos = weapon_data["position"].get<std::vector<std::float_t> >();
                    std::vector<std::float_t> weapon_force = weapon_data["force"].get<std::vector<std::float_t>>();

                    std::shared_ptr<ThrowingWeapon> weapon_ptr = ThrowingWeapon::generate(
                        sf::Vector2f(weapon_pos.front(), weapon_pos.back()),
                        sf::Vector2f(weapon_force.front(), weapon_force.back())
                    );
                    weapon_ptr->setID(weapon_data["id"]);
                    this->append(weapon_ptr);
                }
            }
        }
        
    }catch(std::exception e){

    }
}

void World::connect_room(const nlohmann::json &data){
    std::int64_t l_idx = data["init"]["left"];
    std::int64_t r_idx = data["init"]["right"];

    std::int64_t idx = data["client_id"];

    this->player->setDefaultFacingDirection(l_idx == idx ? RIGHT : LEFT);
    this->enemy->setDefaultFacingDirection(l_idx != idx ? RIGHT : LEFT);
}

void World::start_game(bool on_left_side){
    const sf::Vector2f left_character_pos = sf::Vector2f(200,180);
    const sf::Vector2f right_character_pos = sf::Vector2f(350,550);
    
    if(on_left_side){
        this->player->setPosition(left_character_pos);

        this->enemy->setPosition(right_character_pos);
    }else{
        this->player->setPosition(right_character_pos);

        this->enemy->setPosition(left_character_pos);
    }
}

GameScreen::GameScreen(const std::string &name):Screen(name){

}

void GameScreen::on_mouse_pressed(const sf::Event &event){

}
void GameScreen::on_mouse_released(const sf::Event &event){
    
}


void GameScreen::on_key_press_event(const sf::Event &event){
    World::Instance()->handle_keyboard_event(event);
}
void GameScreen::on_key_release_event(const sf::Event &event){
    World::Instance()->handle_keyboard_event(event);
}
void GameScreen::update(float delta){
    World::Instance()->update(delta);
}
void GameScreen::draw(sf::RenderWindow &window){
    World::Instance()->draw(window);
}


