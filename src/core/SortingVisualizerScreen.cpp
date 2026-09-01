#include "core/SortingVisualizerScreen.hpp"
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

SortingVisualizerScreen::SortingVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("Sorting Visualizer"),
      m_currentAlgorithm(SortingAlgorithm::Bubble),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_currentComparisons(0),
      m_currentSwaps(0),
      m_totalComparisons(0),
      m_totalSwaps(0),
      m_elapsedTime(0.f),
      m_showCompletion(false) {
    
    srand(static_cast<unsigned int>(time(nullptr)));
    initCommonUI(window);
    initSortingUI();
    
    // Set default initial data size
    m_dataSize = static_cast<int>(m_sizeSlider->getValue());
    generateRandomArray();
    loadAlgorithm(SortingAlgorithm::Bubble);
}

void SortingVisualizerScreen::initSortingUI() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // Initialize explanation text
    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(12);
    m_explanationText.setFillColor(colors.textSecondary);
    m_explanationText.setString("Initialize sorting array to begin.");

    // 1. Initialize algorithm selectors
    std::vector<std::string> names = {"Bubble", "Selection", "Insertion", "Merge", "Quick", "Heap"};
    for (size_t i = 0; i < names.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(names[i], sf::Vector2f(88.f, 26.f), 11);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        SortingAlgorithm algo = static_cast<SortingAlgorithm>(i);
        btn->setCallback([this, algo]() {
            loadAlgorithm(algo);
        });
        m_selectorButtons.push_back(std::move(btn));
    }

    // 2. Initialize array generator buttons
    std::vector<std::string> genNames = {"Random", "Shuffle", "Reverse", "Nearly Sorted"};
    for (size_t i = 0; i < genNames.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(genNames[i], sf::Vector2f(85.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        if (i == 0) {
            btn->setCallback([this]() { generateRandomArray(); });
        } else if (i == 1) {
            btn->setCallback([this]() { shuffleArray(); });
        } else if (i == 2) {
            btn->setCallback([this]() { generateReverseSortedArray(); });
        } else {
            btn->setCallback([this]() { generateNearlySortedArray(); });
        }
        m_genButtons.push_back(std::move(btn));
    }

    // 3. Initialize Live Statistics text blocks
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("LIVE STATISTICS");
    m_statsTitleText.setCharacterSize(14);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsComparisonsText.setFont(regularFont);
    m_statsComparisonsText.setCharacterSize(12);
    m_statsComparisonsText.setFillColor(colors.textPrimary);

    m_statsSwapsText.setFont(regularFont);
    m_statsSwapsText.setCharacterSize(12);
    m_statsSwapsText.setFillColor(colors.textPrimary);

    m_statsStepText.setFont(regularFont);
    m_statsStepText.setCharacterSize(12);
    m_statsStepText.setFillColor(colors.textPrimary);

    m_statsSizeText.setFont(regularFont);
    m_statsSizeText.setCharacterSize(12);
    m_statsSizeText.setFillColor(colors.textPrimary);

    m_statsTimeText.setFont(regularFont);
    m_statsTimeText.setCharacterSize(12);
    m_statsTimeText.setFillColor(colors.textPrimary);

    // 4. Initialize Completion Overlay Card
    m_completionOverlay = std::make_unique<ui::Panel>(sf::Vector2f(340.f, 270.f), "Sorting Completed!");
    m_completionOverlay->setBackgroundColor(sf::Color(26, 26, 36, 240));
    m_completionOverlay->setBorderColor(colors.success);
    m_completionOverlay->setBorderThickness(2.f);

    m_completionTitle.setFont(boldFont);
    m_completionTitle.setString("Sorting Completed!");
    m_completionTitle.setCharacterSize(18);
    m_completionTitle.setFillColor(colors.success);

    m_completionStatsText.setFont(regularFont);
    m_completionStatsText.setCharacterSize(12);
    m_completionStatsText.setFillColor(colors.textPrimary);

    m_completionResetBtn = std::make_unique<ui::Button>("Run Again", sf::Vector2f(100.f, 32.f), 13);
    m_completionResetBtn->setColors(colors.success, colors.accentHover, colors.textPrimary);
    m_completionResetBtn->setCallback([this]() {
        onReset();
    });
}

void SortingVisualizerScreen::updateSortingLayout() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    // Selector buttons layout (top row)
    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    for (size_t i = 0; i < m_selectorButtons.size(); ++i) {
        m_selectorButtons[i]->setPosition(sf::Vector2f(startX + i * (88.f + spacing), startY));
    }

    // Generator buttons layout (second row)
    float genStartY = startY + 36.f;
    for (size_t i = 0; i < m_genButtons.size(); ++i) {
        m_genButtons[i]->setPosition(sf::Vector2f(startX + i * (85.f + spacing), genStartY));
    }

    // Explanation label position
    m_explanationText.setPosition(bounds.left + 15.f, genStartY + 35.f);

    // Live Statistics placement in Info Panel (right column)
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 75.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsComparisonsText.setPosition(infoBounds.left, statsStartY + 22.f);
    m_statsSwapsText.setPosition(infoBounds.left, statsStartY + 40.f);
    m_statsStepText.setPosition(infoBounds.left, statsStartY + 58.f);
    m_statsSizeText.setPosition(infoBounds.left, statsStartY + 76.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 94.f);

    // Push down description/theory headers
    m_theoryTitle.setPosition(infoBounds.left, statsStartY + 130.f);
    m_theoryBody.setPosition(infoBounds.left, statsStartY + 155.f);

    // Push down pseudocode line text stacking
    float pseudocodeStartY = statsStartY + 270.f;
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        m_pseudocodeTexts[i].setPosition(infoBounds.left + 10.f, pseudocodeStartY + i * 16.f);
    }

    // Layout Completion Overlay in the center of visualization panel
    float ox = bounds.left + (bounds.width - 340.f) / 2.f;
    float oy = bounds.top + (bounds.height - 270.f) / 2.f;
    m_completionOverlay->setPosition(sf::Vector2f(ox, oy));

    sf::FloatRect obounds = m_completionOverlay->getContentBounds();
    m_completionTitle.setPosition(obounds.left, obounds.top + 5.f);
    m_completionStatsText.setPosition(obounds.left, obounds.top + 35.f);
    m_completionResetBtn->setPosition(sf::Vector2f(obounds.left + (obounds.width - 100.f) / 2.f, obounds.top + obounds.height - 40.f));
}

