#include "core/SearchingVisualizerScreen.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include "core/ScreenManager.hpp"
#include "core/MainMenuScreen.hpp"
#include "ui/RoundedRectangleShape.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstdio>

namespace core {

SearchingVisualizerScreen::SearchingVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("Searching Visualizer"),
      m_currentAlgorithm(SearchingAlgorithm::Linear),
      m_targetValue(50),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_currentIdx(-1),
      m_lowIdx(-1),
      m_highIdx(-1),
      m_midIdx(-1),
      m_foundIdx(-1),
      m_isInputActive(false),
      m_cursorBlinkTime(0.f),
      m_currentComparisons(0),
      m_totalComparisons(0),
      m_elapsedTime(0.f),
      m_showCompletion(false) {
    
    srand(static_cast<unsigned int>(time(nullptr)));
    initCommonUI(window);
    initSearchingUI();

    m_dataSize = static_cast<int>(m_sizeSlider->getValue());
    generateRandomArray();
    loadAlgorithm(SearchingAlgorithm::Linear);
}

void SearchingVisualizerScreen::initSearchingUI() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // Explanation text
    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(12);
    m_explanationText.setFillColor(colors.textSecondary);
    m_explanationText.setString("Initialize search target to begin.");

    // 1. Selector buttons (Linear, Binary)
    std::vector<std::string> names = {"Linear Search", "Binary Search"};
    for (size_t i = 0; i < names.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(names[i], sf::Vector2f(120.f, 26.f), 11);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        SearchingAlgorithm algo = static_cast<SearchingAlgorithm>(i);
        btn->setCallback([this, algo]() {
            loadAlgorithm(algo);
        });
        m_selectorButtons.push_back(std::move(btn));
    }

    // 2. Target Adjuster Buttons
    m_decTargetBtn = std::make_unique<ui::Button>("-", sf::Vector2f(24.f, 24.f), 12);
    m_decTargetBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_decTargetBtn->setCallback([this]() { adjustTarget(-1); });

    m_incTargetBtn = std::make_unique<ui::Button>("+", sf::Vector2f(24.f, 24.f), 12);
    m_incTargetBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_incTargetBtn->setCallback([this]() { adjustTarget(1); });

    m_randTargetBtn = std::make_unique<ui::Button>("Random Target", sf::Vector2f(110.f, 24.f), 10);
    m_randTargetBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_randTargetBtn->setCallback([this]() { generateRandomTarget(); });

    m_pickExistBtn = std::make_unique<ui::Button>("Pick Existing", sf::Vector2f(110.f, 24.f), 10);
    m_pickExistBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_pickExistBtn->setCallback([this]() { pickExistingTarget(); });

    m_inputFieldFrame = std::make_unique<ui::Button>("Target: 50", sf::Vector2f(100.f, 24.f), 10);
    m_inputFieldFrame->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    // Manual click callback sets input focus
    m_inputFieldFrame->setCallback([this]() {
        m_isInputActive = true;
        m_inputTextBuffer = "";
    });

    m_targetLabelText.setFont(regularFont);
    m_targetLabelText.setCharacterSize(11);
    m_targetLabelText.setFillColor(colors.textPrimary);

