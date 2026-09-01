#ifndef PANEL_HPP
#define PANEL_HPP

#include "ui/RoundedRectangleShape.hpp"
#include <SFML/Graphics.hpp>
#include <string>

namespace ui {

class Panel : public sf::Drawable {
public:
    Panel(const sf::Vector2f& size = sf::Vector2f(200.f, 200.f), const std::string& title = "");

    void setPosition(const sf::Vector2f& position);
    sf::Vector2f getPosition() const;

    void setSize(const sf::Vector2f& size);
    sf::Vector2f getSize() const;

    void setTitle(const std::string& title);
    
    void setBackgroundColor(const sf::Color& color);
    void setBorderColor(const sf::Color& color);
    void setBorderThickness(float thickness);
    
    sf::FloatRect getContentBounds() const;

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    RoundedRectangleShape m_background;
    sf::Text m_titleText;
    bool m_hasTitle;
    
    void updateTitlePosition();
};

} // namespace ui

#endif // PANEL_HPP
