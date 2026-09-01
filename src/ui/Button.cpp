#include "ui/Button.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include <iostream>

namespace ui {

Button::Button(const std::string& label, const sf::Vector2f& size, unsigned int fontSize)
    : m_isHovered(false), m_isPressed(false) {
    
    // Set default colors from ThemeManager
    const auto& colors = core::ThemeManager::getInstance().getColors();
    m_normalBgColor = colors.accent;
    m_hoverBgColor = colors.accentHover;
    m_textColor = colors.textPrimary;
    m_currentBgColor = m_normalBgColor;

    m_background.setSize(size);
    m_background.setCornerRadius(6.f);
    m_background.setCornerPointCount(8);
    m_background.setFillColor(m_currentBgColor);

    // Load font from ResourceManager
    sf::Font& font = core::ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");
    m_text.setFont(font);
    m_text.setString(label);
    m_text.setCharacterSize(fontSize);
    m_text.setFillColor(m_textColor);

    centerText();
}

void Button::setPosition(const sf::Vector2f& position) {
    m_background.setPosition(position);
    centerText();
}

sf::Vector2f Button::getPosition() const {
    return m_background.getPosition();
}

sf::Vector2f Button::getSize() const {
    return m_background.getSize();
}

void Button::setLabel(const std::string& label) {
    m_text.setString(label);
    centerText();
}

void Button::setCallback(std::function<void()> callback) {
    m_callback = callback;
}

void Button::setColors(const sf::Color& normalBg, const sf::Color& hoverBg, const sf::Color& textCol) {
    m_normalBgColor = normalBg;
    m_hoverBgColor = hoverBg;
    m_textColor = textCol;
    m_text.setFillColor(textCol);
    m_currentBgColor = m_normalBgColor;
    m_background.setFillColor(m_currentBgColor);
}

void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect bounds = m_background.getGlobalBounds();
        if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            m_isPressed = true;
        }
    }
    
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (m_isPressed) {
            m_isPressed = false;
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::FloatRect bounds = m_background.getGlobalBounds();
            if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                if (m_callback) {
                    m_callback();
                }
            }
        }
    }
}

void Button::update(float deltaTime, const sf::RenderWindow& window) {
    // Check if mouse hovers
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::FloatRect bounds = m_background.getGlobalBounds();
    m_isHovered = bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    // Linear interpolate button background color
    sf::Color targetColor = m_isHovered ? m_hoverBgColor : m_normalBgColor;
    
    float lerpSpeed = 10.f;
    float r = m_currentBgColor.r + (targetColor.r - m_currentBgColor.r) * lerpSpeed * deltaTime;
    float g = m_currentBgColor.g + (targetColor.g - m_currentBgColor.g) * lerpSpeed * deltaTime;
    float b = m_currentBgColor.b + (targetColor.b - m_currentBgColor.b) * lerpSpeed * deltaTime;
    float a = m_currentBgColor.a + (targetColor.a - m_currentBgColor.a) * lerpSpeed * deltaTime;
    
    m_currentBgColor = sf::Color(static_cast<sf::Uint8>(r), 
                                 static_cast<sf::Uint8>(g), 
                                 static_cast<sf::Uint8>(b), 
                                 static_cast<sf::Uint8>(a));
    
    m_background.setFillColor(m_currentBgColor);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_background, states);
    target.draw(m_text, states);
}

void Button::centerText() {
    sf::FloatRect textBounds = m_text.getLocalBounds();
    sf::FloatRect bgBounds = m_background.getGlobalBounds();
    
    // Set origin to center of text bounds
    m_text.setOrigin(textBounds.left + textBounds.width / 2.f, 
                     textBounds.top + textBounds.height / 2.f);
    
    // Place at center of background
    m_text.setPosition(bgBounds.left + bgBounds.width / 2.f, 
                       bgBounds.top + bgBounds.height / 2.f);
}

void Button::click() {
    if (m_callback) {
        m_callback();
    }
}

} // namespace ui
