#ifndef SLIDER_HPP
#define SLIDER_HPP

#include <SFML/Graphics.hpp>
#include "ui/RoundedRectangleShape.hpp"
#include <functional>
#include <string>

namespace ui {

class Slider : public sf::Drawable {
public:
    Slider(const std::string& label, float minValue, float maxValue, float defaultValue, const sf::Vector2f& size = sf::Vector2f(200.f, 6.f));

    void setPosition(const sf::Vector2f& position);
    sf::Vector2f getPosition() const;
    sf::Vector2f getSize() const;

    float getValue() const;
    void setValue(float value);

    void setCallback(std::function<void(float)> callback);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float deltaTime, const sf::RenderWindow& window);

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    float m_minValue;
    float m_maxValue;
    float m_value;

    std::function<void(float)> m_callback;

    sf::Text m_label;
    sf::Text m_valueText;
    
    RoundedRectangleShape m_track;
    RoundedRectangleShape m_fillTrack;
    sf::CircleShape m_handle;

    bool m_isDragging;
    
    void updateElements();
};

} // namespace ui

#endif // SLIDER_HPP
