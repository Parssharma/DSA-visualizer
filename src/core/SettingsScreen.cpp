#include "core/SettingsScreen.hpp"
#include "core/SettingsManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include "core/ScreenManager.hpp"
#include "core/MainMenuScreen.hpp"
#include <iostream>

namespace core {

SettingsScreen::SettingsScreen(sf::RenderWindow& window)
    : m_window(&window),
      m_tempArraySize(SettingsManager::getInstance().getDefaultArraySize()),
      m_tempSoundEnabled(SettingsManager::getInstance().isSoundEnabled()) {
    
    initUI();
}

void SettingsScreen::initUI() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    m_titleText.setFont(boldFont);
    m_titleText.setString("SYSTEM CONFIGURATION");
    m_titleText.setCharacterSize(22);
    m_titleText.setFillColor(colors.textPrimary);

    m_descText.setFont(regularFont);
    m_descText.setString("Adjust visual themes, animation ratios, default dimensions, and accessibility guidelines.");
    m_descText.setCharacterSize(12);
    m_descText.setFillColor(colors.textSecondary);

    m_contentPanel = std::make_unique<ui::Panel>(sf::Vector2f(100.f, 100.f), "Preferences");

    // Back Button
    m_backButton = std::make_unique<ui::Button>("Back", sf::Vector2f(80.f, 32.f), 13);
    m_backButton->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_backButton->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<MainMenuScreen>(*m_window));
    });

    // 1. Theme selection label & buttons
    m_themeLabel.setFont(boldFont);
    m_themeLabel.setString("Select Visual Theme:");
    m_themeLabel.setCharacterSize(13);
    m_themeLabel.setFillColor(colors.accent);

    std::vector<std::string> themeNames = {"Dark Mode", "Light Mode", "High Contrast"};
    for (size_t i = 0; i < 3; ++i) {
        auto btn = std::make_unique<ui::Button>(themeNames[i], sf::Vector2f(120.f, 30.f), 11);
        ThemeType type = static_cast<ThemeType>(i);
        btn->setCallback([this, type]() {
            SettingsManager::getInstance().setTheme(type);
            initUI(); // Re-initialize colors
            updateLayout(m_window->getSize());
        });
        m_themeButtons.push_back(std::move(btn));
    }

    // Update active button state colors
    ThemeType currentTheme = SettingsManager::getInstance().getTheme();
    for (size_t i = 0; i < 3; ++i) {
        if (static_cast<size_t>(currentTheme) == i) {
            m_themeButtons[i]->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        } else {
            m_themeButtons[i]->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        }
    }

    // 2. Default Array Size label & adjusters
    m_arraySizeLabel.setFont(boldFont);
    m_arraySizeLabel.setString("Default Array Size (N):");
    m_arraySizeLabel.setCharacterSize(13);
    m_arraySizeLabel.setFillColor(colors.accent);

    m_decSizeBtn = std::make_unique<ui::Button>("-", sf::Vector2f(30.f, 30.f), 12);
    m_decSizeBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_decSizeBtn->setCallback([this]() {
        m_tempArraySize = std::max(5, m_tempArraySize - 5);
        m_sizeValBtn->setLabel(std::to_string(m_tempArraySize));
    });

    m_incSizeBtn = std::make_unique<ui::Button>("+", sf::Vector2f(30.f, 30.f), 12);
    m_incSizeBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_incSizeBtn->setCallback([this]() {
        m_tempArraySize = std::min(200, m_tempArraySize + 5);
        m_sizeValBtn->setLabel(std::to_string(m_tempArraySize));
    });

    m_sizeValBtn = std::make_unique<ui::Button>(std::to_string(m_tempArraySize), sf::Vector2f(80.f, 30.f), 12);
    m_sizeValBtn->setColors(colors.cardBg, colors.cardBg, colors.textPrimary);

    // 3. Sound Effects Toggle
    m_soundLabel.setFont(boldFont);
    m_soundLabel.setString("Audio Sound Effects:");
    m_soundLabel.setCharacterSize(13);
    m_soundLabel.setFillColor(colors.accent);

    std::string soundLabelStr = m_tempSoundEnabled ? "Sound: ON" : "Sound: OFF";
    sf::Color soundColor = m_tempSoundEnabled ? colors.success : colors.panelBg;
    m_soundToggleBtn = std::make_unique<ui::Button>(soundLabelStr, sf::Vector2f(100.f, 30.f), 11);
    m_soundToggleBtn->setColors(soundColor, colors.accentHover, colors.textPrimary);
    m_soundToggleBtn->setCallback([this]() {
        m_tempSoundEnabled = !m_tempSoundEnabled;
        const auto& colors = ThemeManager::getInstance().getColors();
        m_soundToggleBtn->setLabel(m_tempSoundEnabled ? "Sound: ON" : "Sound: OFF");
        m_soundToggleBtn->setColors(m_tempSoundEnabled ? colors.success : colors.panelBg, colors.accentHover, colors.textPrimary);
    });

    // 4. Interface Font Scale
    m_fontScaleLabel.setFont(boldFont);
    m_fontScaleLabel.setString("Interface Font Scale:");
    m_fontScaleLabel.setCharacterSize(13);
    m_fontScaleLabel.setFillColor(colors.accent);

    float currentScale = SettingsManager::getInstance().getFontScale();
    m_fontScaleSlider = std::make_unique<ui::Slider>("Scale", 0.8f, 1.5f, currentScale, sf::Vector2f(200.f, 6.f));

    // Save Button
    m_saveButton = std::make_unique<ui::Button>("Apply & Save", sf::Vector2f(130.f, 36.f), 13);
    m_saveButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
    m_saveButton->setCallback([this]() {
        SettingsManager::getInstance().setDefaultArraySize(m_tempArraySize);
        SettingsManager::getInstance().setSoundEnabled(m_tempSoundEnabled);
        SettingsManager::getInstance().setFontScale(m_fontScaleSlider->getValue());
        
        ScreenManager::getInstance().changeScreen(std::make_unique<MainMenuScreen>(*m_window));
    });

    updateLayout(m_window->getSize());
}

