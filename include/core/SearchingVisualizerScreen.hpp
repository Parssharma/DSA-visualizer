#ifndef SEARCHING_VISUALIZER_SCREEN_HPP
#define SEARCHING_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

enum class SearchingAlgorithm {
    Linear,
    Binary
};

struct SearchStep {
    enum class Type {
        Compare,
        SetPointers, // low, high, mid
        DiscardSection,
        Explain,
        HighlightLine,
        Found,
        NotFound
    };
    Type type;
    int index1; // current / mid / low
    int index2; // high
    int value;  // compare val
    int line;
    std::string explanation;
};

class SearchingVisualizerScreen : public VisualizerBase {
public:
    explicit SearchingVisualizerScreen(sf::RenderWindow& window);
    virtual ~SearchingVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    SearchingAlgorithm m_currentAlgorithm;

    std::vector<int> m_originalArray;
    std::vector<int> m_currentArray;
    int m_targetValue;
    std::vector<SearchStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Highlights state
    int m_currentIdx;
    std::vector<bool> m_isVisited;
    int m_lowIdx;
    int m_highIdx;
    int m_midIdx;
    int m_foundIdx;

    // UI algorithm selection tabs
    std::vector<std::unique_ptr<ui::Button>> m_selectorButtons;

    // Array Generation buttons
    std::vector<std::unique_ptr<ui::Button>> m_genButtons;

    // Target selector controls
    std::unique_ptr<ui::Button> m_decTargetBtn;
    std::unique_ptr<ui::Button> m_incTargetBtn;
    std::unique_ptr<ui::Button> m_randTargetBtn;
    std::unique_ptr<ui::Button> m_pickExistBtn;
    std::unique_ptr<ui::Button> m_inputFieldFrame;
    sf::Text m_targetLabelText;

    // Manual Input State
    bool m_isInputActive;
    std::string m_inputTextBuffer;
    float m_cursorBlinkTime;

    // Live Statistics (drawn inside Info Panel)
    int m_currentComparisons;
    int m_totalComparisons;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsComparisonsText;
    sf::Text m_statsStepText;
    sf::Text m_statsSizeText;
    sf::Text m_statsTimeText;
    sf::Text m_statsRangeText;

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

    void initSearchingUI();
    void updateSearchingLayout();
    void loadAlgorithm(SearchingAlgorithm algo);

    // Array Generator strategies
    void generateRandomArray();
    void generateSortedArray();
    void shuffleArray();
    void generateReverseSortedArray();
    void generateNearlySortedArray();

    // Target Selection helpers
    void pickExistingTarget();
    void generateRandomTarget();
    void adjustTarget(int delta);
    void commitTargetInput();

    // Recording operations
    void runSearchingRecord();
    void addCompareStep(int idx, int line, const std::string& explanation);
    void addSetPointersStep(int low, int mid, int high, int line, const std::string& explanation);
    void addDiscardStep(int idx1, int idx2, int line, const std::string& explanation);
    void addHighlightStep(int line);
    void addExplainStep(const std::string& explanation);
    void addFoundStep(int idx, int line, const std::string& explanation);
    void addNotFoundStep(int line, const std::string& explanation);

    // Algorithm implementations for the recorder
    void recordLinearSearch();
    void recordBinarySearch();

    // Dynamic pseudocode loading
    void loadPseudocode(SearchingAlgorithm algo);
};

} // namespace core

#endif // SEARCHING_VISUALIZER_SCREEN_HPP