void SortingVisualizerScreen::loadAlgorithm(SortingAlgorithm algo) {
    m_currentAlgorithm = algo;
    m_isPlaying = false;
    m_playPauseButton->setLabel("Play");
    
    const auto& colors = ThemeManager::getInstance().getColors();
    m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);

    // Highlight selected tab
    for (size_t i = 0; i < m_selectorButtons.size(); ++i) {
        if (i == static_cast<size_t>(algo)) {
            m_selectorButtons[i]->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        } else {
            m_selectorButtons[i]->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        }
    }

    // Load complexity and theory description info
    switch (algo) {
        case SortingAlgorithm::Bubble:
            m_complexityTitle.setString("Bubble Sort");
            m_timeComplexity.setString("Time Complexity: O(N^2)");
            m_spaceComplexity.setString("Space Complexity: O(1)");
            m_theoryBody.setString("Bubble Sort works by repeatedly swapping adjacent elements if they are in the wrong order. It is stable and in-place but highly inefficient for large arrays.");
            break;
        case SortingAlgorithm::Selection:
            m_complexityTitle.setString("Selection Sort");
            m_timeComplexity.setString("Time Complexity: O(N^2)");
            m_spaceComplexity.setString("Space Complexity: O(1)");
            m_theoryBody.setString("Selection Sort divides the array into sorted and unsorted parts. It repeatedly finds the minimum element from the unsorted part and swaps it into position. It is unstable but in-place.");
            break;
        case SortingAlgorithm::Insertion:
            m_complexityTitle.setString("Insertion Sort");
            m_timeComplexity.setString("Time Complexity: O(N^2)");
            m_spaceComplexity.setString("Space Complexity: O(1)");
            m_theoryBody.setString("Insertion Sort builds the sorted array one item at a time. It consumes one element per iteration and places it in its correct relative position. Highly efficient for small or nearly sorted data.");
            break;
        case SortingAlgorithm::Merge:
            m_complexityTitle.setString("Merge Sort");
            m_timeComplexity.setString("Time Complexity: O(N log N)");
            m_spaceComplexity.setString("Space Complexity: O(N)");
            m_theoryBody.setString("Merge Sort is a stable, divide-and-conquer algorithm. It recursively splits the array in halves, sorts each half, and merges them. Requires auxiliary space for temporary buffers.");
            break;
        case SortingAlgorithm::Quick:
            m_complexityTitle.setString("Quick Sort");
            m_timeComplexity.setString("Time Complexity: O(N log N)");
            m_spaceComplexity.setString("Space Complexity: O(log N)");
            m_theoryBody.setString("Quick Sort selects a 'pivot' element and partitions the array such that elements smaller than the pivot go to the left and larger to the right. Fast, unstable, and sorts in-place.");
            break;
        case SortingAlgorithm::Heap:
            m_complexityTitle.setString("Heap Sort");
            m_timeComplexity.setString("Time Complexity: O(N log N)");
            m_spaceComplexity.setString("Space Complexity: O(1)");
            m_theoryBody.setString("Heap Sort uses a binary heap data structure. It builds a max-heap, then repeatedly extracts the maximum element and restores the heap property. Unstable but runs in-place with guaranteed O(N log N).");
            break;
    }

    loadPseudocode(algo);
    onReset();
}

