#include "core/AboutScreen.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include "core/ScreenManager.hpp"
#include "core/MainMenuScreen.hpp"

namespace core {

AboutScreen::AboutScreen(sf::RenderWindow& window) : m_window(&window) {
    initUI();
    updateLayout(window.getSize());
}

void AboutScreen::initUI() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // Back Button
    m_backButton = std::make_unique<ui::Button>("Back", sf::Vector2f(80.f, 32.f), 13);
    m_backButton->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_backButton->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<MainMenuScreen>(*m_window));
    });

    // Content Panel
    m_contentPanel = std::make_unique<ui::Panel>(sf::Vector2f(100.f, 100.f), "Project Information");

    // Title
    m_titleText.setFont(boldFont);
    m_titleText.setString("About DSA Visualizer");
    m_titleText.setCharacterSize(22);
    m_titleText.setFillColor(colors.textPrimary);

    // Description
    m_descText.setFont(regularFont);
    m_descText.setString("DSA Visualizer is an interactive educational dashboard designed to facilitate understanding of standard data structures and algorithmic steps through dynamic 2D visual simulations. The project aims to provide intuitive insights into sorting, searching, linked lists, trees, graphs, and stack/queue representations.");
    m_descText.setCharacterSize(13);
    m_descText.setFillColor(colors.textSecondary);

    // Tech Stack Header
    m_techTitleText.setFont(boldFont);
    m_techTitleText.setString("TECHNOLOGY STACK");
    m_techTitleText.setCharacterSize(14);
    m_techTitleText.setFillColor(colors.accent);

    // Tech Stack Detail
    m_techBodyText.setFont(regularFont);
    m_techBodyText.setString("- Programming Language Standard: C++17\n- Graphics / Windowing Library: SFML 2.6.1 (Statically Linked)\n- Build Automation System: CMake 3.14+\n- Primary Typography Family: Inter Google Font family\n- UI Rendering Paradigm: Custom RoundedRectangleShape styling system");
    m_techBodyText.setCharacterSize(12);
    m_techBodyText.setFillColor(colors.textSecondary);

    // Developer Header
    m_devTitleText.setFont(boldFont);
    m_devTitleText.setString("AUTHORS & CONTRIBUTORS");
    m_devTitleText.setCharacterSize(14);
    m_devTitleText.setFillColor(colors.accent);

    // Developer Details
    m_devBodyText.setFont(regularFont);
    m_devBodyText.setString("Designed and developed collaboratively by Antigravity and team. Under active project phases.\nSpecial thanks to the Google DeepMind team and SFML open source community.");
    m_devBodyText.setCharacterSize(12);
    m_devBodyText.setFillColor(colors.textSecondary);
}

void AboutScreen::updateLayout(const sf::Vector2u& windowSize) {
    float w = static_cast<float>(windowSize.x);
    float h = static_cast<float>(windowSize.y);

    float marginX = 40.f;
    float startY = 40.f;

    // Header items
    m_backButton->setPosition(sf::Vector2f(marginX, startY));
    m_titleText.setPosition(marginX + 100.f, startY + 2.f);

    // Panel layout
    float panelY = startY + 60.f;
    m_contentPanel->setPosition(sf::Vector2f(marginX, panelY));
    m_contentPanel->setSize(sf::Vector2f(w - 2.f * marginX, h - panelY - marginX));

    // Texts layout inside content panel bounds
    sf::FloatRect bounds = m_contentPanel->getContentBounds();
    m_descText.setPosition(bounds.left, bounds.top + 10.f);
    
    // Simple text wrapping for description to fit panel width
    // Just a rough estimate helper; bounds.width - 20.f
    std::string str = m_descText.getString();
    // We could wrap it dynamically, but since description text length is static, we can let standard wrapping logic (or manual newlines) work or write a simple wrap.
    // Let's write manual wrapping or just keep standard paragraph shape since width is ample (usually 1200px)
    
    m_techTitleText.setPosition(bounds.left, bounds.top + 100.f);
    m_techBodyText.setPosition(bounds.left, bounds.top + 125.f);
    
    m_devTitleText.setPosition(bounds.left, bounds.top + 230.f);
    m_devBodyText.setPosition(bounds.left, bounds.top + 255.f);
}

void AboutScreen::handleEvent(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::Resized) {
        sf::FloatRect visibleArea(0.f, 0.f, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
        window.setView(sf::View(visibleArea));
        updateLayout(window.getSize());
    }

    m_backButton->handleEvent(event, window);
}

void AboutScreen::update(float deltaTime) {
    if (m_window) {
        m_backButton->update(deltaTime, *m_window);
    }
}

void AboutScreen::draw(sf::RenderWindow& window) {
    window.clear(ThemeManager::getInstance().getColors().background);

    window.draw(*m_contentPanel);
    window.draw(*m_backButton);
    window.draw(m_titleText);

    // Draw inner text content
    window.draw(m_descText);
    window.draw(m_techTitleText);
    window.draw(m_techBodyText);
    window.draw(m_devTitleText);
    window.draw(m_devBodyText);
}

} // namespace core