    // 3. Array Generators
    std::vector<std::string> genNames = {"Random", "Sorted", "Shuffle", "Reverse", "Nearly Sorted"};
    for (size_t i = 0; i < genNames.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(genNames[i], sf::Vector2f(85.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        if (i == 0) {
            btn->setCallback([this]() { generateRandomArray(); });
        } else if (i == 1) {
            btn->setCallback([this]() { generateSortedArray(); });
        } else if (i == 2) {
            btn->setCallback([this]() { shuffleArray(); });
        } else if (i == 3) {
            btn->setCallback([this]() { generateReverseSortedArray(); });
        } else {
            btn->setCallback([this]() { generateNearlySortedArray(); });
        }
        m_genButtons.push_back(std::move(btn));
    }

    // 4. Live Statistics labels (in Info Panel)
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("LIVE STATISTICS");
    m_statsTitleText.setCharacterSize(14);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsComparisonsText.setFont(regularFont);
    m_statsComparisonsText.setCharacterSize(12);
    m_statsComparisonsText.setFillColor(colors.textPrimary);

    m_statsStepText.setFont(regularFont);
    m_statsStepText.setCharacterSize(12);
    m_statsStepText.setFillColor(colors.textPrimary);

    m_statsSizeText.setFont(regularFont);
    m_statsSizeText.setCharacterSize(12);
    m_statsSizeText.setFillColor(colors.textPrimary);

    m_statsTimeText.setFont(regularFont);
    m_statsTimeText.setCharacterSize(12);
    m_statsTimeText.setFillColor(colors.textPrimary);

    m_statsRangeText.setFont(regularFont);
    m_statsRangeText.setCharacterSize(12);
    m_statsRangeText.setFillColor(colors.textPrimary);

    // 5. Completion Overlay
    m_completionOverlay = std::make_unique<ui::Panel>(sf::Vector2f(340.f, 270.f), "Search Finished");
    m_completionOverlay->setBackgroundColor(sf::Color(26, 26, 36, 240));
    m_completionOverlay->setBorderThickness(2.f);

    m_completionTitle.setFont(boldFont);
    m_completionTitle.setString("Search Completed");
    m_completionTitle.setCharacterSize(18);

    m_completionStatsText.setFont(regularFont);
    m_completionStatsText.setCharacterSize(12);
    m_completionStatsText.setFillColor(colors.textPrimary);

    m_completionResetBtn = std::make_unique<ui::Button>("Search Again", sf::Vector2f(110.f, 32.f), 12);
    m_completionResetBtn->setColors(colors.success, colors.accentHover, colors.textPrimary);
    m_completionResetBtn->setCallback([this]() { onReset(); });
}

void SearchingVisualizerScreen::updateSearchingLayout() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    // Selector buttons (row 1)
    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    for (size_t i = 0; i < m_selectorButtons.size(); ++i) {
        m_selectorButtons[i]->setPosition(sf::Vector2f(startX + i * (120.f + spacing), startY));
    }

    // Target controls (row 2)
    float targetY = startY + 36.f;
    m_decTargetBtn->setPosition(sf::Vector2f(startX, targetY));
    m_inputFieldFrame->setPosition(sf::Vector2f(startX + 24.f + spacing, targetY));
    m_incTargetBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 100.f + spacing, targetY));
    m_randTargetBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 100.f + spacing + 24.f + spacing, targetY));
    m_pickExistBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 100.f + spacing + 24.f + spacing + 110.f + spacing, targetY));

    // Generators (row 3)
    float genY = targetY + 34.f;
    for (size_t i = 0; i < m_genButtons.size(); ++i) {
        m_genButtons[i]->setPosition(sf::Vector2f(startX + i * (85.f + spacing), genY));
    }

    // Explanation Y
    m_explanationText.setPosition(bounds.left + 15.f, genY + 33.f);

    // Live Stats right column layout
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 75.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsComparisonsText.setPosition(infoBounds.left, statsStartY + 22.f);
    m_statsStepText.setPosition(infoBounds.left, statsStartY + 40.f);
    m_statsSizeText.setPosition(infoBounds.left, statsStartY + 58.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 76.f);
    m_statsRangeText.setPosition(infoBounds.left, statsStartY + 94.f);

    // Dynamic complexities push-downs
    m_theoryTitle.setPosition(infoBounds.left, statsStartY + 130.f);
    m_theoryBody.setPosition(infoBounds.left, statsStartY + 155.f);

    // Stacking pseudocode text rows
    float pseudocodeStartY = statsStartY + 270.f;
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        m_pseudocodeTexts[i].setPosition(infoBounds.left + 10.f, pseudocodeStartY + i * 16.f);
    }

    // Centering completion popup overlay
    float ox = bounds.left + (bounds.width - 340.f) / 2.f;
    float oy = bounds.top + (bounds.height - 270.f) / 2.f;
    m_completionOverlay->setPosition(sf::Vector2f(ox, oy));

    sf::FloatRect obounds = m_completionOverlay->getContentBounds();
    m_completionTitle.setPosition(obounds.left, obounds.top + 5.f);
    m_completionStatsText.setPosition(obounds.left, obounds.top + 35.f);
    m_completionResetBtn->setPosition(sf::Vector2f(obounds.left + (obounds.width - 110.f) / 2.f, obounds.top + obounds.height - 40.f));
}

