#include "ui/Card.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include <iostream>

namespace ui {

static void wrapText(sf::Text& text, float width) {
    std::string str = text.getString();
    std::string wrapped = "";
    std::string word = "";
    float spaceWidth = text.getFont()->getGlyph(' ', text.getCharacterSize(), false).advance;
    float currentWidth = 0.f;
    
    for (char c : str) {
        if (c == '\n') {
            wrapped += word + "\n";
            word = "";
            currentWidth = 0.f;
        } else if (c == ' ') {
            float wordWidth = 0.f;
            for (char wc : word) {
                wordWidth += text.getFont()->getGlyph(wc, text.getCharacterSize(), false).advance;
            }
            if (currentWidth + wordWidth + spaceWidth > width) {
                wrapped += "\n" + word;
                currentWidth = wordWidth;
            } else {
                if (!wrapped.empty() && wrapped.back() != '\n') {
                    wrapped += " ";
                    currentWidth += spaceWidth;
                }
                wrapped += word;
                currentWidth += wordWidth;
            }
            word = "";
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        float wordWidth = 0.f;
        for (char wc : word) {
            wordWidth += text.getFont()->getGlyph(wc, text.getCharacterSize(), false).advance;
        }
        if (currentWidth + wordWidth > width) {
            wrapped += "\n" + word;
        } else {
            if (!wrapped.empty() && wrapped.back() != '\n') {
                wrapped += " ";
            }
            wrapped += word;
        }
    }
    text.setString(wrapped);
}

Card::Card(const std::string& title, const std::string& description, const sf::Vector2f& size)
    : m_size(size), m_isHovered(false), m_isPressed(false), m_hoverProgress(0.f), m_hasStatus(false) {
    
    const auto& colors = core::ThemeManager::getInstance().getColors();

    m_background.setSize(size);
    m_background.setCornerRadius(10.f);
    m_background.setCornerPointCount(8);
    m_background.setFillColor(colors.cardBg);
    m_background.setOutlineColor(colors.border);
    m_background.setOutlineThickness(1.f);

    // Fonts
    sf::Font& boldFont = core::ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = core::ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // Title Text
    m_titleText.setFont(boldFont);
    m_titleText.setString(title);
    m_titleText.setCharacterSize(15);
    m_titleText.setFillColor(colors.textPrimary);

    // Description Text
    m_descText.setFont(regularFont);
    m_descText.setString(description);
    m_descText.setCharacterSize(11);
    m_descText.setFillColor(colors.textSecondary);

    // Limit wrap width
    wrapText(m_descText, m_size.x - 30.f);

    updateElements();
}

void Card::setPosition(const sf::Vector2f& position) {
    m_basePosition = position;
    updateElements();
}

sf::Vector2f Card::getPosition() const {
    return m_basePosition;
}

sf::Vector2f Card::getSize() const {
    return m_size;
}

void Card::setCallback(std::function<void()> callback) {
    m_callback = callback;
}

void Card::setStatus(const std::string& statusText, const sf::Color& statusColor) {
    m_hasStatus = true;
    m_statusString = statusText;
    m_statusColor = statusColor;

    sf::Font& regularFont = core::ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");
    m_statusText.setFont(regularFont);
    m_statusText.setString(statusText);
    m_statusText.setCharacterSize(9);
    m_statusText.setFillColor(sf::Color::White);

    // Setup status badge rounded rectangle
    sf::FloatRect textBounds = m_statusText.getLocalBounds();
    m_statusBadge.setSize(sf::Vector2f(textBounds.width + 12.f, textBounds.height + 6.f));
    m_statusBadge.setCornerRadius(4.f);
    m_statusBadge.setCornerPointCount(6);
    m_statusBadge.setFillColor(statusColor);

    updateElements();
}

void Card::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    // Check if mouse hovers currently
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::FloatRect bounds = m_background.getGlobalBounds();
    bool isInside = bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (isInside) {
            m_isPressed = true;
        }
    }
    
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (m_isPressed) {
            m_isPressed = false;
            if (isInside && m_callback) {
                m_callback();
            }
        }
    }
}

