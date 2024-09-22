#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <memory>
#include <algorithm>
#include <map>
#include <unordered_set>
#include <cmath>
#include <set>

#include "screen.hpp"
#include "tilemap.hpp"
#include "external/json/include/nlohmann/json.hpp"

// Константы

#define ANIMATION_THRESHOLD .1f

class ThrowingWeapon;
class Weapon;
class Player;

class Object{
public:
    Object();

    std::int64_t getID();
    void setID(std::int64_t new_id);

    virtual void update(std::float_t delta) = 0;
    virtual void draw(sf::RenderWindow &window) = 0;
    virtual nlohmann::json getInfo() = 0;
    virtual std::string getClassName() = 0;
    virtual sf::FloatRect getGlobalBounds() const = 0;

    void markupForDelete();
    bool deletedInFuture();
    static std::int64_t getCounter();
    static bool exist(std::int64_t check_id);

protected:
    std::int64_t id;
    inline static std::int64_t counter = 0;
    inline static std::set<std::int64_t> existed_id;

    std::atomic<bool> hasMarkupForDelete = false;
};

sf::Vector2f operator*(const sf::Vector2f &vec, float_t i);

enum Facing {
    RIGHT,
    UP,
    DOWN,
    LEFT
};

typedef struct s_state {
    bool is_moving;
    Facing facing_direction;
} state;

typedef struct s_Frame {
    sf::Vector2u idle_left = sf::Vector2u(0, 0);
    sf::Vector2u idle_right = sf::Vector2u(0, 0);
    sf::Vector2u idle_up = sf::Vector2u(0, 0);
    sf::Vector2u idle_down = sf::Vector2u(0, 0);

    sf::Vector2u run_left = sf::Vector2u(0, 0);
    sf::Vector2u run_right = sf::Vector2u(0, 0);
    sf::Vector2u run_up = sf::Vector2u(0, 0);
    sf::Vector2u run_down = sf::Vector2u(0, 0);

    sf::IntRect tile_rect = sf::IntRect(0, 0, 0, 0);

    state current_state = {.is_moving = false, .facing_direction = Facing::RIGHT};
    sf::Uint64 current_frame = 0;

    sf::Uint64 operator++();
    sf::Uint64 operator++(int);
    void set_state(Facing facing_direction, bool is_moving = false);
    sf::IntRect operator()();
} Frame;

// Классы HealthBar, Weapon, ThrowingWeapon, Sprite, Player, World, GameScreen
class HealthBar {
public:
    HealthBar(float_t max_health, float_t width, float_t height = 10);
    void draw(sf::RenderWindow &window);
    void setHealth(std::int64_t health);
    std::int64_t getHealth();
    void setOffset(const sf::Vector2f offset);
    void setPosition(const sf::Vector2f position);
private:
    sf::RectangleShape health_bar_background;
    sf::RectangleShape health_bar;
    float_t max_health;
    float_t health;
};

class Sprite: public Object {
public:
    Sprite(const std::string &path_to_tilemap, const Frame &frame, const sf::Vector2f position = sf::Vector2f(0, 0));
    void update(float delta);
    bool takeDamage(const Weapon &weapon);
    void applyForce(const sf::Vector2f &force_direction, float_t force_strength);
    void draw(sf::RenderWindow &window);
    sf::FloatRect getGlobalBounds() const override;
    sf::Vector2f getPosition() const;
    sf::Vector2u getTileSize() const;
    void setPosition(const sf::Vector2f &position);
    std::int64_t getHealth();
    sf::Vector2f velocity;
    sf::Vector2f force;
    bool isMoving();
protected:
    sf::Sprite sprite;
    sf::Texture texture;
    Frame frame;
    HealthBar health_bar;
    
    float animation_threshold;
    float speed;
    float ellapsed_time;
    void update_frame();
    void update_moving();
};

class Backpack{
public:
    typedef struct s_BackpackSlot{
        std::shared_ptr<Weapon> ptr = nullptr;
        std::float_t ellapsed_time = 0;
        bool has_cooldown = false;
    } BackpackSlot;

    Backpack(std::size_t size = 2);

    void draw(sf::RenderWindow &window);
    void update(std::float_t delta);

    void append(std::shared_ptr<Weapon> weapon);
    
    bool canUseActive(); 
    void useActive(const std::shared_ptr<Player> player);

    std::shared_ptr<Weapon> getActive();
    void setActive(std::size_t index);
  
private:
    std::size_t size;
    std::size_t active_index;

    std::vector<BackpackSlot> slots; 

    std::size_t findFirstEmpty();
    bool isActiveSlotEmpty();
};

