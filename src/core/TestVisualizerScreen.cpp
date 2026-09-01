#include "core/TestVisualizerScreen.hpp"
#include "core/ThemeManager.hpp"
#include <cstdlib>
#include <ctime>

namespace core {

TestVisualizerScreen::TestVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("DSA Visualizer - Architecture Demo"), m_stepAccumulator(0.f) {
    srand(static_cast<unsigned int>(time(nullptr)));
    initCommonUI(window);
    onReset();
}

void TestVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    if (m_array.empty()) return;

    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    float padding = 2.f;
    float totalWidth = bounds.width;
    float barWidth = totalWidth / m_array.size();
    float maxHeight = bounds.height - 40.f;

    const auto& colors = ThemeManager::getInstance().getColors();

    for (size_t i = 0; i < m_array.size(); ++i) {
        float val = m_array[i] / 100.f;
        float barHeight = val * maxHeight;
        
        // Draw the bar relative to the panel content origin
        sf::RectangleShape bar(sf::Vector2f(barWidth - padding, barHeight));
        // Draw bottom-aligned inside panel content bounds
        bar.setPosition(i * barWidth + padding / 2.f, bounds.height - barHeight - 10.f);
        bar.setFillColor(colors.barDefault);
        
        // Highlight active one
        if (i == m_array.size() / 2) {
            bar.setFillColor(colors.barActive);
        }
        
        window.draw(bar);
    }
}

void TestVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    // No specific events for test screen
}

void TestVisualizerScreen::updateVisuals(float deltaTime) {
    if (m_isPlaying) {
        m_stepAccumulator += deltaTime * m_animationSpeed;
        // Animation step frequency scales with slider multiplier
        float stepTime = 0.5f; 
        if (m_stepAccumulator >= stepTime) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }
}

void TestVisualizerScreen::onReset() {
    m_array.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        m_array.push_back(10 + rand() % 90);
    }
}

void TestVisualizerScreen::onStep() {
    if (m_array.empty()) return;
    // Perform a cyclic shift
    int first = m_array[0];
    for (size_t i = 0; i < m_array.size() - 1; ++i) {
        m_array[i] = m_array[i+1];
    }
    m_array[m_array.size() - 1] = first;
}

void TestVisualizerScreen::onSizeChanged(int newSize) {
    onReset();
}

} // namespace core