void Card::update(float deltaTime, const sf::RenderWindow& window) {
    // Check hover state
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::FloatRect bounds = m_background.getGlobalBounds();
    m_isHovered = bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    // Animate hover progress
    float animationSpeed = 12.f;
    if (m_isHovered) {
        m_hoverProgress += deltaTime * animationSpeed;
        if (m_hoverProgress > 1.f) m_hoverProgress = 1.f;
    } else {
        m_hoverProgress -= deltaTime * animationSpeed;
        if (m_hoverProgress < 0.f) m_hoverProgress = 0.f;
    }

    // Interactive zoom effect (up by 3% scale and offset upward by 5px)
    float currentScale = 1.f + 0.03f * m_hoverProgress;
    float offsetY = -5.f * m_hoverProgress;

    sf::Vector2f currentSize = m_size * currentScale;
    sf::Vector2f center = m_basePosition + m_size / 2.f;
    sf::Vector2f currentPos = center - currentSize / 2.f + sf::Vector2f(0.f, offsetY);

    m_background.setSize(currentSize);
    m_background.setPosition(currentPos);

    // Border transition colors (interpolate between border and accent)
    const auto& colors = core::ThemeManager::getInstance().getColors();
    sf::Color normalBorder = colors.border;
    sf::Color hoverBorder = colors.accent;

    float r = normalBorder.r + (hoverBorder.r - normalBorder.r) * m_hoverProgress;
    float g = normalBorder.g + (hoverBorder.g - normalBorder.g) * m_hoverProgress;
    float b = normalBorder.b + (hoverBorder.b - normalBorder.b) * m_hoverProgress;
    float a = normalBorder.a + (hoverBorder.a - normalBorder.a) * m_hoverProgress;

    m_background.setOutlineColor(sf::Color(static_cast<sf::Uint8>(r),
                                           static_cast<sf::Uint8>(g),
                                           static_cast<sf::Uint8>(b),
                                           static_cast<sf::Uint8>(a)));
    m_background.setOutlineThickness(1.f + 1.f * m_hoverProgress);

    // Update coordinates of child elements
    float marginX = 14.f * currentScale;
    float marginY = 14.f * currentScale;

    m_titleText.setPosition(currentPos.x + marginX, currentPos.y + marginY);
    m_titleText.setScale(currentScale, currentScale);

    m_descText.setPosition(currentPos.x + marginX, currentPos.y + marginY + 26.f * currentScale);
    m_descText.setScale(currentScale, currentScale);

    if (m_hasStatus) {
        float badgeW = m_statusBadge.getSize().x * currentScale;
        float badgeH = m_statusBadge.getSize().y * currentScale;

        // Position at the top right inside the card
        sf::Vector2f badgePos(currentPos.x + currentSize.x - badgeW - marginX, currentPos.y + marginY);
        m_statusBadge.setPosition(badgePos);
        m_statusBadge.setSize(m_statusBadge.getSize()); // keep base size for shape, use scale for drawing
        m_statusBadge.setScale(currentScale, currentScale);

        // Center text in badge
        sf::FloatRect textBounds = m_statusText.getLocalBounds();
        sf::Vector2f textPos(badgePos.x + (badgeW - textBounds.width * currentScale) / 2.f - textBounds.left * currentScale,
                             badgePos.y + (badgeH - textBounds.height * currentScale) / 2.f - textBounds.top * currentScale);
        m_statusText.setPosition(textPos);
        m_statusText.setScale(currentScale, currentScale);
    }
}

void Card::updateElements() {
    // Initial static updates (before dynamic hover animation)
    m_background.setPosition(m_basePosition);
    m_background.setSize(m_size);

    float marginX = 14.f;
    float marginY = 14.f;

    m_titleText.setPosition(m_basePosition.x + marginX, m_basePosition.y + marginY);
    m_descText.setPosition(m_basePosition.x + marginX, m_basePosition.y + marginY + 26.f);

    if (m_hasStatus) {
        float badgeW = m_statusBadge.getSize().x;
        float badgeH = m_statusBadge.getSize().y;

        sf::Vector2f badgePos(m_basePosition.x + m_size.x - badgeW - marginX, m_basePosition.y + marginY);
        m_statusBadge.setPosition(badgePos);

        sf::FloatRect textBounds = m_statusText.getLocalBounds();
        sf::Vector2f textPos(badgePos.x + (badgeW - textBounds.width) / 2.f - textBounds.left,
                             badgePos.y + (badgeH - textBounds.height) / 2.f - textBounds.top);
        m_statusText.setPosition(textPos);
    }
}

void Card::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_background, states);
    target.draw(m_titleText, states);
    target.draw(m_descText, states);
    if (m_hasStatus) {
        target.draw(m_statusBadge, states);
        target.draw(m_statusText, states);
    }
}

} // namespace ui
