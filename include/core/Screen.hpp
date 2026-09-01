#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace core {

class Screen {
public:
    virtual ~Screen() = default;

    virtual void handleEvent(sf::Event& event, sf::RenderWindow& window) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};

} // namespace core

#endif // SCREEN_HPP