void SearchingVisualizerScreen::loadAlgorithm(SearchingAlgorithm algo) {
    m_currentAlgorithm = algo;
    m_isPlaying = false;
    m_playPauseButton->setLabel("Play");
    
    const auto& colors = ThemeManager::getInstance().getColors();
    m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);

    // Tabs update
    for (size_t i = 0; i < m_selectorButtons.size(); ++i) {
        if (i == static_cast<size_t>(algo)) {
            m_selectorButtons[i]->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        } else {
            m_selectorButtons[i]->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        }
    }

    // Theory complex details
    switch (algo) {
        case SearchingAlgorithm::Linear:
            m_complexityTitle.setString("Linear Search");
            m_timeComplexity.setString("Time Complexity: O(N)");
            m_spaceComplexity.setString("Space Complexity: O(1)");
            m_theoryBody.setString("Linear Search (Sequential Search) scans the array elements sequentially from index 0 to N-1 until target value is found or array exhausts. Works on unsorted data.");
            break;
        case SearchingAlgorithm::Binary:
            m_complexityTitle.setString("Binary Search");
            m_timeComplexity.setString("Time Complexity: O(log N)");
            m_spaceComplexity.setString("Space Complexity: O(1)");
            m_theoryBody.setString("Binary Search halving search space dynamically. Requires a sorted array. It compares target with middle element and discards half of the partition on each mismatch.");
            break;
    }

    loadPseudocode(algo);

    // If Binary Search, automatically sort before resetting
    if (algo == SearchingAlgorithm::Binary) {
        std::sort(m_originalArray.begin(), m_originalArray.end());
        m_currentArray = m_originalArray;
    }

    onReset();

    if (algo == SearchingAlgorithm::Binary) {
        m_explanationText.setString("Binary Search requires a sorted array. The array has been automatically sorted.");
    }
}

void SearchingVisualizerScreen::loadPseudocode(SearchingAlgorithm algo) {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    switch (algo) {
        case SearchingAlgorithm::Linear:
            m_pseudocodeLines = {
                "linearSearch(Array A, target):",
                "  for i from 0 to A.length - 1:",
                "    if A[i] == target:",
                "      return i",
                "  return -1"
            };
            break;
        case SearchingAlgorithm::Binary:
            m_pseudocodeLines = {
                "binarySearch(Array A, target):",
                "  low = 0, high = A.length - 1",
                "  while low <= high:",
                "    mid = low + (high - low) / 2",
                "    if A[mid] == target: return mid",
                "    else if A[mid] < target:",
                "      low = mid + 1",
                "    else:",
                "      high = mid - 1",
                "  return -1"
            };
            break;
    }

    sf::Font& monoFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");
    const auto& colors = ThemeManager::getInstance().getColors();

    for (const auto& line : m_pseudocodeLines) {
        sf::Text text;
        text.setFont(monoFont);
        text.setString(line);
        text.setCharacterSize(10);
        text.setFillColor(colors.textSecondary);
        m_pseudocodeTexts.push_back(text);
    }

    m_activePseudocodeLine = -1;
    updateSearchingLayout();
}

// -------------------------------------------------------------
// DYNAMIC GENERATION ACTIONS
// -------------------------------------------------------------
void SearchingVisualizerScreen::generateRandomArray() {
    m_originalArray.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        m_originalArray.push_back(5 + rand() % 95);
    }
    if (m_currentAlgorithm == SearchingAlgorithm::Binary) {
        std::sort(m_originalArray.begin(), m_originalArray.end());
    }
    onReset();
}

void SearchingVisualizerScreen::generateSortedArray() {
    m_originalArray.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        float t = static_cast<float>(i) / std::max(1, m_dataSize - 1);
        m_originalArray.push_back(static_cast<int>(5 + t * 90));
    }
    onReset();
}

void SearchingVisualizerScreen::shuffleArray() {
    if (m_currentAlgorithm == SearchingAlgorithm::Binary) {
        m_explanationText.setString("Cannot shuffle! Binary Search requires array to remain sorted.");
        return;
    }
    std::random_shuffle(m_originalArray.begin(), m_originalArray.end());
    onReset();
}

void SearchingVisualizerScreen::generateReverseSortedArray() {
    m_originalArray.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        float t = static_cast<float>(m_dataSize - 1 - i) / std::max(1, m_dataSize - 1);
        m_originalArray.push_back(static_cast<int>(5 + t * 90));
    }
    if (m_currentAlgorithm == SearchingAlgorithm::Binary) {
        std::sort(m_originalArray.begin(), m_originalArray.end());
    }
    onReset();
}

