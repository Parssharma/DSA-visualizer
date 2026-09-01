#include "core/VisualizerBase.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include "core/ScreenManager.hpp"
#include "core/MainMenuScreen.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace core {

static std::string cleanTextString(const std::string& str) {
    std::string result = "";
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\n') {
            if (i + 1 < str.length() && str[i+1] == '\n') {
                result += "\n\n";
                i++; // skip next newline
            } else {
                result += " ";
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

static void wrapText(sf::Text& text, float width) {
    if (!text.getFont()) return;
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

VisualizerBase::VisualizerBase(const std::string& title)
    : m_title(title), m_isPlaying(false), m_animationSpeed(1.f), m_dataSize(50) {
}

void VisualizerBase::initCommonUI(sf::RenderWindow& window) {
    m_window = &window;
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& titleFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // Title text
    m_titleText.setFont(titleFont);
    m_titleText.setString(m_title);
    m_titleText.setCharacterSize(22);
    m_titleText.setFillColor(colors.textPrimary);

    // Initialize Panels
    m_controlPanel = std::make_unique<ui::Panel>(sf::Vector2f(100.f, 100.f));
    m_visualizationPanel = std::make_unique<ui::Panel>(sf::Vector2f(100.f, 100.f), "Visualization");
    m_infoPanel = std::make_unique<ui::Panel>(sf::Vector2f(100.f, 100.f), "Algorithm Info");

    // Back Button
    m_backButton = std::make_unique<ui::Button>("Back", sf::Vector2f(80.f, 32.f), 13);
    m_backButton->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_backButton->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<MainMenuScreen>(*m_window));
    });

    // Control Buttons
    m_playPauseButton = std::make_unique<ui::Button>("Play", sf::Vector2f(80.f, 36.f), 14);
    m_playPauseButton->setCallback([this]() {
        m_isPlaying = !m_isPlaying;
        m_playPauseButton->setLabel(m_isPlaying ? "Pause" : "Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        if (m_isPlaying) {
            m_playPauseButton->setColors(colors.success, colors.accentHover, colors.textPrimary);
        } else {
            m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        }
    });

    m_stepButton = std::make_unique<ui::Button>("Step", sf::Vector2f(80.f, 36.f), 14);
    m_stepButton->setCallback([this]() {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        onStep();
    });

    m_resetButton = std::make_unique<ui::Button>("Reset", sf::Vector2f(80.f, 36.f), 14);
    m_resetButton->setCallback([this]() {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        onReset();
    });

    // Sliders
    m_speedSlider = std::make_unique<ui::Slider>("Speed", 0.1f, 10.f, 1.f, sf::Vector2f(160.f, 5.f));
    m_speedSlider->setCallback([this](float val) {
        m_animationSpeed = val;
    });

    m_sizeSlider = std::make_unique<ui::Slider>("Data Size", 5.f, 200.f, 50.f, sf::Vector2f(160.f, 5.f));
    m_sizeSlider->setCallback([this](float val) {
        m_dataSize = static_cast<int>(val);
        onSizeChanged(m_dataSize);
    });

    // Info Text inside info panel
    m_complexityTitle.setFont(titleFont);
    m_complexityTitle.setString("Complexities");
    m_complexityTitle.setCharacterSize(15);
    m_complexityTitle.setFillColor(colors.accent);

    m_timeComplexity.setFont(regularFont);
    m_timeComplexity.setString("Time Complexity: O(N log N)");
    m_timeComplexity.setCharacterSize(13);
    m_timeComplexity.setFillColor(colors.textPrimary);

    m_spaceComplexity.setFont(regularFont);
    m_spaceComplexity.setString("Space Complexity: O(N)");
    m_spaceComplexity.setCharacterSize(13);
    m_spaceComplexity.setFillColor(colors.textPrimary);

    m_theoryTitle.setFont(titleFont);
    m_theoryTitle.setString("Description & Theory");
    m_theoryTitle.setCharacterSize(15);
    m_theoryTitle.setFillColor(colors.accent);

    m_theoryBody.setFont(regularFont);
    m_theoryBody.setString("Please select an algorithm or initialize array.\n\nUse control panel options below to step through simulation, pause, and adjust speed/size.");
    m_theoryBody.setCharacterSize(13);
    m_theoryBody.setFillColor(colors.textSecondary);

    // Global Help Overlay
    m_showHelpOverlay = false;
    m_helpOverlayPanel = std::make_unique<ui::Panel>(sf::Vector2f(420.f, 280.f), "Help Center");
    m_helpOverlayPanel->setBackgroundColor(sf::Color(26, 26, 36, 245));
    m_helpOverlayPanel->setBorderThickness(2.f);

    m_helpTitleText.setFont(titleFont);
    m_helpTitleText.setString("KEYBOARD SHORTCUTS & USER GUIDE");
    m_helpTitleText.setCharacterSize(14);
    m_helpTitleText.setFillColor(colors.accent);

    m_helpContentText.setFont(regularFont);
    m_helpContentText.setString(
        "[Space] - Play / Pause animation playback\n"
        "[S]     - Step forward one frame single instruction\n"
        "[R]     - Reset visualization queue/arrays\n"
        "[T]     - Cycle themes (Dark -> Light -> High Contrast)\n"
        "[H]     - Toggle Help Shortcuts Guide Overlay dialog\n"
        "[P]     - Export screenshot of canvas to dsa_screenshot.png\n"
        "[D]     - Export current data state snapshot to dsa_data.json\n\n"
        "Use the Left Panel tabs to choose algorithms. Use sliders\n"
        "at the top to customize speed rates and size inputs."
    );
    m_helpContentText.setCharacterSize(11);
    m_helpContentText.setFillColor(colors.textPrimary);

    m_helpCloseButton = std::make_unique<ui::Button>("Close", sf::Vector2f(90.f, 28.f), 12);
    m_helpCloseButton->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_helpCloseButton->setCallback([this]() { m_showHelpOverlay = false; });

    updateLayout(window.getSize());
}

