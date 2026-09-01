#ifndef SCREEN_MANAGER_HPP
#define SCREEN_MANAGER_HPP

#include "core/Screen.hpp"
#include <memory>

namespace core {

class ScreenManager {
public:
    static ScreenManager& getInstance() {
        static ScreenManager instance;
        return instance;
    }

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

    void changeScreen(std::unique_ptr<Screen> newScreen);
    void handleEvent(sf::Event& event, sf::RenderWindow& window);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);

private:
    ScreenManager() = default;
    ~ScreenManager() = default;

    std::unique_ptr<Screen> m_currentScreen;
    std::unique_ptr<Screen> m_nextScreen;
    void applyPendingScreen();
};

} // namespace core

#endif // SCREEN_MANAGER_HPP