void SearchingVisualizerScreen::generateNearlySortedArray() {
    m_originalArray.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        float t = static_cast<float>(i) / std::max(1, m_dataSize - 1);
        m_originalArray.push_back(static_cast<int>(5 + t * 90));
    }
    if (m_currentAlgorithm == SearchingAlgorithm::Binary) {
        // Binary search requires fully sorted
        onReset();
        return;
    }
    int numSwaps = std::max(1, m_dataSize / 20);
    for (int s = 0; s < numSwaps; ++s) {
        int idx1 = rand() % m_dataSize;
        int idx2 = std::min(m_dataSize - 1, idx1 + 1 + rand() % 3);
        std::swap(m_originalArray[idx1], m_originalArray[idx2]);
    }
    onReset();
}

// -------------------------------------------------------------
// TARGET ADJUSTERS
// -------------------------------------------------------------
void SearchingVisualizerScreen::adjustTarget(int delta) {
    m_targetValue += delta;
    if (m_targetValue < 1) m_targetValue = 1;
    if (m_targetValue > 100) m_targetValue = 100;
    onReset();
}

void SearchingVisualizerScreen::generateRandomTarget() {
    m_targetValue = 5 + rand() % 95;
    onReset();
}

void SearchingVisualizerScreen::pickExistingTarget() {
    if (m_originalArray.empty()) return;
    int idx = rand() % m_originalArray.size();
    m_targetValue = m_originalArray[idx];
    onReset();
}

void SearchingVisualizerScreen::commitTargetInput() {
    m_isInputActive = false;
    if (!m_inputTextBuffer.empty()) {
        int val = std::atoi(m_inputTextBuffer.c_str());
        if (val >= 1 && val <= 100) {
            m_targetValue = val;
        } else {
            std::cout << "Invalid input range [1, 100]" << std::endl;
        }
    }
    onReset();
}

// -------------------------------------------------------------
// CONTROL LOOPS & RECORDERS
// -------------------------------------------------------------
void SearchingVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_currentIdx = -1;
    m_lowIdx = -1;
    m_highIdx = -1;
    m_midIdx = -1;
    m_foundIdx = -1;
    
    m_isVisited.clear();
    m_isVisited.assign(m_originalArray.size(), false);
    
    m_currentArray = m_originalArray;

    m_currentComparisons = 0;
    m_elapsedTime = 0.f;
    m_showCompletion = false;

    runSearchingRecord();

    // Compute totals
    m_totalComparisons = 0;
    for (const auto& step : m_steps) {
        if (step.type == SearchStep::Type::Compare) m_totalComparisons++;
    }

    m_explanationText.setString("Ready. Search Target: " + std::to_string(m_targetValue));
    m_activePseudocodeLine = -1;

    if (m_window) {
        updateSearchingLayout();
    }
}

void SearchingVisualizerScreen::onSizeChanged(int newSize) {
    m_dataSize = newSize;
    generateRandomArray();
}

void SearchingVisualizerScreen::runSearchingRecord() {
    if (m_currentAlgorithm == SearchingAlgorithm::Linear) {
        recordLinearSearch();
    } else {
        recordBinarySearch();
    }
}

void SearchingVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        
        m_showCompletion = true;
        m_currentIdx = -1;
        m_lowIdx = -1;
        m_highIdx = -1;
        m_midIdx = -1;
        m_activePseudocodeLine = -1;

        std::string algoName = m_currentAlgorithm == SearchingAlgorithm::Linear ? "Linear Search" : "Binary Search";
        std::string resStr = m_foundIdx != -1 ? "FOUND" : "NOT FOUND";
        std::string indexStr = m_foundIdx != -1 ? std::to_string(m_foundIdx) : "N/A";
        
        std::string timeComplexity = m_currentAlgorithm == SearchingAlgorithm::Linear ? "O(N)" : "O(log N)";
        std::string spaceComplexity = "O(1)";

        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), 
                     "Algorithm: %s\nTarget: %d\nResult: %s\nFound Index: %s\nComparisons: %d\nVisualization Time: %.2fs\nTime Complexity: %s\nSpace Complexity: %s", 
                     algoName.c_str(), m_targetValue, resStr.c_str(), indexStr.c_str(),
                     m_totalComparisons, m_elapsedTime, timeComplexity.c_str(), spaceComplexity.c_str());
        m_completionStatsText.setString(buffer);
        m_explanationText.setString("Search finished. Result: " + resStr);
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);

    switch (step.type) {
        case SearchStep::Type::Compare:
            m_currentComparisons++;
            m_currentIdx = step.index1;
            break;
        case SearchStep::Type::SetPointers:
            m_lowIdx = step.index1;
            m_midIdx = step.value;
            m_highIdx = step.index2;
            m_currentIdx = -1;
            break;
        case SearchStep::Type::DiscardSection:
            // Set pointers visually representing discard
            m_lowIdx = step.index1;
            m_highIdx = step.index2;
            m_midIdx = -1;
            m_currentIdx = -1;
            break;
        case SearchStep::Type::Explain:
            break;
        case SearchStep::Type::HighlightLine:
            break;
        case SearchStep::Type::Found:
            m_foundIdx = step.index1;
            m_lowIdx = -1;
            m_highIdx = -1;
            m_midIdx = -1;
            m_currentIdx = -1;
            break;
        case SearchStep::Type::NotFound:
            m_foundIdx = -1;
            m_lowIdx = -1;
            m_highIdx = -1;
            m_midIdx = -1;
            m_currentIdx = -1;
            break;
    }

    // Track visited in linear search
    if (m_currentAlgorithm == SearchingAlgorithm::Linear && m_currentIdx != -1) {
        for (int i = 0; i <= m_currentIdx; ++i) {
            m_isVisited[i] = true;
        }
    }

    m_currentStepIndex++;
}

