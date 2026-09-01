#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "ui/RoundedRectangleShape.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

namespace ui {

class Button : public sf::Drawable {
public:
    Button(const std::string& label, const sf::Vector2f& size = sf::Vector2f(100.f, 40.f), unsigned int fontSize = 16);

    void setPosition(const sf::Vector2f& position);
    sf::Vector2f getPosition() const;
    sf::Vector2f getSize() const;

    void setLabel(const std::string& label);
    void setCallback(std::function<void()> callback);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float deltaTime, const sf::RenderWindow& window);
    void click();

    // Style configuration
    void setColors(const sf::Color& normalBg, const sf::Color& hoverBg, const sf::Color& textCol);

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    RoundedRectangleShape m_background;
    sf::Text m_text;
    std::function<void()> m_callback;

    sf::Color m_normalBgColor;
    sf::Color m_hoverBgColor;
    sf::Color m_textColor;
    sf::Color m_currentBgColor;

    bool m_isHovered;
    bool m_isPressed;

    void centerText();
};

} // namespace ui

#endif // BUTTON_HPP
