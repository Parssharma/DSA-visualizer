#include "ui/Slider.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include <iomanip>
#include <sstream>

namespace ui {

Slider::Slider(const std::string& label, float minValue, float maxValue, float defaultValue, const sf::Vector2f& size)
    : m_minValue(minValue), m_maxValue(maxValue), m_value(defaultValue), m_isDragging(false) {
    
    const auto& colors = core::ThemeManager::getInstance().getColors();
    
    // Set up Track
    m_track.setSize(size);
    m_track.setCornerRadius(size.y / 2.f);
    m_track.setCornerPointCount(4);
    m_track.setFillColor(colors.border);
    
    // Set up Filled Track
    m_fillTrack.setSize(sf::Vector2f(0.f, size.y));
    m_fillTrack.setCornerRadius(size.y / 2.f);
    m_fillTrack.setCornerPointCount(4);
    m_fillTrack.setFillColor(colors.accent);

    // Set up Handle
    float radius = size.y * 1.5f;
    m_handle.setRadius(radius);
    m_handle.setOrigin(radius, radius);
    m_handle.setFillColor(colors.accent);
    m_handle.setOutlineColor(colors.textPrimary);
    m_handle.setOutlineThickness(1.5f);

    // Load Fonts
    sf::Font& regularFont = core::ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");
    
    m_label.setFont(regularFont);
    m_label.setString(label);
    m_label.setCharacterSize(13);
    m_label.setFillColor(colors.textSecondary);

    m_valueText.setFont(regularFont);
    m_valueText.setCharacterSize(13);
    m_valueText.setFillColor(colors.textPrimary);

    setValue(defaultValue);
}

void Slider::setPosition(const sf::Vector2f& position) {
    m_track.setPosition(position);
    updateElements();
}

sf::Vector2f Slider::getPosition() const {
    return m_track.getPosition();
}

sf::Vector2f Slider::getSize() const {
    return m_track.getSize();
}

float Slider::getValue() const {
    return m_value;
}

void Slider::setValue(float value) {
    m_value = std::max(m_minValue, std::min(m_maxValue, value));
    
    // Update value text
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << m_value;
    m_valueText.setString(ss.str());
    
    updateElements();
}

void Slider::setCallback(std::function<void(float)> callback) {
    m_callback = callback;
}

void Slider::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::FloatRect handleBounds = m_handle.getGlobalBounds();
    // Expand click boundary slightly for better UX
    handleBounds.left -= 5.f;
    handleBounds.top -= 5.f;
    handleBounds.width += 10.f;
    handleBounds.height += 10.f;

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (handleBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            m_isDragging = true;
        } else {
            // Check if track is clicked, jump to that position
            sf::FloatRect trackBounds = m_track.getGlobalBounds();
            // Pad hit area
            trackBounds.top -= 10.f;
            trackBounds.height += 20.f;
            if (trackBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                float localX = mousePos.x - trackBounds.left;
                float pct = localX / trackBounds.width;
                setValue(m_minValue + pct * (m_maxValue - m_minValue));
                m_isDragging = true;
                if (m_callback) {
                    m_callback(m_value);
                }
            }
        }
    }
    
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        m_isDragging = false;
    }
}

void Slider::update(float deltaTime, const sf::RenderWindow& window) {
    if (m_isDragging) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect trackBounds = m_track.getGlobalBounds();
        
        float localX = static_cast<float>(mousePos.x) - trackBounds.left;
        float pct = localX / trackBounds.width;
        pct = std::max(0.f, std::min(1.f, pct));
        
        setValue(m_minValue + pct * (m_maxValue - m_minValue));
        if (m_callback) {
            m_callback(m_value);
        }
    }
    
    // Light effect on handle hover
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::FloatRect handleBounds = m_handle.getGlobalBounds();
    const auto& colors = core::ThemeManager::getInstance().getColors();
    if (handleBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)) || m_isDragging) {
        m_handle.setFillColor(colors.accentHover);
    } else {
        m_handle.setFillColor(colors.accent);
    }
}

void Slider::updateElements() {
    sf::Vector2f trackPos = m_track.getPosition();
    sf::Vector2f trackSize = m_track.getSize();
    
    float ratio = (m_value - m_minValue) / (m_maxValue - m_minValue);
    if (m_maxValue - m_minValue == 0.f) ratio = 0.f;
    
    // Handle position
    float handleX = trackPos.x + ratio * trackSize.x;
    float handleY = trackPos.y + trackSize.y / 2.f;
    m_handle.setPosition(handleX, handleY);
    
    // Fill Track size
    m_fillTrack.setSize(sf::Vector2f(ratio * trackSize.x, trackSize.y));
    m_fillTrack.setPosition(trackPos);
    
    // Label positioning: top-left
    m_label.setPosition(trackPos.x, trackPos.y - 20.f);
    
    // Value text positioning: top-right
    sf::FloatRect valBounds = m_valueText.getLocalBounds();
    m_valueText.setPosition(trackPos.x + trackSize.x - valBounds.width, trackPos.y - 20.f);
}

void Slider::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_track, states);
    target.draw(m_fillTrack, states);
    target.draw(m_handle, states);
    target.draw(m_label, states);
    target.draw(m_valueText, states);
}

} // namespace ui