void SearchingVisualizerScreen::updateVisuals(float deltaTime) {
    if (m_window) {
        if (m_showCompletion) {
            m_completionResetBtn->update(deltaTime, *m_window);
        } else {
            for (auto& btn : m_selectorButtons) btn->update(deltaTime, *m_window);
            for (auto& btn : m_genButtons) btn->update(deltaTime, *m_window);
            m_decTargetBtn->update(deltaTime, *m_window);
            m_incTargetBtn->update(deltaTime, *m_window);
            m_randTargetBtn->update(deltaTime, *m_window);
            m_pickExistBtn->update(deltaTime, *m_window);
            m_inputFieldFrame->update(deltaTime, *m_window);
        }

        if (m_isInputActive) {
            m_cursorBlinkTime += deltaTime;
        }
    }

    if (m_isPlaying && !m_showCompletion) {
        m_elapsedTime += deltaTime;
        m_stepAccumulator += deltaTime * m_animationSpeed;

        float stepDelay = 0.15f; // slower delay to perceive values clearly
        if (m_stepAccumulator >= stepDelay) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }

    // Refresh live stats text labels
    m_statsComparisonsText.setString("Comparisons: " + std::to_string(m_currentComparisons) + " / " + std::to_string(m_totalComparisons));
    m_statsStepText.setString("Step: " + std::to_string(m_currentStepIndex) + " / " + std::to_string(m_steps.size()));
    m_statsSizeText.setString("Array Size: " + std::to_string(m_dataSize));

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Elapsed Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);

    if (m_currentAlgorithm == SearchingAlgorithm::Binary) {
        std::string lowStr = m_lowIdx != -1 ? std::to_string(m_lowIdx) : "N/A";
        std::string highStr = m_highIdx != -1 ? std::to_string(m_highIdx) : "N/A";
        m_statsRangeText.setString("Search Bounds: [" + lowStr + ", " + highStr + "]");
    } else {
        m_statsRangeText.setString("Search Bounds: N/A");
    }

    // Sync input frame button text
    if (m_isInputActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_inputFieldFrame->setLabel(displayStr);
    } else {
        m_inputFieldFrame->setLabel("Target: " + std::to_string(m_targetValue));
    }
}

void SearchingVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    if (m_showCompletion) {
        m_completionResetBtn->handleEvent(event, window);
        return;
    }

    // Text buffering for Target Input box
    if (m_isInputActive && event.type == sf::Event::TextEntered) {
        sf::Uint32 code = event.text.unicode;
        if (code == 8) { // Backspace
            if (!m_inputTextBuffer.empty()) m_inputTextBuffer.pop_back();
        } else if (code == 13) { // Enter
            commitTargetInput();
        } else if (code >= '0' && code <= '9') {
            if (m_inputTextBuffer.size() < 3) {
                m_inputTextBuffer += static_cast<char>(code);
            }
        }
        return;
    }

    // Dismiss focus if click outside frame
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f bounds = m_inputFieldFrame->getSize(); // placeholder bounds
        // Actual button bounds checking
        sf::Vector2f pos = m_inputFieldFrame->getPosition();
        sf::FloatRect abounds(pos.x, pos.y, bounds.x, bounds.y);
        
        if (!abounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if (m_isInputActive) {
                commitTargetInput();
            }
        }
    }

    for (auto& btn : m_selectorButtons) btn->handleEvent(event, window);
    for (auto& btn : m_genButtons) btn->handleEvent(event, window);
    m_decTargetBtn->handleEvent(event, window);
    m_incTargetBtn->handleEvent(event, window);
    m_randTargetBtn->handleEvent(event, window);
    m_pickExistBtn->handleEvent(event, window);
    m_inputFieldFrame->handleEvent(event, window);
}

void SearchingVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();

    // 1. Draw UI controls
    for (auto& btn : m_selectorButtons) window.draw(*btn);
    for (auto& btn : m_genButtons) window.draw(*btn);
    window.draw(*m_decTargetBtn);
    window.draw(*m_incTargetBtn);
    window.draw(*m_randTargetBtn);
    window.draw(*m_pickExistBtn);
    window.draw(*m_inputFieldFrame);
    window.draw(m_explanationText);

    // 2. Draw Array Bars
    int count = m_currentArray.size();
    if (count > 0) {
        float drawAreaH = bounds.height - 155.f;
        float padding = 1.5f;
        float barWidth = bounds.width / count;
        float barStartY = bounds.top + 140.f;

        for (int i = 0; i < count; ++i) {
            float val = m_currentArray[i] / 100.f;
            float barHeight = val * drawAreaH;

            sf::RectangleShape bar(sf::Vector2f(std::max(0.5f, barWidth - padding), barHeight));
            bar.setPosition(bounds.left + i * barWidth + padding / 2.f, bounds.top + bounds.height - barHeight - 10.f);

            // Establish color styling
            sf::Color barColor = colors.barDefault;
            float opacityFactor = 1.f;

            if (m_currentAlgorithm == SearchingAlgorithm::Linear) {
                if (i == m_foundIdx) {
                    barColor = colors.success;
                } else if (i == m_currentIdx) {
                    barColor = colors.barActive;
                } else if (m_isVisited[i]) {
                    barColor = colors.border; // visited elements
                    opacityFactor = 0.5f;
                }
            } else {
                // Binary Search Colors
                if (i == m_foundIdx) {
                    barColor = colors.success;
                } else if (i == m_midIdx) {
                    barColor = colors.danger; // mid
                } else if (i == m_lowIdx || i == m_highIdx) {
                    barColor = colors.warning; // low/high pointers
                } else if (m_lowIdx != -1 && m_highIdx != -1 && (i < m_lowIdx || i > m_highIdx)) {
                    opacityFactor = 0.2f; // faded discarded range
                }
            }

            sf::Color fill = barColor;
            fill.a = static_cast<sf::Uint8>(fill.a * opacityFactor);
            bar.setFillColor(fill);
            window.draw(bar);
        }
    }

    // 3. Draw statistics in Info panel
    window.draw(m_statsTitleText);
    window.draw(m_statsComparisonsText);
    window.draw(m_statsStepText);
    window.draw(m_statsSizeText);
    window.draw(m_statsTimeText);
    window.draw(m_statsRangeText);

    // 4. Render Pseudocode with highlights
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        if (static_cast<int>(i) == m_activePseudocodeLine) {
            m_pseudocodeTexts[i].setFillColor(colors.accent);
            m_pseudocodeTexts[i].setStyle(sf::Text::Bold);
        } else {
            m_pseudocodeTexts[i].setFillColor(colors.textSecondary);
            m_pseudocodeTexts[i].setStyle(sf::Text::Regular);
        }
        window.draw(m_pseudocodeTexts[i]);
    }

    // 5. Draw overlay
    if (m_showCompletion) {
        // Adapt completion popup color to found/not found case
        if (m_foundIdx != -1) {
            m_completionOverlay->setBorderColor(colors.success);
            m_completionTitle.setFillColor(colors.success);
            m_completionTitle.setString("Target Found!");
        } else {
            m_completionOverlay->setBorderColor(colors.danger);
            m_completionTitle.setFillColor(colors.danger);
            m_completionTitle.setString("Target Not Found");
        }
        window.draw(*m_completionOverlay);
        window.draw(m_completionTitle);
        window.draw(m_completionStatsText);
        window.draw(*m_completionResetBtn);
    }
}