void SortingVisualizerScreen::loadPseudocode(SortingAlgorithm algo) {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    switch (algo) {
        case SortingAlgorithm::Bubble:
            m_pseudocodeLines = {
                "bubbleSort(Array A):",
                "  for i from 0 to A.length - 1:",
                "    for j from 0 to A.length - i - 2:",
                "      if A[j] > A[j+1]:",
                "        swap(A[j], A[j+1])"
            };
            break;
        case SortingAlgorithm::Selection:
            m_pseudocodeLines = {
                "selectionSort(Array A):",
                "  for i from 0 to A.length - 1:",
                "    min_idx = i",
                "    for j from i + 1 to A.length - 1:",
                "      if A[j] < A[min_idx]:",
                "        min_idx = j",
                "    swap(A[i], A[min_idx])"
            };
            break;
        case SortingAlgorithm::Insertion:
            m_pseudocodeLines = {
                "insertionSort(Array A):",
                "  for i from 1 to A.length - 1:",
                "    key = A[i]",
                "    j = i - 1",
                "    while j >= 0 and A[j] > key:",
                "      A[j+1] = A[j]",
                "      j = j - 1",
                "    A[j+1] = key"
            };
            break;
        case SortingAlgorithm::Merge:
            m_pseudocodeLines = {
                "mergeSort(Array A, left, right):",
                "  if left < right:",
                "    mid = (left + right) / 2",
                "    mergeSort(A, left, mid)",
                "    mergeSort(A, mid + 1, right)",
                "    merge(A, left, mid, right)"
            };
            break;
        case SortingAlgorithm::Quick:
            m_pseudocodeLines = {
                "quickSort(Array A, low, high):",
                "  if low < high:",
                "    p = partition(A, low, high)",
                "    quickSort(A, low, p - 1)",
                "    quickSort(A, p + 1, high)"
            };
            break;
        case SortingAlgorithm::Heap:
            m_pseudocodeLines = {
                "heapSort(Array A):",
                "  buildMaxHeap(A)",
                "  for i from A.length - 1 down to 1:",
                "    swap(A[0], A[i])",
                "    maxHeapify(A, 0, i)"
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
    updateSortingLayout();
}

// -------------------------------------------------------------
// ARRAY GENERATION STRATEGIES
// -------------------------------------------------------------
void SortingVisualizerScreen::generateRandomArray() {
    m_originalArray.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        m_originalArray.push_back(5 + rand() % 95);
    }
    onReset();
}

void SortingVisualizerScreen::shuffleArray() {
    std::random_shuffle(m_originalArray.begin(), m_originalArray.end());
    onReset();
}

void SortingVisualizerScreen::generateReverseSortedArray() {
    m_originalArray.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        float t = static_cast<float>(m_dataSize - 1 - i) / std::max(1, m_dataSize - 1);
        m_originalArray.push_back(static_cast<int>(5 + t * 90));
    }
    onReset();
}

