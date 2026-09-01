#include "ui/RoundedRectangleShape.hpp"
#include <cmath>

namespace ui {

RoundedRectangleShape::RoundedRectangleShape(const sf::Vector2f& size, float radius, unsigned int cornerPointCount)
    : m_size(size), m_radius(radius), m_cornerPointCount(cornerPointCount) {
    update();
}

void RoundedRectangleShape::setSize(const sf::Vector2f& size) {
    m_size = size;
    update();
}

const sf::Vector2f& RoundedRectangleShape::getSize() const {
    return m_size;
}

void RoundedRectangleShape::setCornerRadius(float radius) {
    m_radius = radius;
    update();
}

float RoundedRectangleShape::getCornerRadius() const {
    return m_radius;
}

void RoundedRectangleShape::setCornerPointCount(unsigned int count) {
    m_cornerPointCount = count;
    update();
}

std::size_t RoundedRectangleShape::getPointCount() const {
    return m_cornerPointCount * 4;
}

sf::Vector2f RoundedRectangleShape::getPoint(std::size_t index) const {
    if (m_radius <= 0.f) {
        std::size_t corner = index / m_cornerPointCount;
        switch (corner) {
            default:
            case 0: return {m_size.x, 0.f};
            case 1: return {0.f, 0.f};
            case 2: return {0.f, m_size.y};
            case 3: return {m_size.x, m_size.y};
        }
    }

    std::size_t corner = index / m_cornerPointCount;
    std::size_t pointIndex = index % m_cornerPointCount;
    
    // Convert angle to radians
    // 0 to 90 degrees for corner 0 (Top-Right)
    // 90 to 180 degrees for corner 1 (Top-Left)
    // 180 to 270 degrees for corner 2 (Bottom-Left)
    // 270 to 360 degrees for corner 3 (Bottom-Right)
    float angle = (corner * 90.f + (static_cast<float>(pointIndex) / (m_cornerPointCount - 1)) * 90.f) * 3.14159265f / 180.f;
    
    sf::Vector2f center;
    switch (corner) {
        case 0: // Top-Right
            center = {m_size.x - m_radius, m_radius};
            break;
        case 1: // Top-Left
            center = {m_radius, m_radius};
            break;
        case 2: // Bottom-Left
            center = {m_radius, m_size.y - m_radius};
            break;
        case 3: // Bottom-Right
            center = {m_size.x - m_radius, m_size.y - m_radius};
            break;
    }
    
    return center + sf::Vector2f(std::cos(angle) * m_radius, -std::sin(angle) * m_radius);
}

} // namespace ui