// -------------------------------------------------------------
// RECORDER ACCUMULATORS
// -------------------------------------------------------------
void SearchingVisualizerScreen::addCompareStep(int idx, int line, const std::string& explanation) {
    SearchStep step;
    step.type = SearchStep::Type::Compare;
    step.index1 = idx;
    step.index2 = -1;
    step.value = 0;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SearchingVisualizerScreen::addSetPointersStep(int low, int mid, int high, int line, const std::string& explanation) {
    SearchStep step;
    step.type = SearchStep::Type::SetPointers;
    step.index1 = low;
    step.index2 = high;
    step.value = mid;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SearchingVisualizerScreen::addDiscardStep(int idx1, int idx2, int line, const std::string& explanation) {
    SearchStep step;
    step.type = SearchStep::Type::DiscardSection;
    step.index1 = idx1;
    step.index2 = idx2;
    step.value = 0;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SearchingVisualizerScreen::addHighlightStep(int line) {
    SearchStep step;
    step.type = SearchStep::Type::HighlightLine;
    step.index1 = -1;
    step.index2 = -1;
    step.value = 0;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    m_steps.push_back(step);
}

void SearchingVisualizerScreen::addExplainStep(const std::string& explanation) {
    SearchStep step;
    step.type = SearchStep::Type::Explain;
    step.index1 = -1;
    step.index2 = -1;
    step.value = 0;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SearchingVisualizerScreen::addFoundStep(int idx, int line, const std::string& explanation) {
    SearchStep step;
    step.type = SearchStep::Type::Found;
    step.index1 = idx;
    step.index2 = -1;
    step.value = 0;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SearchingVisualizerScreen::addNotFoundStep(int line, const std::string& explanation) {
    SearchStep step;
    step.type = SearchStep::Type::NotFound;
    step.index1 = -1;
    step.index2 = -1;
    step.value = 0;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

// -------------------------------------------------------------
// RECORDER IMPLEMENTATIONS
// -------------------------------------------------------------
void SearchingVisualizerScreen::recordLinearSearch() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();

    addExplainStep("Initializing Linear Search. Scanning elements sequentially.");
    addHighlightStep(0);

    for (int i = 0; i < n; ++i) {
        addHighlightStep(1); // for i from 0 to A.length - 1:
        addCompareStep(i, 2, "Checking element at index " + std::to_string(i) + ": A[i] = " + std::to_string(arr[i]) + ". Comparing with target " + std::to_string(m_targetValue));
        
        if (arr[i] == m_targetValue) {
            addFoundStep(i, 3, "Target element " + std::to_string(m_targetValue) + " matches! Returning index " + std::to_string(i));
            return;
        }
    }
    
    addNotFoundStep(4, "Target element " + std::to_string(m_targetValue) + " not found. Returning -1");
}

void SearchingVisualizerScreen::recordBinarySearch() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();

    addExplainStep("Initializing Binary Search bounds.");
    int low = 0;
    int high = n - 1;
    addSetPointersStep(low, -1, high, 1, "Setting low = 0 and high = " + std::to_string(high));

    while (low <= high) {
        addHighlightStep(2); // while low <= high:
        int mid = low + (high - low) / 2;
        addSetPointersStep(low, mid, high, 3, "Computing mid index: low + (high - low)/2 = " + std::to_string(mid));

        addCompareStep(mid, 4, "Comparing middle element A[mid] = " + std::to_string(arr[mid]) + " with target " + std::to_string(m_targetValue));
        
        if (arr[mid] == m_targetValue) {
            addFoundStep(mid, 4, "Target matches middle element! Returning index " + std::to_string(mid));
            return;
        }
        else if (arr[mid] < m_targetValue) {
            addHighlightStep(5);
            addDiscardStep(mid + 1, high, 6, "A[mid] (" + std::to_string(arr[mid]) + ") < target. Discarding left space. Setting low = mid + 1 (" + std::to_string(mid + 1) + ")");
            low = mid + 1;
        }
        else {
            addHighlightStep(7);
            addDiscardStep(low, mid - 1, 8, "A[mid] (" + std::to_string(arr[mid]) + ") > target. Discarding right space. Setting high = mid - 1 (" + std::to_string(mid - 1) + ")");
            high = mid - 1;
        }
    }

    addNotFoundStep(9, "Search space exhausted (low > high). Target " + std::to_string(m_targetValue) + " not found in array. Returning -1");
}

} // namespace core
