#include "../tilemap.hpp"

TileMap::TileMap(const std::string path_to_source){
    boost::filesystem::path path_to_folder(path_to_source);
    boost::filesystem::path js_filename("map.json");
    boost::filesystem::path tileset_filename("spritesheet.png");
    
    if(!boost::filesystem::exists(path_to_folder / js_filename)){
        throw std::runtime_error("Can`t find json file");
    }
    if(!boost::filesystem::exists(path_to_folder / tileset_filename)){
        throw std::runtime_error("Cant`t fine tileset file");
    }

    std::ifstream js_file( (path_to_folder / js_filename).c_str() );

    if(!js_file.is_open()){
        throw std::runtime_error("Can`t open json file");
    }

    nlohmann::json data = nlohmann::json::parse(js_file);

    if(!this->texture.loadFromFile((path_to_folder / tileset_filename).c_str())){
        throw std::runtime_error("Can`t load texture");
    }


    std::int32_t tile_size = data["tileSize"].get<std::int32_t>();

    std::int32_t tileset_width = 0;
    std::int32_t tileset_height = 0;

    try{
        boost::gil::rgba8_image_t tileset_image;
        boost::gil::read_image(
            (path_to_folder / tileset_filename).c_str(), tileset_image, boost::gil::png_tag()
        );
        tileset_width = tileset_image.width() / tile_size;
        tileset_height = tileset_image.height() / tile_size;

    }catch(std::exception e){
        throw e;
    }

    this->layers.reserve(data["layers"].size());

    for(auto it = data["layers"].rbegin(); it != data["layers"].rend(); it++){
        std::vector< std::vector<sf::ScaledSprite> > layer(
            data["mapHeight"].get<size_t>(), 
            std::vector<sf::ScaledSprite>(data["mapWidth"].get<size_t>())
        );

        for(auto jt = (*it)["tiles"].begin(); jt != (*it)["tiles"].end(); jt++){
            std::int32_t x = (*jt)["x"].get<std::int32_t>();
            std::int32_t y = (*jt)["y"].get<std::int32_t>();
            std::int32_t id = std::stoi((*jt)["id"].get<std::string>());

            sf::ScaledSprite &sprite = layer.at(y).at(x); 

            std::int32_t row = id / tileset_width;
            std::int32_t column = id % tileset_width;

            sprite.setTexture(texture);
            sprite.setTextureRect(
                sf::IntRect(column * tile_size,row * tile_size, tile_size, tile_size)
            );

            sprite.setPosition(sf::Vector2f(
                tile_size * x * TILE_SCALE,
                tile_size * y * TILE_SCALE
            ));

            if( (*it)["collider"] ){
                this->obstacles.push_back(
                    sf::FloatRect(
                        sf::Vector2f(sprite.getPosition().x, sprite.getPosition().y), 
                        sf::Vector2f(TILE_SCALE * tile_size, TILE_SCALE * tile_size))
                );
            }
        }

        this->layers.push_back(layer);
    }
}


void TileMap::draw(sf::RenderWindow &window){
    for(auto layer : this->layers){
        for(auto line : layer){
            for(auto sprite : line){
                window.draw(sprite);
            }
        }
    }
}

bool TileMap::collide(const sf::FloatRect &rect){
    for(auto obstacle : this->obstacles){
        if(obstacle.intersects(rect)) return true;
    }
    return false;
}

const std::vector<sf::FloatRect>& TileMap::getObstacles() const {
    return this->obstacles;
}