void SortingVisualizerScreen::generateNearlySortedArray() {
    m_originalArray.clear();
    for (int i = 0; i < m_dataSize; ++i) {
        float t = static_cast<float>(i) / std::max(1, m_dataSize - 1);
        m_originalArray.push_back(static_cast<int>(5 + t * 90));
    }
    
    // Apply 5% random swaps
    int numSwaps = std::max(1, m_dataSize / 20);
    for (int s = 0; s < numSwaps; ++s) {
        int idx1 = rand() % m_dataSize;
        int idx2 = std::min(m_dataSize - 1, idx1 + 1 + rand() % 3);
        std::swap(m_originalArray[idx1], m_originalArray[idx2]);
    }
    onReset();
}

// -------------------------------------------------------------
// SIMULATION CONTROL
// -------------------------------------------------------------
void SortingVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_compareIdx1 = -1;
    m_compareIdx2 = -1;
    m_swapIdx1 = -1;
    m_swapIdx2 = -1;

    m_isSorted.clear();
    m_isSorted.assign(m_originalArray.size(), false);
    m_currentArray = m_originalArray;

    // Reset Live Stats
    m_currentComparisons = 0;
    m_currentSwaps = 0;
    m_elapsedTime = 0.f;
    m_showCompletion = false;

    // Re-record step queue
    runSortingRecord();

    // Re-compute totals
    m_totalComparisons = 0;
    m_totalSwaps = 0;
    for (const auto& step : m_steps) {
        if (step.type == SortStep::Type::Compare) m_totalComparisons++;
        if (step.type == SortStep::Type::Swap || step.type == SortStep::Type::Write) m_totalSwaps++;
    }

    m_explanationText.setString("Ready. Click Play or Step to visualize.");
    m_activePseudocodeLine = -1;
    
    if (m_window) {
        updateSortingLayout();
    }
}

void SortingVisualizerScreen::onSizeChanged(int newSize) {
    m_dataSize = newSize;
    generateRandomArray();
}

void SortingVisualizerScreen::runSortingRecord() {
    switch (m_currentAlgorithm) {
        case SortingAlgorithm::Bubble: recordBubbleSort(); break;
        case SortingAlgorithm::Selection: recordSelectionSort(); break;
        case SortingAlgorithm::Insertion: recordInsertionSort(); break;
        case SortingAlgorithm::Merge: recordMergeSort(); break;
        case SortingAlgorithm::Quick: recordQuickSort(); break;
        case SortingAlgorithm::Heap: recordHeapSort(); break;
    }
}

void SortingVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        
        // Finalize state
        for (size_t i = 0; i < m_isSorted.size(); ++i) {
            m_isSorted[i] = true;
        }
        m_compareIdx1 = -1;
        m_compareIdx2 = -1;
        m_swapIdx1 = -1;
        m_swapIdx2 = -1;
        m_activePseudocodeLine = -1;
        
        // Show Completion Overlay Card
        m_showCompletion = true;
        
        std::string algoName = "";
        std::string timeComplexity = "";
        std::string spaceComplexity = "";
        switch (m_currentAlgorithm) {
            case SortingAlgorithm::Bubble: algoName = "Bubble Sort"; timeComplexity = "O(N^2)"; spaceComplexity = "O(1)"; break;
            case SortingAlgorithm::Selection: algoName = "Selection Sort"; timeComplexity = "O(N^2)"; spaceComplexity = "O(1)"; break;
            case SortingAlgorithm::Insertion: algoName = "Insertion Sort"; timeComplexity = "O(N^2)"; spaceComplexity = "O(1)"; break;
            case SortingAlgorithm::Merge: algoName = "Merge Sort"; timeComplexity = "O(N log N)"; spaceComplexity = "O(N)"; break;
            case SortingAlgorithm::Quick: algoName = "Quick Sort"; timeComplexity = "O(N log N)"; spaceComplexity = "O(log N)"; break;
            case SortingAlgorithm::Heap: algoName = "Heap Sort"; timeComplexity = "O(N log N)"; spaceComplexity = "O(1)"; break;
        }

        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), 
                     "Algorithm: %s\nTotal Comparisons: %d\nTotal Swaps / Writes: %d\nVisualization Time: %.2fs\nTime Complexity: %s\nSpace Complexity: %s", 
                     algoName.c_str(), m_totalComparisons, m_totalSwaps, m_elapsedTime, 
                     timeComplexity.c_str(), spaceComplexity.c_str());
        m_completionStatsText.setString(buffer);
        m_explanationText.setString("Sorting complete!");
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);

    switch (step.type) {
        case SortStep::Type::Compare:
            m_currentComparisons++;
            m_compareIdx1 = step.index1;
            m_compareIdx2 = step.index2;
            m_swapIdx1 = -1;
            m_swapIdx2 = -1;
            break;
        case SortStep::Type::Swap:
            m_currentSwaps++;
            m_swapIdx1 = step.index1;
            m_swapIdx2 = step.index2;
            m_compareIdx1 = -1;
            m_compareIdx2 = -1;
            if (step.index1 < static_cast<int>(m_currentArray.size()) && 
                step.index2 < static_cast<int>(m_currentArray.size())) {
                std::swap(m_currentArray[step.index1], m_currentArray[step.index2]);
            }
            break;
        case SortStep::Type::Write:
            m_currentSwaps++;
            m_swapIdx1 = step.index1;
            m_swapIdx2 = -1;
            m_compareIdx1 = -1;
            m_compareIdx2 = -1;
            if (step.index1 < static_cast<int>(m_currentArray.size())) {
                m_currentArray[step.index1] = step.value;
            }
            break;
        case SortStep::Type::Explain:
            m_compareIdx1 = -1;
            m_compareIdx2 = -1;
            m_swapIdx1 = -1;
            m_swapIdx2 = -1;
            break;
        case SortStep::Type::HighlightLine:
            break;
        case SortStep::Type::MarkSorted:
            if (step.index1 < static_cast<int>(m_isSorted.size())) {
                m_isSorted[step.index1] = true;
            }
            break;
    }

    m_currentStepIndex++;
}

void SortingVisualizerScreen::updateVisuals(float deltaTime) {
    if (m_window) {
        if (m_showCompletion) {
            m_completionResetBtn->update(deltaTime, *m_window);
        } else {
            for (auto& btn : m_selectorButtons) btn->update(deltaTime, *m_window);
            for (auto& btn : m_genButtons) btn->update(deltaTime, *m_window);
        }
    }

    if (m_isPlaying && !m_showCompletion) {
        m_elapsedTime += deltaTime;
        m_stepAccumulator += deltaTime * m_animationSpeed;
        
        float stepDelay = 0.08f; // base delay at speed multiplier = 1
        if (m_stepAccumulator >= stepDelay) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }

    // Refresh live stats strings
    m_statsComparisonsText.setString("Comparisons: " + std::to_string(m_currentComparisons) + " / " + std::to_string(m_totalComparisons));
    m_statsSwapsText.setString("Swaps / Writes: " + std::to_string(m_currentSwaps) + " / " + std::to_string(m_totalSwaps));
    m_statsStepText.setString("Step: " + std::to_string(m_currentStepIndex) + " / " + std::to_string(m_steps.size()));
    m_statsSizeText.setString("Array Size: " + std::to_string(m_dataSize));

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Elapsed Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);
}

void SortingVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    if (m_showCompletion) {
        m_completionResetBtn->handleEvent(event, window);
    } else {
        for (auto& btn : m_selectorButtons) btn->handleEvent(event, window);
        for (auto& btn : m_genButtons) btn->handleEvent(event, window);
    }
}

void SortingVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();

    // 1. Draw Controls
    for (auto& btn : m_selectorButtons) window.draw(*btn);
    for (auto& btn : m_genButtons) window.draw(*btn);
    window.draw(m_explanationText);

    // 2. Draw Array Bars
    int count = m_currentArray.size();
    if (count > 0) {
        float drawAreaH = bounds.height - 120.f;
        float padding = 1.5f;
        float barWidth = bounds.width / count;

        for (int i = 0; i < count; ++i) {
            float val = m_currentArray[i] / 100.f;
            float barHeight = val * drawAreaH;

            sf::RectangleShape bar(sf::Vector2f(std::max(0.5f, barWidth - padding), barHeight));
            bar.setPosition(bounds.left + i * barWidth + padding / 2.f, bounds.top + bounds.height - barHeight - 10.f);

            if (i == m_swapIdx1 || i == m_swapIdx2) {
                bar.setFillColor(colors.barSwap);
            } else if (i == m_compareIdx1 || i == m_compareIdx2) {
                bar.setFillColor(colors.barCompare);
            } else if (m_isSorted[i]) {
                bar.setFillColor(colors.barSorted);
            } else {
                bar.setFillColor(colors.barDefault);
            }
            window.draw(bar);
        }
    }

    // 3. Render Statistics in Info panel
    window.draw(m_statsTitleText);
    window.draw(m_statsComparisonsText);
    window.draw(m_statsSwapsText);
    window.draw(m_statsStepText);
    window.draw(m_statsSizeText);
    window.draw(m_statsTimeText);

    // 4. Render Pseudocode in Info panel
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

    // 5. Draw Completion Overlay Card
    if (m_showCompletion) {
        window.draw(*m_completionOverlay);
        window.draw(m_completionTitle);
        window.draw(m_completionStatsText);
        window.draw(*m_completionResetBtn);
    }
}

