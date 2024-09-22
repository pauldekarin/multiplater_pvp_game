#ifndef TILEMAP_HPP
#define TILEMAP_HPP

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>
#include "nlohmann/json.hpp"
#include "globals.hpp"

class TileMap{
public:
    TileMap(const std::string path_to_source = "");
    void draw(sf::RenderWindow &window);
    bool collide(const sf::FloatRect &rect);
    const std::vector< sf::FloatRect > &getObstacles() const;
private:
    std::vector< std::vector <std::vector<sf::ScaledSprite> > > layers;
    std::vector< sf::FloatRect > obstacles;
    sf::Texture texture;
};

#endif