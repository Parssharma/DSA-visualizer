#ifndef DP_VISUALIZER_SCREEN_HPP
#define DP_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

enum class DPAlgorithm {
    FibMemo,
    FibTab,
    Knapsack,
    LCS,
    CoinChange,
    MatrixChain,
    EditDistance
};

struct DPGridCell {
    int row;
    int col;
    int value;
    std::string labelRow;
    std::string labelCol;
    int state; // 0: Empty, 1: Filling (Orange), 2: Filled (Turquoise), 3: Dependency (Yellow), 4: Solution (Green)
};

struct DPDependencyLink {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
};

struct DPStep {
    enum class Type {
        CellUpdate,
        ReadCache,
        DependencyHighlight,
        BacktrackHighlight,
        Explain,
        SuccessAlert,
        FailAlert
    };
    Type type;
    int line;
    std::string explanation;
    std::vector<DPGridCell> gridSnapshot;
    std::vector<DPDependencyLink> dependencySnapshot;
    
    // Stats variables in this step
    int cellsComputed;
    int cacheHits;
    int cacheMisses;
    int statesVisited;
};

class DPVisualizerScreen : public VisualizerBase {
public:
    explicit DPVisualizerScreen(sf::RenderWindow& window);
    virtual ~DPVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    DPAlgorithm m_algorithm;
    int m_inputValue;
    bool m_isEducationalMode;

    std::vector<DPStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Visual grid cells & dependency arrows snapshot
    std::vector<DPGridCell> m_currentGridCells;
    std::vector<DPDependencyLink> m_currentDependencies;

    // UI elements
    std::vector<std::unique_ptr<ui::Button>> m_algoTabs; // FibMemo, FibTab, Knapsack, LCS, CoinChange, MatrixChain, EditDistance
    std::vector<std::unique_ptr<ui::Button>> m_opButtons;  // Run, Reset, Clear, Speed toggles

    std::unique_ptr<ui::Button> m_decValBtn;
    std::unique_ptr<ui::Button> m_incValBtn;
    std::unique_ptr<ui::Button> m_valInputFrame;
    std::unique_ptr<ui::Button> m_eduModeBtn;

    // Manual typing variables
    bool m_isValActive;
    std::string m_inputTextBuffer;
    float m_cursorBlinkTime;

    // Statistics
    int m_cellsComputed;
    int m_cacheHits;
    int m_cacheMisses;
    int m_statesVisited;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsComputedText;
    sf::Text m_statsHitsText;
    sf::Text m_statsMissesText;
    sf::Text m_statsStatesText;
    sf::Text m_statsTimeText;

    // Alert overlays
    bool m_showAlert;
    std::string m_alertTitle;
    std::string m_alertMessage;
    std::unique_ptr<ui::Panel> m_alertOverlay;
    sf::Text m_alertTitleText;
    sf::Text m_alertMsgText;
    std::unique_ptr<ui::Button> m_alertResetBtn;

    // Explanation & Pseudocode
    sf::Text m_explanationText;
    std::vector<std::string> m_pseudocodeLines;
    std::vector<sf::Text> m_pseudocodeTexts;
    int m_activePseudocodeLine;

    // Panel bounds
    sf::FloatRect m_gridPanelBounds;

    void initUIComponents();
    void updateUILayouts();
    void loadAlgorithm(DPAlgorithm algo);

    // Variable adjustments helpers
    void adjustVal(int delta);
    void commitValueInput();

    // Recorders triggers
    void triggerRun();
    
    // Core DP Recorders
    void recordFibMemo();
    int fibMemoRec(int n, std::vector<int>& memo);
    void recordFibTab();
    void recordKnapsack();
    void recordLCS();
    void recordCoinChange();
    void recordMatrixChain();
    void recordEditDistance();

    // Steps queue helper nodes
    void addCellUpdateStep(int r, int c, int val, int line, const std::string& explanation);
    void addCacheHitStep(int r, int c, int val, int line, const std::string& explanation);
    void addDependencyStep(int r, int c, const std::vector<std::pair<int, int>>& deps, int line, const std::string& explanation);
    void addBacktrackStep(int r, int c, int line, const std::string& explanation);
    void addExplainStep(const std::string& explanation);
    void addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addSnapshotStep(int line, const std::string& explanation);
    void addHighlightStep(int line);

    // Grid states helpers
    std::vector<DPGridCell> captureGridSnapshot();

    // Dynamic pseudocode loading
    void loadPseudocode(DPAlgorithm algo);
};

} // namespace core

#endif // DP_VISUALIZER_SCREEN_HPP
