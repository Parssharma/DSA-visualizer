#include "ui/Panel.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"

namespace ui {

Panel::Panel(const sf::Vector2f& size, const std::string& title)
    : m_hasTitle(!title.empty()) {
    
    const auto& colors = core::ThemeManager::getInstance().getColors();
    
    m_background.setSize(size);
    m_background.setCornerRadius(8.f);
    m_background.setCornerPointCount(8);
    m_background.setFillColor(colors.panelBg);
    m_background.setOutlineColor(colors.border);
    m_background.setOutlineThickness(1.5f);

    sf::Font& font = core::ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    m_titleText.setFont(font);
    m_titleText.setString(title);
    m_titleText.setCharacterSize(15);
    m_titleText.setFillColor(colors.textPrimary);
    
    updateTitlePosition();
}

void Panel::setPosition(const sf::Vector2f& position) {
    m_background.setPosition(position);
    updateTitlePosition();
}

sf::Vector2f Panel::getPosition() const {
    return m_background.getPosition();
}

void Panel::setSize(const sf::Vector2f& size) {
    m_background.setSize(size);
    updateTitlePosition();
}

sf::Vector2f Panel::getSize() const {
    return m_background.getSize();
}

void Panel::setTitle(const std::string& title) {
    m_titleText.setString(title);
    m_hasTitle = !title.empty();
    updateTitlePosition();
}

void Panel::setBackgroundColor(const sf::Color& color) {
    m_background.setFillColor(color);
}

void Panel::setBorderColor(const sf::Color& color) {
    m_background.setOutlineColor(color);
}

void Panel::setBorderThickness(float thickness) {
    m_background.setOutlineThickness(thickness);
}

sf::FloatRect Panel::getContentBounds() const {
    sf::FloatRect bounds = m_background.getGlobalBounds();
    if (m_hasTitle) {
        // Offset content area by title height
        float headerOffset = 35.f;
        return sf::FloatRect(bounds.left + 10.f, bounds.top + headerOffset, 
                             bounds.width - 20.f, bounds.height - headerOffset - 10.f);
    }
    return sf::FloatRect(bounds.left + 10.f, bounds.top + 10.f, 
                         bounds.width - 20.f, bounds.height - 20.f);
}

void Panel::updateTitlePosition() {
    sf::Vector2f pos = m_background.getPosition();
    m_titleText.setPosition(pos.x + 15.f, pos.y + 12.f);
}

void Panel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_background, states);
    if (m_hasTitle) {
        target.draw(m_titleText, states);
    }
}

} // namespace ui
