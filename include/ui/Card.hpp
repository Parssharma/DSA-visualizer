#ifndef CARD_HPP
#define CARD_HPP

#include "ui/RoundedRectangleShape.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

namespace ui {

class Card : public sf::Drawable {
public:
    Card(const std::string& title, const std::string& description, const sf::Vector2f& size = sf::Vector2f(220.f, 140.f));

    void setPosition(const sf::Vector2f& position);
    sf::Vector2f getPosition() const;
    sf::Vector2f getSize() const;

    void setCallback(std::function<void()> callback);
    void setStatus(const std::string& statusText, const sf::Color& statusColor);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float deltaTime, const sf::RenderWindow& window);

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    sf::Vector2f m_basePosition;
    sf::Vector2f m_size;
    std::function<void()> m_callback;

    RoundedRectangleShape m_background;
    sf::Text m_titleText;
    sf::Text m_descText;
    
    // Status Badge
    bool m_hasStatus;
    std::string m_statusString;
    sf::Color m_statusColor;
    RoundedRectangleShape m_statusBadge;
    sf::Text m_statusText;

    bool m_isHovered;
    bool m_isPressed;
    float m_hoverProgress; // 0.f to 1.f

    void updateElements();
};

} // namespace ui

#endif // CARD_HPP
