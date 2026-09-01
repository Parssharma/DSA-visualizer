#ifndef TEST_VISUALIZER_SCREEN_HPP
#define TEST_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <vector>

namespace core {

class TestVisualizerScreen : public VisualizerBase {
public:
    explicit TestVisualizerScreen(sf::RenderWindow& window);
    virtual ~TestVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    std::vector<int> m_array;
    float m_stepAccumulator;
};

} // namespace core

#endif // TEST_VISUALIZER_SCREEN_HPP
