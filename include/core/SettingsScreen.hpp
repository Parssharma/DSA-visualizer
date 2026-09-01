#ifndef SETSETTINGS_SCREEN_HPP
#define SETSETTINGS_SCREEN_HPP

#include "core/Screen.hpp"
#include "ui/Button.hpp"
#include "ui/Slider.hpp"
#include "ui/Panel.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

namespace core {

class SettingsScreen : public Screen {
public:
    explicit SettingsScreen(sf::RenderWindow& window);
    virtual ~SettingsScreen() = default;

    virtual void handleEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void update(float deltaTime) override;
    virtual void draw(sf::RenderWindow& window) override;

private:
    sf::RenderWindow* m_window;

    std::unique_ptr<ui::Button> m_backButton;
    std::unique_ptr<ui::Panel> m_contentPanel;

    sf::Text m_titleText;
    sf::Text m_descText;

    // Controls
    sf::Text m_themeLabel;
    std::vector<std::unique_ptr<ui::Button>> m_themeButtons; // Dark, Light, High Contrast

    sf::Text m_arraySizeLabel;
    std::unique_ptr<ui::Button> m_decSizeBtn;
    std::unique_ptr<ui::Button> m_incSizeBtn;
    std::unique_ptr<ui::Button> m_sizeValBtn;

    sf::Text m_soundLabel;
    std::unique_ptr<ui::Button> m_soundToggleBtn;

    sf::Text m_fontScaleLabel;
    std::unique_ptr<ui::Slider> m_fontScaleSlider;

    std::unique_ptr<ui::Button> m_saveButton;

    // State parameters
    int m_tempArraySize;
    bool m_tempSoundEnabled;

    void initUI();
    void updateLayout(const sf::Vector2u& windowSize);
};

} // namespace core

#endif // SETSETTINGS_SCREEN_HPP
