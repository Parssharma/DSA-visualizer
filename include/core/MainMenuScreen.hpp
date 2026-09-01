#ifndef MAIN_MENU_SCREEN_HPP
#define MAIN_MENU_SCREEN_HPP

#include "core/Screen.hpp"
#include "ui/Card.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

namespace core {

class MainMenuScreen : public Screen {
public:
    explicit MainMenuScreen(sf::RenderWindow& window);
    virtual ~MainMenuScreen() = default;

    virtual void handleEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void update(float deltaTime) override;
    virtual void draw(sf::RenderWindow& window) override;

private:
    sf::RenderWindow* m_window;

    sf::Text m_titleText;
    sf::Text m_subtitleText;

    // Section headers
    sf::Text m_algoHeader;
    sf::Text m_dsHeader;
    sf::Text m_systemHeader;

    // Card collections
    std::vector<std::unique_ptr<ui::Card>> m_algoCards;
    std::vector<std::unique_ptr<ui::Card>> m_dsCards;
    std::vector<std::unique_ptr<ui::Card>> m_systemCards;

    void initUI();
    void updateLayout(const sf::Vector2u& windowSize);
};

} // namespace core

#endif // MAIN_MENU_SCREEN_HPP
