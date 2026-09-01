#include "core/Application.hpp"
#include "core/ScreenManager.hpp"
#include "core/MainMenuScreen.hpp"
#include "core/ResourceManager.hpp"
#include <iostream>

namespace core {

Application::Application() : m_title("DSA Visualizer"), m_defaultSize(1280, 720) {
    initWindow();
}

Application::~Application() {
    ResourceManager::getInstance().clear();
}

void Application::initWindow() {
    m_window.create(sf::VideoMode(m_defaultSize.x, m_defaultSize.y), m_title, sf::Style::Default);
    m_window.setFramerateLimit(60);
    
    // Set initial screen to the Main Dashboard Menu
    ScreenManager::getInstance().changeScreen(std::make_unique<MainMenuScreen>(m_window));
}

void Application::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }
        ScreenManager::getInstance().handleEvent(event, m_window);
    }
}

void Application::update(float deltaTime) {
    ScreenManager::getInstance().update(deltaTime);
}

void Application::render() {
    ScreenManager::getInstance().draw(m_window);
    m_window.display();
}

void Application::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        
        processEvents();
        update(deltaTime);
        render();
    }
}

} // namespace core
