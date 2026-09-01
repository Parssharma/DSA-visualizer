#ifndef ROUNDED_RECTANGLE_SHAPE_HPP
#define ROUNDED_RECTANGLE_SHAPE_HPP

#include <SFML/Graphics/Shape.hpp>

namespace ui {

class RoundedRectangleShape : public sf::Shape {
public:
    explicit RoundedRectangleShape(const sf::Vector2f& size = sf::Vector2f(0.f, 0.f), float radius = 0.f, unsigned int cornerPointCount = 8);

    void setSize(const sf::Vector2f& size);
    const sf::Vector2f& getSize() const;

    void setCornerRadius(float radius);
    float getCornerRadius() const;

    void setCornerPointCount(unsigned int count);
    unsigned int getCornerPointCount() const;

    virtual std::size_t getPointCount() const override;
    virtual sf::Vector2f getPoint(std::size_t index) const override;

private:
    sf::Vector2f m_size;
    float m_radius;
    unsigned int m_cornerPointCount;
};

} // namespace ui

#endif // ROUNDED_RECTANGLE_SHAPE_HPP
