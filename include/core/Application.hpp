#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <SFML/Graphics.hpp>
#include <string>

namespace core {

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    sf::RenderWindow m_window;
    std::string m_title;
    sf::Vector2u m_defaultSize;
    
    void initWindow();
    void processEvents();
    void update(float deltaTime);
    void render();
};

} // namespace core

#endif // APPLICATION_HPP