void VisualizerBase::updateLayout(const sf::Vector2u& windowSize) {
    float w = static_cast<float>(windowSize.x);
    float h = static_cast<float>(windowSize.y);

    // Padding / margins
    float margin = 15.f;
    
    // Title & Back Button Layout
    m_backButton->setPosition(sf::Vector2f(margin, margin));
    m_titleText.setPosition(margin + 100.f, margin + 2.f);

    // Control panel (horizontal bar under header)
    float controlY = 60.f;
    float controlHeight = 65.f;
    m_controlPanel->setPosition(sf::Vector2f(margin, controlY));
    m_controlPanel->setSize(sf::Vector2f(w - 2.f * margin, controlHeight));

    // Align items inside control panel
    float startX = margin + 20.f;
    float centerY = controlY + (controlHeight - 36.f) / 2.f;

    m_playPauseButton->setPosition(sf::Vector2f(startX, centerY));
    m_stepButton->setPosition(sf::Vector2f(startX + 90.f, centerY));
    m_resetButton->setPosition(sf::Vector2f(startX + 180.f, centerY));

    // Sliders
    float sliderY = controlY + (controlHeight - 5.f) / 2.f + 5.f; // vertical centering offset for sliders
    m_speedSlider->setPosition(sf::Vector2f(startX + 300.f, sliderY));
    m_sizeSlider->setPosition(sf::Vector2f(startX + 500.f, sliderY));

    // Split area below control bar
    float contentY = controlY + controlHeight + margin;
    float contentHeight = h - contentY - margin;
    float visWidth = (w - 3.f * margin) * 0.72f;
    float infoWidth = (w - 3.f * margin) * 0.28f;

    m_visualizationPanel->setPosition(sf::Vector2f(margin, contentY));
    m_visualizationPanel->setSize(sf::Vector2f(visWidth, contentHeight));

    m_infoPanel->setPosition(sf::Vector2f(margin + visWidth + margin, contentY));
    m_infoPanel->setSize(sf::Vector2f(infoWidth, contentHeight));

    // Layout text elements inside Info Panel content bounds
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    m_complexityTitle.setPosition(infoBounds.left, infoBounds.top);
    m_timeComplexity.setPosition(infoBounds.left, infoBounds.top + 22.f);
    m_spaceComplexity.setPosition(infoBounds.left, infoBounds.top + 42.f);

    m_theoryTitle.setPosition(infoBounds.left, infoBounds.top + 80.f);
    m_theoryBody.setPosition(infoBounds.left, infoBounds.top + 105.f);

    // Help Center centering layout
    float hx = (w - 420.f) / 2.f;
    float hy = (h - 280.f) / 2.f;
    m_helpOverlayPanel->setPosition(sf::Vector2f(hx, hy));

    sf::FloatRect hpBounds = m_helpOverlayPanel->getContentBounds();
    m_helpTitleText.setPosition(hpBounds.left, hpBounds.top + 5.f);
    m_helpContentText.setPosition(hpBounds.left, hpBounds.top + 32.f);
    m_helpCloseButton->setPosition(sf::Vector2f(hpBounds.left + (hpBounds.width - 90.f) / 2.f, hpBounds.top + 215.f));
}

