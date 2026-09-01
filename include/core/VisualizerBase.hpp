#ifndef VISUALIZER_BASE_HPP
#define VISUALIZER_BASE_HPP

#include "core/Screen.hpp"
#include "ui/Button.hpp"
#include "ui/Slider.hpp"
#include "ui/Panel.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

namespace core {

class VisualizerBase : public Screen {
public:
    explicit VisualizerBase(const std::string& title);
    virtual ~VisualizerBase() = default;

    virtual void handleEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void update(float deltaTime) override;
    virtual void draw(sf::RenderWindow& window) override;

protected:
    std::string m_title;
    sf::RenderWindow* m_window = nullptr;
    
    // UI Controls
    std::unique_ptr<ui::Button> m_backButton;
    std::unique_ptr<ui::Button> m_playPauseButton;
    std::unique_ptr<ui::Button> m_stepButton;
    std::unique_ptr<ui::Button> m_resetButton;
    
    std::unique_ptr<ui::Slider> m_speedSlider;
    std::unique_ptr<ui::Slider> m_sizeSlider;
    
    // Layout Panels
    std::unique_ptr<ui::Panel> m_controlPanel;
    std::unique_ptr<ui::Panel> m_infoPanel;
    std::unique_ptr<ui::Panel> m_visualizationPanel;
    
    // Playback state
    bool m_isPlaying;
    float m_animationSpeed;
    int m_dataSize;
    
    // Text labels
    sf::Text m_titleText;
    
    // Complexity texts (displayed in info panel)
    sf::Text m_complexityTitle;
    sf::Text m_timeComplexity;
    sf::Text m_spaceComplexity;
    sf::Text m_theoryTitle;
    sf::Text m_theoryBody;
    
    // Global Help Overlay & Shortcuts
    bool m_showHelpOverlay;
    std::unique_ptr<ui::Panel> m_helpOverlayPanel;
    std::unique_ptr<ui::Button> m_helpCloseButton;
    sf::Text m_helpTitleText;
    sf::Text m_helpContentText;

    std::string m_lastWrappedText;
    float m_lastWrappedWidth = 0.f;

    void initCommonUI(sf::RenderWindow& window);
    void updateLayout(const sf::Vector2u& windowSize);
    void captureScreenshot();
    virtual void exportStateData();
    
    // Pure virtual callbacks for subclasses
    virtual void drawVisuals(sf::RenderWindow& window) = 0;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) = 0;
    virtual void updateVisuals(float deltaTime) = 0;
    virtual void onReset() = 0;
    virtual void onStep() = 0;
    virtual void onSizeChanged(int newSize) = 0;
};

} // namespace core

#endif // VISUALIZER_BASE_HPP
