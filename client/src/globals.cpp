#include "../globals.hpp"

bool operator==(const sf::Vector2f &lhs, const sf::Vector2f &rhs){
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

namespace sf{
    ScaledSprite::ScaledSprite() : Sprite(){
        this->setScale(Vector2f(TILE_SCALE, TILE_SCALE));
    }
    ScaledSprite::ScaledSprite(const Texture &texture) : Sprite(texture){
        this->setScale(Vector2f(TILE_SCALE, TILE_SCALE));
    }
    ScaledSprite::ScaledSprite(const Texture &texture, const IntRect &rect):Sprite(texture, rect){
        this->setScale(Vector2f(TILE_SCALE, TILE_SCALE));
    }
};


namespace utils{
    sf::Vector2f normalize(const sf::Vector2f& vector) {
        float length = std::sqrt(vector.x * vector.x + vector.y * vector.y); // Вычисляем длину вектора
        if (length != 0) {
            return vector / length; // Если длина не нулевая, делим вектор на его длину
        }
        return sf::Vector2f(0, 0); // Если длина нулевая, возвращаем нулевой вектор
    }

    float_t len(const sf::Vector2f &vector){
        return std::sqrt(std::pow(vector.x, 2) + std::pow(vector.y, 2));
    }

    sf::Vector2f interpolate(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
        // Убедитесь, что t находится в пределах [0, 1]
        t = std::clamp(t, 0.0f, 1.0f);
        
        return (1.0f - t) * a + t * b;
    }   

    sf::Vector2f clamp(const sf::Vector2f &a, const sf::Vector2f &lhs, const sf::Vector2f &rhs){
        sf::Vector2f r = a;

        r.x = std::min(rhs.x, r.x);
        r.x = std::max(r.x, lhs.x);
                
        r.y = std::min(rhs.y, r.y);
        r.y = std::max(r.y, lhs.y);

        return r;
    }

}