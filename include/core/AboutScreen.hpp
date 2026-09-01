#ifndef ABOUT_SCREEN_HPP
#define ABOUT_SCREEN_HPP

#include "core/Screen.hpp"
#include "ui/Button.hpp"
#include "ui/Panel.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

namespace core {

class AboutScreen : public Screen {
public:
    explicit AboutScreen(sf::RenderWindow& window);
    virtual ~AboutScreen() = default;

    virtual void handleEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void update(float deltaTime) override;
    virtual void draw(sf::RenderWindow& window) override;

private:
    sf::RenderWindow* m_window;

    std::unique_ptr<ui::Button> m_backButton;
    std::unique_ptr<ui::Panel> m_contentPanel;

    sf::Text m_titleText;
    sf::Text m_descText;
    sf::Text m_techTitleText;
    sf::Text m_techBodyText;
    sf::Text m_devTitleText;
    sf::Text m_devBodyText;

    void initUI();
    void updateLayout(const sf::Vector2u& windowSize);
};

} // namespace core

#endif // ABOUT_SCREEN_HPP