void SettingsScreen::updateLayout(const sf::Vector2u& windowSize) {
    float w = static_cast<float>(windowSize.x);
    float h = static_cast<float>(windowSize.y);
    float margin = 20.f;

    m_backButton->setPosition(sf::Vector2f(margin, margin));
    m_titleText.setPosition(margin + 100.f, margin + 2.f);
    m_descText.setPosition(margin + 100.f, margin + 28.f);

    float panelY = margin + 55.f;
    m_contentPanel->setPosition(sf::Vector2f(margin, panelY));
    m_contentPanel->setSize(sf::Vector2f(w - 2.f * margin, h - panelY - margin));

    sf::FloatRect bounds = m_contentPanel->getContentBounds();
    float startX = bounds.left + 25.f;
    float startY = bounds.top + 15.f;

    // Layout theme rows
    m_themeLabel.setPosition(startX, startY);
    for (size_t i = 0; i < 3; ++i) {
        m_themeButtons[i]->setPosition(sf::Vector2f(startX + 180.f + i * 130.f, startY - 8.f));
    }

    // Size adjustments row
    float sizeRowY = startY + 45.f;
    m_arraySizeLabel.setPosition(startX, sizeRowY);
    m_decSizeBtn->setPosition(sf::Vector2f(startX + 180.f, sizeRowY - 8.f));
    m_sizeValBtn->setPosition(sf::Vector2f(startX + 180.f + 35.f, sizeRowY - 8.f));
    m_incSizeBtn->setPosition(sf::Vector2f(startX + 180.f + 35.f + 85.f, sizeRowY - 8.f));

    // Sound toggle row
    float soundRowY = sizeRowY + 45.f;
    m_soundLabel.setPosition(startX, soundRowY);
    m_soundToggleBtn->setPosition(sf::Vector2f(startX + 180.f, soundRowY - 8.f));

    // Font Scale row
    float fontRowY = soundRowY + 45.f;
    m_fontScaleLabel.setPosition(startX, fontRowY);
    m_fontScaleSlider->setPosition(sf::Vector2f(startX + 180.f, fontRowY));

    // Save button placement
    m_saveButton->setPosition(sf::Vector2f(bounds.left + (bounds.width - 130.f) / 2.f, bounds.top + bounds.height - 50.f));
}

void SettingsScreen::handleEvent(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::Resized) {
        sf::FloatRect visibleArea(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
        window.setView(sf::View(visibleArea));
        updateLayout(window.getSize());
    }

    m_backButton->handleEvent(event, window);
    for (auto& btn : m_themeButtons) btn->handleEvent(event, window);
    m_decSizeBtn->handleEvent(event, window);
    m_incSizeBtn->handleEvent(event, window);
    m_sizeValBtn->handleEvent(event, window);
    m_soundToggleBtn->handleEvent(event, window);
    m_fontScaleSlider->handleEvent(event, window);
    m_saveButton->handleEvent(event, window);
}

void SettingsScreen::update(float deltaTime) {
    m_backButton->update(deltaTime, *m_window);
    for (auto& btn : m_themeButtons) btn->update(deltaTime, *m_window);
    m_decSizeBtn->update(deltaTime, *m_window);
    m_incSizeBtn->update(deltaTime, *m_window);
    m_sizeValBtn->update(deltaTime, *m_window);
    m_soundToggleBtn->update(deltaTime, *m_window);
    m_fontScaleSlider->update(deltaTime, *m_window);
    m_saveButton->update(deltaTime, *m_window);
}

void SettingsScreen::draw(sf::RenderWindow& window) {
    window.clear(ThemeManager::getInstance().getColors().background);

    window.draw(*m_contentPanel);

    window.draw(*m_backButton);
    window.draw(m_titleText);
    window.draw(m_descText);

    // Labels
    window.draw(m_themeLabel);
    window.draw(m_arraySizeLabel);
    window.draw(m_soundLabel);
    window.draw(m_fontScaleLabel);

    // Controls
    for (auto& btn : m_themeButtons) window.draw(*btn);
    window.draw(*m_decSizeBtn);
    window.draw(*m_sizeValBtn);
    window.draw(*m_incSizeBtn);
    window.draw(*m_soundToggleBtn);
    window.draw(*m_fontScaleSlider);
    window.draw(*m_saveButton);
}

} // namespace core