void VisualizerBase::handleEvent(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::Resized) {
        // Adjust views
        sf::FloatRect visibleArea(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
        window.setView(sf::View(visibleArea));
        updateLayout(window.getSize());
    }

    if (m_showHelpOverlay) {
        m_helpCloseButton->handleEvent(event, window);
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::H) {
            m_showHelpOverlay = false;
        }
        return;
    }

    // Keyboard Shortcuts
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Space) {
            m_playPauseButton->click();
        } else if (event.key.code == sf::Keyboard::S) {
            m_stepButton->click();
        } else if (event.key.code == sf::Keyboard::R) {
            m_resetButton->click();
        } else if (event.key.code == sf::Keyboard::H) {
            m_showHelpOverlay = true;
        } else if (event.key.code == sf::Keyboard::T) {
            ThemeType curr = ThemeManager::getInstance().getThemeType();
            ThemeType next = ThemeType::Dark;
            if (curr == ThemeType::Dark) next = ThemeType::Light;
            else if (curr == ThemeType::Light) next = ThemeType::HighContrast;
            ThemeManager::getInstance().setTheme(next);
            updateLayout(window.getSize());
        } else if (event.key.code == sf::Keyboard::P) {
            captureScreenshot();
        } else if (event.key.code == sf::Keyboard::D) {
            exportStateData();
        }
    }

    m_backButton->handleEvent(event, window);
    m_playPauseButton->handleEvent(event, window);
    m_stepButton->handleEvent(event, window);
    m_resetButton->handleEvent(event, window);
    m_speedSlider->handleEvent(event, window);
    m_sizeSlider->handleEvent(event, window);

    handleVisualsEvent(event, window);
}

void VisualizerBase::update(float deltaTime) {
    if (m_window) {
        if (m_showHelpOverlay) {
            m_helpCloseButton->update(deltaTime, *m_window);
        } else {
            m_backButton->update(deltaTime, *m_window);
            m_playPauseButton->update(deltaTime, *m_window);
            m_stepButton->update(deltaTime, *m_window);
            m_resetButton->update(deltaTime, *m_window);
            m_speedSlider->update(deltaTime, *m_window);
            m_sizeSlider->update(deltaTime, *m_window);
        }
    }
    updateVisuals(deltaTime);
}

void VisualizerBase::draw(sf::RenderWindow& window) {
    // Background fill
    window.clear(ThemeManager::getInstance().getColors().background);

    // Draw panels
    window.draw(*m_controlPanel);
    window.draw(*m_visualizationPanel);
    window.draw(*m_infoPanel);

    // Draw buttons & sliders
    window.draw(*m_backButton);
    window.draw(*m_playPauseButton);
    window.draw(*m_stepButton);
    window.draw(*m_resetButton);
    window.draw(*m_speedSlider);
    window.draw(*m_sizeSlider);

    // Draw title
    window.draw(m_titleText);

    // Draw complexity text inside info panel
    window.draw(m_complexityTitle);
    window.draw(m_timeComplexity);
    window.draw(m_spaceComplexity);
    window.draw(m_theoryTitle);

    // Clean and wrap theory body text if width or content changed
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    std::string currentStr = m_theoryBody.getString();
    if (currentStr != m_lastWrappedText || infoBounds.width != m_lastWrappedWidth) {
        std::string unwrappedStr = cleanTextString(currentStr);
        m_theoryBody.setString(unwrappedStr);
        wrapText(m_theoryBody, infoBounds.width);
        m_lastWrappedText = m_theoryBody.getString();
        m_lastWrappedWidth = infoBounds.width;
    }
    window.draw(m_theoryBody);

    // Draw subclass visuals inside visualization panel content bounds
    sf::View originalView = window.getView();
    sf::FloatRect visBounds = m_visualizationPanel->getContentBounds();
    
    // Setup viewport to clip visuals to panel boundaries
    sf::View visView;
    visView.reset(visBounds);
    
    sf::Vector2u winSize = window.getSize();
    visView.setViewport(sf::FloatRect(
        visBounds.left / winSize.x,
        visBounds.top / winSize.y,
        visBounds.width / winSize.x,
        visBounds.height / winSize.y
    ));

    window.setView(visView);
    drawVisuals(window);
    
    // Restore original view
    window.setView(originalView);

    // Draw Help Overlay on top of everything
    if (m_showHelpOverlay) {
        window.draw(*m_helpOverlayPanel);
        window.draw(m_helpTitleText);
        window.draw(m_helpContentText);
        window.draw(*m_helpCloseButton);
    }
}

void VisualizerBase::captureScreenshot() {
    if (!m_window) return;
    sf::Texture texture;
    texture.create(m_window->getSize().x, m_window->getSize().y);
    texture.update(*m_window);
    sf::Image img = texture.copyToImage();
    if (img.saveToFile("dsa_screenshot.png")) {
        std::cout << "[System] Screenshot exported to dsa_screenshot.png" << std::endl;
    }
}

void VisualizerBase::exportStateData() {
    // Default fallback state exporter
    std::ofstream file("dsa_data.json");
    if (file.is_open()) {
        file << "{\n";
        file << "  \"title\": \"" << m_title << "\",\n";
        file << "  \"dataSize\": " << m_dataSize << ",\n";
        file << "  \"speed\": " << m_animationSpeed << "\n";
        file << "}\n";
        file.close();
        std::cout << "[System] State data exported to dsa_data.json" << std::endl;
    }
}

} // namespace core
