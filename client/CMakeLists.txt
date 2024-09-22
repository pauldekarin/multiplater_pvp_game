cmake_minimum_required(VERSION 3.10)
project(MyWebSocketClient)

set(CMAKE_CXX_STANDARD 17)

# Include Libraties

##WebSocket++
include_directories(/opt/homebrew/Cellar/websocketpp/0.8.2/include/)
##Boost
include_directories(/opt/homebrew/Cellar/boost/1.86.0/include/)
##OpenSSL
include_directories(/opt/homebrew/Cellar/openssl@3/3.3.1/include/)
##SFML
include_directories(/opt/homebrew/Cellar/sfml/2.6.1/include)
include_directories(/Users/bimba/Desktop/multiplayer_sfml_heroku_server/client/external/json/include/)
include_directories(/opt/homebrew/Cellar/libpng/1.6.43/include)
# Link Libraries

link_directories(/opt/homebrew/Cellar/libpng/1.6.43/lib)
##Boost
link_directories(/opt/homebrew/Cellar/boost/1.86.0/lib/)
##OpenSSL
link_directories(/opt/homebrew/Cellar/openssl@3/3.3.1/lib/)
##WebSocket++
link_directories(/opt/homebrew/Cellar/websocketpp/lib)
##SFML
link_directories(/opt/homebrew/Cellar/sfml/2.6.1/lib)

file(GLOB SOURCES "./src/*.cpp")

add_subdirectory("./external/json")

add_executable(main ${SOURCES})

# Линковка библиотек
target_link_libraries(main
    boost_system
    boost_filesystem
    png
    ssl
    crypto
    pthread
    sfml-graphics
    sfml-system
    sfml-window
)