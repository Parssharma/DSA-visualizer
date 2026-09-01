#include "core/ScreenManager.hpp"

namespace core {

void ScreenManager::changeScreen(std::unique_ptr<Screen> newScreen) {
    m_nextScreen = std::move(newScreen);
}

void ScreenManager::applyPendingScreen() {
    if (m_nextScreen) {
        m_currentScreen = std::move(m_nextScreen);
    }
}

void ScreenManager::handleEvent(sf::Event& event, sf::RenderWindow& window) {
    applyPendingScreen();
    if (m_currentScreen) {
        m_currentScreen->handleEvent(event, window);
    }
}

void ScreenManager::update(float deltaTime) {
    applyPendingScreen();
    if (m_currentScreen) {
        m_currentScreen->update(deltaTime);
    }
}

void ScreenManager::draw(sf::RenderWindow& window) {
    applyPendingScreen();
    if (m_currentScreen) {
        m_currentScreen->draw(window);
    }
}

} // namespace core
