#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <SFML/Graphics.hpp>

#define TILE_SCALE 2
#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 600

bool operator==(const sf::Vector2f &lhs, const sf::Vector2f &rhs);

namespace sf{
    class ScaledSprite : public Sprite{
        public:
        ScaledSprite();
        ScaledSprite(const Texture &texture);
        ScaledSprite(const Texture &texture, const IntRect &rect);
    };
};

namespace utils {
    sf::Vector2f normalize(const sf::Vector2f& vector);
    float_t len(const sf::Vector2f &vector);
    sf::Vector2f interpolate(const sf::Vector2f& a, const sf::Vector2f& b, float t);
    sf::Vector2f clamp(const sf::Vector2f &a, const sf::Vector2f &lhs, const sf::Vector2f &rhs);
};

namespace Color
{
    const sf::Color Primary = sf::Color(168, 200, 162);
} // namespace Color

#endif