#include "core/PlaceholderVisualizerScreen.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include "core/ScreenManager.hpp"
#include "core/MainMenuScreen.hpp"
#include "ui/RoundedRectangleShape.hpp"
#include <cmath>

namespace core {

PlaceholderVisualizerScreen::PlaceholderVisualizerScreen(sf::RenderWindow& window, const std::string& moduleTitle, const std::string& description)
    : VisualizerBase(moduleTitle), m_moduleTitle(moduleTitle), m_description(description), m_pulseTime(0.f), m_stepAccumulator(0.f) {
    
    initCommonUI(window);

    // Override Back Button Callback
    m_backButton->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<MainMenuScreen>(*m_window));
    });

    const auto& colors = ThemeManager::getInstance().getColors();
    m_complexityTitle.setString("Module Status");
    m_timeComplexity.setString("Status: Under Development");
    m_timeComplexity.setFillColor(colors.warning);
    m_spaceComplexity.setString("Interactive Preview Ready");
    m_spaceComplexity.setFillColor(colors.success);

    m_theoryBody.setString(description + "\n\nNote: This visualizer module is currently under active development. Standard buttons and speed/size configurations are functional, but visualization animations are in development.");

    onReset();
}

void PlaceholderVisualizerScreen::onReset() {
    m_pulseTime = 0.f;
    m_stepAccumulator = 0.f;
    m_dataSize = static_cast<int>(m_sizeSlider->getValue());

    // Initialize dummy node positions based on type
    m_nodes.clear();
    m_nodeSpeeds.clear();

    int nodeCount = std::min(m_dataSize / 10 + 3, 10); // keep nodes bounded between 3 and 10 for display

    for (int i = 0; i < nodeCount; ++i) {
        m_nodes.push_back(sf::Vector2f(0.f, 0.f));
        m_nodeSpeeds.push_back(1.f + (rand() % 100) / 100.f);
    }
}

void PlaceholderVisualizerScreen::onStep() {
    if (m_nodes.empty()) return;
    
    // Cycle dummy node values or offset
    m_pulseTime += 1.f;
}

void PlaceholderVisualizerScreen::onSizeChanged(int newSize) {
    onReset();
}

void PlaceholderVisualizerScreen::updateVisuals(float deltaTime) {
    m_pulseTime += deltaTime;

    if (m_isPlaying) {
        m_stepAccumulator += deltaTime * m_animationSpeed;
        float stepTime = 1.0f; // 1 step per second at Speed = 1
        if (m_stepAccumulator >= stepTime) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }
}

void PlaceholderVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    
    int activeIndex = static_cast<int>(m_pulseTime * m_animationSpeed) % std::max(1, static_cast<int>(m_nodes.size()));

    // Draw background layout based on title
    if (m_moduleTitle.find("List") != std::string::npos) {
        // --- LINKED LIST REPRESENTATION ---
        float nodeW = 70.f;
        float nodeH = 45.f;
        float spacingX = 40.f;
        
        int n = static_cast<int>(m_nodes.size());
        float startX = (bounds.width - (n * nodeW + (n - 1) * spacingX)) / 2.f;
        float centerY = bounds.height / 2.f - nodeH / 2.f;

        for (int i = 0; i < n; ++i) {
            float cx = startX + i * (nodeW + spacingX);
            float cy = centerY + std::sin(m_pulseTime * m_nodeSpeeds[i]) * 10.f;

            // Draw Node Box
            ui::RoundedRectangleShape box(sf::Vector2f(nodeW, nodeH), 6.f, 6);
            box.setPosition(cx, cy);
            box.setFillColor(colors.cardBg);
            box.setOutlineThickness(2.f);
            box.setOutlineColor(i == activeIndex ? colors.success : colors.border);
            window.draw(box);

            // Draw Node Data Text
            sf::Text nodeText;
            nodeText.setFont(boldFont);
            nodeText.setString(std::to_string((i + 1) * 10));
            nodeText.setCharacterSize(14);
            nodeText.setFillColor(i == activeIndex ? colors.success : colors.textPrimary);
            
            sf::FloatRect textBounds = nodeText.getLocalBounds();
            nodeText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            nodeText.setPosition(cx + nodeW / 2.f, cy + nodeH / 2.f);
            window.draw(nodeText);

            // Draw Connecting Arrow to next node
            if (i < n - 1) {
                float ax = cx + nodeW;
                float ay = cy + nodeH / 2.f;

                sf::RectangleShape line(sf::Vector2f(spacingX - 6.f, 2.f));
                line.setPosition(ax + 2.f, ay - 1.f);
                line.setFillColor(colors.accent);
                window.draw(line);

                sf::CircleShape arrowTip(4.f, 3);
                arrowTip.setOrigin(4.f, 4.f);
                arrowTip.setRotation(90.f);
                arrowTip.setPosition(ax + spacingX - 2.f, ay);
                arrowTip.setFillColor(colors.accent);
                window.draw(arrowTip);
            }
        }

    } else if (m_moduleTitle.find("Stack") != std::string::npos) {
        // --- STACK REPRESENTATION (LIFO) ---
        float blockW = 140.f;
        float blockH = 30.f;
        float spacingY = 8.f;

        int n = static_cast<int>(m_nodes.size());
        float startX = bounds.width / 2.f - blockW / 2.f;
        float startY = bounds.height - 40.f - blockH;

        // Draw Container Base / Bracket
        sf::RectangleShape leftBorder(sf::Vector2f(4.f, (blockH + spacingY) * n + 20.f));
        leftBorder.setPosition(startX - 10.f, startY - (blockH + spacingY) * (n - 1) - 10.f);
        leftBorder.setFillColor(colors.border);
        window.draw(leftBorder);

        sf::RectangleShape rightBorder(sf::Vector2f(4.f, (blockH + spacingY) * n + 20.f));
        rightBorder.setPosition(startX + blockW + 6.f, startY - (blockH + spacingY) * (n - 1) - 10.f);
        rightBorder.setFillColor(colors.border);
        window.draw(rightBorder);

        sf::RectangleShape bottomBorder(sf::Vector2f(blockW + 20.f, 4.f));
        bottomBorder.setPosition(startX - 10.f, startY + blockH + 6.f);
        bottomBorder.setFillColor(colors.border);
        window.draw(bottomBorder);

        for (int i = 0; i < n; ++i) {
            float cy = startY - i * (blockH + spacingY);
            
            ui::RoundedRectangleShape block(sf::Vector2f(blockW, blockH), 4.f, 4);
            block.setPosition(startX, cy);
            block.setFillColor(colors.cardBg);
            block.setOutlineThickness(1.5f);
            block.setOutlineColor(i == activeIndex ? colors.success : colors.border);
            window.draw(block);

            sf::Text blockText;
            blockText.setFont(boldFont);
            blockText.setString("Element " + std::to_string(i));
            blockText.setCharacterSize(12);
            blockText.setFillColor(i == activeIndex ? colors.success : colors.textPrimary);
            
            sf::FloatRect textBounds = blockText.getLocalBounds();
            blockText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            blockText.setPosition(startX + blockW / 2.f, cy + blockH / 2.f);
            window.draw(blockText);
        }

    } else if (m_moduleTitle.find("Queue") != std::string::npos) {
        // --- QUEUE REPRESENTATION (FIFO) ---
        float blockW = 65.f;
        float blockH = 40.f;
        float spacingX = 12.f;

        int n = static_cast<int>(m_nodes.size());
        float startX = (bounds.width - (n * blockW + (n - 1) * spacingX)) / 2.f;
        float centerY = bounds.height / 2.f - blockH / 2.f;

        // Draw horizontal pipe lines
        sf::RectangleShape topPipe(sf::Vector2f(n * blockW + (n - 1) * spacingX + 40.f, 4.f));
        topPipe.setPosition(startX - 20.f, centerY - 10.f);
        topPipe.setFillColor(colors.border);
        window.draw(topPipe);

        sf::RectangleShape bottomPipe(sf::Vector2f(n * blockW + (n - 1) * spacingX + 40.f, 4.f));
        bottomPipe.setPosition(startX - 20.f, centerY + blockH + 6.f);
        bottomPipe.setFillColor(colors.border);
        window.draw(bottomPipe);

        for (int i = 0; i < n; ++i) {
            // Draw in queue order (left to right)
            float cx = startX + i * (blockW + spacingX);
            
            ui::RoundedRectangleShape block(sf::Vector2f(blockW, blockH), 4.f, 4);
            block.setPosition(cx, centerY);
            block.setFillColor(colors.cardBg);
            block.setOutlineThickness(1.5f);
            block.setOutlineColor(i == activeIndex ? colors.success : colors.border);
            window.draw(block);

            sf::Text blockText;
            blockText.setFont(boldFont);
            blockText.setString("Val " + std::to_string(i * 5));
            blockText.setCharacterSize(11);
            blockText.setFillColor(i == activeIndex ? colors.success : colors.textPrimary);
            
            sf::FloatRect textBounds = blockText.getLocalBounds();
            blockText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            blockText.setPosition(cx + blockW / 2.f, centerY + blockH / 2.f);
            window.draw(blockText);
        }

    } else if (m_moduleTitle.find("Tree") != std::string::npos || m_moduleTitle.find("Graph") != std::string::npos) {
        // --- TREE & GRAPH REPRESENTATION ---
        float radius = 22.f;
        int n = static_cast<int>(m_nodes.size());
        
        // Define coordinates statically for nodes (normalized)
        std::vector<sf::Vector2f> nodeLayouts = {
            sf::Vector2f(0.5f, 0.2f),   // Root
            sf::Vector2f(0.3f, 0.45f),  // Left
            sf::Vector2f(0.7f, 0.45f),  // Right
            sf::Vector2f(0.18f, 0.75f), // Left-Left
            sf::Vector2f(0.42f, 0.75f), // Left-Right
            sf::Vector2f(0.58f, 0.75f), // Right-Left
            sf::Vector2f(0.82f, 0.75f), // Right-Right
            sf::Vector2f(0.35f, 0.2f),  // Auxiliary
            sf::Vector2f(0.65f, 0.2f),
            sf::Vector2f(0.5f, 0.5f)
        };

        // Draw connections first (lines)
        std::vector<std::pair<int, int>> connections;
        if (m_moduleTitle.find("Tree") != std::string::npos) {
            connections = {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6}};
        } else {
            connections = {{0, 1}, {0, 2}, {1, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6}, {3, 4}, {5, 6}, {0, 9}};
        }

        for (auto& conn : connections) {
            if (conn.first < n && conn.second < n) {
                sf::Vector2f p1(nodeLayouts[conn.first].x * bounds.width, nodeLayouts[conn.first].y * bounds.height);
                sf::Vector2f p2(nodeLayouts[conn.second].x * bounds.width, nodeLayouts[conn.second].y * bounds.height);

                sf::Vertex line[] = {
                    sf::Vertex(p1, colors.border),
                    sf::Vertex(p2, colors.border)
                };
                window.draw(line, 2, sf::Lines);
            }
        }

        // Draw circles
        for (int i = 0; i < n; ++i) {
            float cx = nodeLayouts[i].x * bounds.width;
            float cy = nodeLayouts[i].y * bounds.height;
            float floatOffset = std::sin(m_pulseTime * m_nodeSpeeds[i]) * 4.f;

            sf::CircleShape nodeCircle(radius);
            nodeCircle.setOrigin(radius, radius);
            nodeCircle.setPosition(cx, cy + floatOffset);
            nodeCircle.setFillColor(colors.cardBg);
            nodeCircle.setOutlineThickness(2.f);
            nodeCircle.setOutlineColor(i == activeIndex ? colors.success : colors.accent);
            window.draw(nodeCircle);

            sf::Text nodeText;
            nodeText.setFont(boldFont);
            nodeText.setString(std::to_string(i));
            nodeText.setCharacterSize(14);
            nodeText.setFillColor(i == activeIndex ? colors.success : colors.textPrimary);
            
            sf::FloatRect textBounds = nodeText.getLocalBounds();
            nodeText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            nodeText.setPosition(cx, cy + floatOffset);
            window.draw(nodeText);
        }

    } else {
        // --- GENERAL STYLING (GRID OF PULSING BARS) ---
        int barCount = std::min(m_dataSize, 25);
        float padding = 4.f;
        float barWidth = bounds.width / barCount;
        float maxHeight = bounds.height - 50.f;

        for (int i = 0; i < barCount; ++i) {
            float pulseHeight = (0.3f + 0.6f * std::abs(std::sin(m_pulseTime * 0.8f + i * 0.2f))) * maxHeight;
            
            sf::RectangleShape bar(sf::Vector2f(barWidth - padding, pulseHeight));
            bar.setPosition(i * barWidth + padding / 2.f, bounds.height - pulseHeight - 10.f);
            
            if (i == activeIndex) {
                bar.setFillColor(colors.barActive);
            } else if (i < activeIndex) {
                bar.setFillColor(colors.barSorted);
            } else {
                bar.setFillColor(colors.barDefault);
            }

            window.draw(bar);
        }
    }
}

void PlaceholderVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    // Event overrides
}

} // namespace core