// -------------------------------------------------------------
// STEP RECORDERS
// -------------------------------------------------------------
void SortingVisualizerScreen::addCompareStep(int idx1, int idx2, int line, const std::string& explanation) {
    SortStep step;
    step.type = SortStep::Type::Compare;
    step.index1 = idx1;
    step.index2 = idx2;
    step.value = 0;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SortingVisualizerScreen::addSwapStep(int idx1, int idx2, int line, const std::string& explanation) {
    SortStep step;
    step.type = SortStep::Type::Swap;
    step.index1 = idx1;
    step.index2 = idx2;
    step.value = 0;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SortingVisualizerScreen::addWriteStep(int idx, int value, int line, const std::string& explanation) {
    SortStep step;
    step.type = SortStep::Type::Write;
    step.index1 = idx;
    step.index2 = -1;
    step.value = value;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SortingVisualizerScreen::addHighlightStep(int line) {
    SortStep step;
    step.type = SortStep::Type::HighlightLine;
    step.index1 = -1;
    step.index2 = -1;
    step.value = 0;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    m_steps.push_back(step);
}

void SortingVisualizerScreen::addExplainStep(const std::string& explanation) {
    SortStep step;
    step.type = SortStep::Type::Explain;
    step.index1 = -1;
    step.index2 = -1;
    step.value = 0;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void SortingVisualizerScreen::addMarkSortedStep(int idx) {
    SortStep step;
    step.type = SortStep::Type::MarkSorted;
    step.index1 = idx;
    step.index2 = -1;
    step.value = 0;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    m_steps.push_back(step);
}

// -------------------------------------------------------------
// 6 ALGORITHM RECORDER IMPLEMENTATIONS
// -------------------------------------------------------------
void SortingVisualizerScreen::recordBubbleSort() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();
    
    addExplainStep("Initializing Bubble Sort loops.");
    addHighlightStep(0);
    
    for (int i = 0; i < n; ++i) {
        addHighlightStep(1);
        for (int j = 0; j < n - i - 1; ++j) {
            addHighlightStep(2);
            addCompareStep(j, j + 1, 3, "Comparing A[" + std::to_string(j) + "] and A[" + std::to_string(j+1) + "]");
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                addSwapStep(j, j + 1, 4, "Swapping A[" + std::to_string(j) + "] and A[" + std::to_string(j+1) + "]");
            }
        }
        addMarkSortedStep(n - i - 1);
    }
}

void SortingVisualizerScreen::recordSelectionSort() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();

    addExplainStep("Initializing Selection Sort.");
    addHighlightStep(0);

    for (int i = 0; i < n; ++i) {
        addHighlightStep(1);
        int min_idx = i;
        addExplainStep("Set min_idx = " + std::to_string(i));
        addHighlightStep(2);

        for (int j = i + 1; j < n; ++j) {
            addHighlightStep(3);
            addCompareStep(j, min_idx, 4, "Comparing A[" + std::to_string(j) + "] with min A[" + std::to_string(min_idx) + "]");
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
                addExplainStep("New minimum at index " + std::to_string(min_idx));
                addHighlightStep(5);
            }
        }
        if (min_idx != i) {
            std::swap(arr[i], arr[min_idx]);
            addSwapStep(i, min_idx, 6, "Swapping A[" + std::to_string(i) + "] with min A[" + std::to_string(min_idx) + "]");
        }
        addMarkSortedStep(i);
    }
}

void SortingVisualizerScreen::recordInsertionSort() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();

    addExplainStep("Initializing Insertion Sort.");
    addHighlightStep(0);

    for (int i = 1; i < n; ++i) {
        addHighlightStep(1);
        int key = arr[i];
        int j = i - 1;
        addExplainStep("Inserting key A[" + std::to_string(i) + "] into sorted partition");
        addHighlightStep(2);
        addHighlightStep(3);

        while (j >= 0) {
            addCompareStep(j, j + 1, 4, "Comparing A[" + std::to_string(j) + "] and key (" + std::to_string(key) + ")");
            if (arr[j] > key) {
                arr[j + 1] = arr[j];
                addWriteStep(j + 1, arr[j], 5, "A[" + std::to_string(j) + "] > key. Shifting right to index " + std::to_string(j + 1));
                j--;
                addHighlightStep(6);
            } else {
                break;
            }
        }
        arr[j + 1] = key;
        addWriteStep(j + 1, key, 7, "Placing key at index " + std::to_string(j + 1));
    }
    
    for (int i = 0; i < n; ++i) {
        addMarkSortedStep(i);
    }
}

void SortingVisualizerScreen::recordMergeSort() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();

    addExplainStep("Starting recursive Merge Sort.");
    recordMergeSortHelper(arr, 0, n - 1);
    
    for (int i = 0; i < n; ++i) {
        addMarkSortedStep(i);
    }
}

void SortingVisualizerScreen::recordMergeSortHelper(std::vector<int>& arr, int left, int right) {
    addHighlightStep(0);
    if (left < right) {
        int mid = left + (right - left) / 2;
        addHighlightStep(2);
        
        addHighlightStep(3);
        recordMergeSortHelper(arr, left, mid);
        
        addHighlightStep(4);
        recordMergeSortHelper(arr, mid + 1, right);
        
        addHighlightStep(5);
        recordMerge(arr, left, mid, right);
    }
}