class Weapon: public Object, public std::enable_shared_from_this<Weapon> {
public:
    Weapon(const std::string &path_to_texture, const sf::Vector2f position = sf::Vector2f(0, 0), std::int64_t damage = 10);
    void draw(sf::RenderWindow &window);
    void update(std::float_t delta);

    bool checkCollision(const sf::FloatRect &bounding_rect);
    std::int64_t getDamage() const;
    sf::Vector2f getPosition() const;
    sf::Vector2f getTileSize() const;

    virtual std::float_t getCooldown() = 0;
    virtual sf::Vector2f getForce() const = 0;
    virtual void use(const sf::Vector2f &position, const sf::Vector2f &force) = 0;

    sf::FloatRect getGlobalBounds() const override;
protected:
    std::unique_ptr<sf::Sprite> sprite;
    std::unique_ptr<sf::Texture> texture;
    std::int64_t damage;
    float_t ellapsed_time;
    float_t animation_threshold;
    float_t cooldown;
};

class ThrowingWeapon : public Weapon {
public:
    ThrowingWeapon(const std::string &path_to_texture, const sf::Vector2f position = sf::Vector2f(0, 0), sf::Vector2f direction = sf::Vector2f(1, 0), std::int64_t damage = 20, float_t speed = 16, float_t rotation_angle = 45);
    void update(float delta) override;
    static std::shared_ptr<ThrowingWeapon> generate(const sf::Vector2f position = sf::Vector2f(0, 0), const sf::Vector2f direction = sf::Vector2f(1, 0));
    sf::Vector2f getForce() const override;
    std::float_t getCooldown() override;
    nlohmann::json getInfo() override;
    void use(const sf::Vector2f &position = sf::Vector2f(0,0), const sf::Vector2f &force = sf::Vector2f(0,0)) override;
    std::string getClassName() override;
private:
    std::float_t rotation_angle;
    sf::Vector2f velocity;
};



class Player : public Sprite {
public:
    Player(const std::string &path_to_tilemap, const Frame &frame, const sf::Vector2f position = sf::Vector2f(0, 0), Facing default_facing_direction = Facing::RIGHT);
    void update(float delta) override;
    void move(const Facing _direction);
    void release(const Facing _direction);
    void die();
    void setHealth(std::int32_t);
    static std::shared_ptr<Player> generate(const sf::Vector2f position = sf::Vector2f(0, 0), Facing default_facing_direction = Facing::RIGHT);
    void setDefaultFacingDirection(Facing facing);
    Facing getDefaultFacingDirection() const;

    bool is_alive() const;
    std::vector<Facing> getMoves() const;

    void setMoves(std::vector<Facing> &&moves);
    void attack();
    nlohmann::json getInfo() override;

    std::string getClassName() override;

    bool takeDamage(const Weapon &weapon);
private:
    std::vector<Facing> moves;
    Facing default_facing_direction;

    std::atomic<bool> is_dead = false;
    std::atomic<bool> attacked = false;
};

class World {
public:
    static World* Instance();
    void throwWeapon(const Player &player);
    void append(std::unique_ptr<Weapon>);

    void handle_keyboard_event(const sf::Event &event, bool main_loop = true);
    void check_collision(Sprite &sprite);
    void update(float delta);
    void update_all(float delta);
    void draw(sf::RenderWindow &window);

    nlohmann::json get_info();
    void apply_info(const nlohmann::json &data);
    void connect_room(const nlohmann::json &data);
    void start_game(bool on_left_side = true);
    void append(std::shared_ptr<Object> object);
    void destroy(std::shared_ptr<Object> object);
    std::vector<std::shared_ptr<Object>> getObjects() const;
protected:
    World();
private:
    std::shared_ptr<Player> player;
    std::shared_ptr<Player> enemy;
    std::shared_ptr<TileMap> map;
    std::unique_ptr<Backpack> backpack;

    std::vector<std::shared_ptr<Weapon>> weapons;
    std::vector<std::shared_ptr<Object>> objects;

    sf::Vector2f resolveCollision(Sprite &sprite, const sf::FloatRect &obstacle);

    float ellapsed_time = 0;

    void move(Player &character);
};

class GameScreen : public Screen {
public:
    GameScreen(const std::string &name = "game_screen");
    void loop();

    void update(std::float_t delta);
    void draw(sf::RenderWindow &window);

    void on_key_press_event(const sf::Event &event);
    void on_key_release_event(const sf::Event &event);
    void on_mouse_pressed(const sf::Event &event);
    void on_mouse_released(const sf::Event &event);
private:

};

#endif // GAME_H
