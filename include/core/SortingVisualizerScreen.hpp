#ifndef SORTING_VISUALIZER_SCREEN_HPP
#define SORTING_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

enum class SortingAlgorithm {
    Bubble,
    Selection,
    Insertion,
    Merge,
    Quick,
    Heap
};

struct SortStep {
    enum class Type {
        Compare,
        Swap,
        Write,
        HighlightLine,
        Explain,
        MarkSorted
    };
    Type type;
    int index1;
    int index2;
    int value;
    int line;
    std::string explanation;
};

class SortingVisualizerScreen : public VisualizerBase {
public:
    explicit SortingVisualizerScreen(sf::RenderWindow& window);
    virtual ~SortingVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    SortingAlgorithm m_currentAlgorithm;

    std::vector<int> m_originalArray;
    std::vector<int> m_currentArray;
    std::vector<SortStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Highlights state
    int m_compareIdx1;
    int m_compareIdx2;
    int m_swapIdx1;
    int m_swapIdx2;
    std::vector<bool> m_isSorted;

    // Algorithm selection tabs
    std::vector<std::unique_ptr<ui::Button>> m_selectorButtons;

    // Array Generation buttons
    std::vector<std::unique_ptr<ui::Button>> m_genButtons;

    // Live Statistics (drawn inside Info Panel)
    int m_currentComparisons;
    int m_currentSwaps;
    int m_totalComparisons;
    int m_totalSwaps;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsComparisonsText;
    sf::Text m_statsSwapsText;
    sf::Text m_statsStepText;
    sf::Text m_statsSizeText;
    sf::Text m_statsTimeText;

    // Completion Summary Overlay
    bool m_showCompletion;
    std::unique_ptr<ui::Panel> m_completionOverlay;
    sf::Text m_completionTitle;
    sf::Text m_completionStatsText;
    std::unique_ptr<ui::Button> m_completionResetBtn;

    // Explanations & Pseudocode
    sf::Text m_explanationText;
    std::vector<std::string> m_pseudocodeLines;
    std::vector<sf::Text> m_pseudocodeTexts;
    int m_activePseudocodeLine;

    void initSortingUI();
    void updateSortingLayout();
    void loadAlgorithm(SortingAlgorithm algo);

    // Array Generator strategies
    void generateRandomArray();
    void shuffleArray();
    void generateReverseSortedArray();
    void generateNearlySortedArray();

    // Recording operations
    void runSortingRecord();
    void addCompareStep(int idx1, int idx2, int line, const std::string& explanation);
    void addSwapStep(int idx1, int idx2, int line, const std::string& explanation);
    void addWriteStep(int idx, int value, int line, const std::string& explanation);
    void addHighlightStep(int line);
    void addExplainStep(const std::string& explanation);
    void addMarkSortedStep(int idx);

    // Algorithm implementations for the recorder
    void recordBubbleSort();
    void recordSelectionSort();
    void recordInsertionSort();
    
    void recordMergeSort();
    void recordMergeSortHelper(std::vector<int>& arr, int left, int right);
    void recordMerge(std::vector<int>& arr, int left, int mid, int right);
    
    void recordQuickSort();
    void recordQuickSortHelper(std::vector<int>& arr, int low, int high);
    int recordQuickPartition(std::vector<int>& arr, int low, int high);
    
    void recordHeapSort();
    void recordHeapify(std::vector<int>& arr, int n, int i);

    // Dynamic pseudocode loading
    void loadPseudocode(SortingAlgorithm algo);
};

} // namespace core

#endif // SORTING_VISUALIZER_SCREEN_HPP