void SortingVisualizerScreen::recordMerge(std::vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> L(n1);
    std::vector<int> R(n2);

    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        addCompareStep(left + i, mid + 1 + j, 5, "Comparing temp elements L[" + std::to_string(i) + "] with R[" + std::to_string(j) + "]");
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            addWriteStep(k, L[i], 5, "Writing L[" + std::to_string(i) + "] to position " + std::to_string(k));
            i++;
        } else {
            arr[k] = R[j];
            addWriteStep(k, R[j], 5, "Writing R[" + std::to_string(j) + "] to position " + std::to_string(k));
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        addWriteStep(k, L[i], 5, "Copying remaining L[" + std::to_string(i) + "] to position " + std::to_string(k));
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        addWriteStep(k, R[j], 5, "Copying remaining R[" + std::to_string(j) + "] to position " + std::to_string(k));
        j++;
        k++;
    }
}

void SortingVisualizerScreen::recordQuickSort() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();

    addExplainStep("Starting recursive Quick Sort.");
    recordQuickSortHelper(arr, 0, n - 1);
    
    for (int i = 0; i < n; ++i) {
        addMarkSortedStep(i);
    }
}

void SortingVisualizerScreen::recordQuickSortHelper(std::vector<int>& arr, int low, int high) {
    addHighlightStep(0);
    if (low < high) {
        addHighlightStep(2);
        int p = recordQuickPartition(arr, low, high);
        
        addHighlightStep(3);
        recordQuickSortHelper(arr, low, p - 1);
        
        addHighlightStep(4);
        recordQuickSortHelper(arr, p + 1, high);
    } else if (low >= 0 && low < static_cast<int>(arr.size())) {
        addMarkSortedStep(low);
    }
}

int SortingVisualizerScreen::recordQuickPartition(std::vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    addExplainStep("Select pivot " + std::to_string(pivot) + " at A[" + std::to_string(high) + "]");
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        addCompareStep(j, high, 2, "Comparing A[" + std::to_string(j) + "] with pivot " + std::to_string(pivot));
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
            addSwapStep(i, j, 2, "Swapping smaller element A[" + std::to_string(i) + "] and A[" + std::to_string(j) + "]");
        }
    }
    std::swap(arr[i + 1], arr[high]);
    addSwapStep(i + 1, high, 2, "Partitioning done. Swapping pivot to index " + std::to_string(i + 1));
    addMarkSortedStep(i + 1);
    return i + 1;
}

void SortingVisualizerScreen::recordHeapSort() {
    std::vector<int> arr = m_originalArray;
    int n = arr.size();

    addExplainStep("Building max heap.");
    addHighlightStep(0);

    for (int i = n / 2 - 1; i >= 0; --i) {
        recordHeapify(arr, n, i);
    }

    addExplainStep("Heap built. Extracting elements sequentially.");
    for (int i = n - 1; i > 0; --i) {
        addHighlightStep(2);
        std::swap(arr[0], arr[i]);
        addSwapStep(0, i, 3, "Swapping heap root A[0] (max) with leaf A[" + std::to_string(i) + "]");
        addMarkSortedStep(i);
        
        addHighlightStep(4);
        recordHeapify(arr, i, 0);
    }
    addMarkSortedStep(0);
}

void SortingVisualizerScreen::recordHeapify(std::vector<int>& arr, int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;

    if (l < n) {
        addCompareStep(l, largest, 4, "Comparing left child A[" + std::to_string(l) + "] with largest A[" + std::to_string(largest) + "]");
        if (arr[l] > arr[largest]) {
            largest = l;
        }
    }

    if (r < n) {
        addCompareStep(r, largest, 4, "Comparing right child A[" + std::to_string(r) + "] with largest A[" + std::to_string(largest) + "]");
        if (arr[r] > arr[largest]) {
            largest = r;
        }
    }

    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        addSwapStep(i, largest, 4, "Swapping root A[" + std::to_string(i) + "] with child A[" + std::to_string(largest) + "]");
        recordHeapify(arr, n, largest);
    }
}

} // namespace core
