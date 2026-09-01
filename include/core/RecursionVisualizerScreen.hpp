#ifndef RECURSION_VISUALIZER_SCREEN_HPP
#define RECURSION_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

enum class RecursionAlgorithm {
    Fibonacci,
    Factorial,
    BinarySearch,
    NQueens,
    Sudoku
};

struct CallStackFrame {
    std::string name;
    std::string parameters;
    int depth;
    std::string returnVal;
    std::string localVars;
    std::string returnAddr;
    int line;
};

struct CallTreeNode {
    int id;
    int parentId;
    int depth;
    std::string label;
    float x;
    float y;
    float targetX;
    float targetY;
    float scale;
    int state; // 0: Active (Orange), 1: Returned (Green), 2: Dimmed (Gray)
    std::string returnVal;
};

struct TimelineEntry {
    std::string funcName;
    int depth;
    std::string actionType; // Call, Return, Backtrack, BaseCase
    std::string description;
};

struct RecursionStep {
    enum class Type {
        PushFrame,
        PopFrame,
        Compare,
        BacktrackAction,
        BaseCaseHit,
        Explain,
        SuccessAlert,
        FailAlert,
        SnapshotState,
        HighlightLine
    };
    Type type;
    int line;
    std::string explanation;
    int activeFrameIdx;
    std::vector<CallStackFrame> framesSnapshot;
    std::vector<CallTreeNode> treeNodesSnapshot;
    std::vector<TimelineEntry> timelineSnapshot;
    
    // Backtracking grid snapshots
    std::vector<int> nQueensBoard;
    std::vector<std::vector<int>> sudokuGrid;
    int backtrackRow;
    int backtrackCol;
    int backtrackState; // 0: Gray, 1: Orange, 2: Green, 3: Red
};

class RecursionVisualizerScreen : public VisualizerBase {
public:
    explicit RecursionVisualizerScreen(sf::RenderWindow& window);
    virtual ~RecursionVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    RecursionAlgorithm m_algorithm;
    int m_inputValue;
    bool m_isEducationalMode;

    std::vector<RecursionStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Visual snapshots
    int m_activeFrameHighlight;
    std::vector<CallStackFrame> m_currentFrames;
    std::vector<CallTreeNode> m_currentTreeNodes;
    std::vector<TimelineEntry> m_currentTimeline;
    std::vector<int> m_currentNQueensBoard;
    std::vector<std::vector<int>> m_currentSudokuGrid;
    int m_activeRow;
    int m_activeCol;
    int m_activeState;

    // UI elements
    std::vector<std::unique_ptr<ui::Button>> m_algoTabs; // Fibonacci, Factorial, Binary Search, NQueens, Sudoku
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
    int m_totalCalls;
    int m_maxDepth;
    int m_currentDepth;
    int m_totalBacktracks;
    int m_solutionsFound;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsCallsText;
    sf::Text m_statsMaxDepthText;
    sf::Text m_statsCurrentDepthText;
    sf::Text m_statsBacktracksText;
    sf::Text m_statsSolutionsText;
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

    // Panels frame drawings helper bounds
    sf::FloatRect m_stackPanelBounds;
    sf::FloatRect m_treePanelBounds;
    sf::FloatRect m_gridPanelBounds;
    sf::FloatRect m_memoryPanelBounds;
    sf::FloatRect m_timelinePanelBounds;

    void initUIComponents();
    void updateUILayouts();
    void loadAlgorithm(RecursionAlgorithm algo);

    // Variable adjustments helpers
    void adjustVal(int delta);
    void commitValueInput();

    // Algorithms recorders
    void triggerRun();
    
    // Core recursive recorders (each builds a step sequence queue)
    int recordFib(int n, int parentId, int& nodeCounter, int depth, bool isLeft = true);
    int recordFact(int n, int parentId, int& nodeCounter, int depth);
    int recordBS(const std::vector<int>& arr, int target, int low, int high, int parentId, int& nodeCounter, int depth, bool isLeft = true);
    bool recordNQueens(int row, int n, int parentId, int& nodeCounter, int depth);
    bool recordSudoku(int row, int col, int parentId, int& nodeCounter, int depth);

    // Helper boards checks
    bool isNQueensSafe(const std::vector<int>& board, int row, int col);
    bool isSudokuSafe(const std::vector<std::vector<int>>& grid, int row, int col, int val);

    // Steps queue helper nodes
    void addPushStep(const std::string& name, const std::string& params, int line, const std::string& explanation);
    void addPopStep(const std::string& returnVal, int line, const std::string& explanation);
    void addCompareStep(int line, const std::string& explanation);
    void addBacktrackStep(int row, int col, int line, const std::string& explanation);
    void addBaseCaseStep(int line, const std::string& explanation);
    void addExplainStep(const std::string& explanation);
    void addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addSnapshotStep(int line, const std::string& explanation);
    void addHighlightStep(int line);

    // Tree layouter: adjusts X/Y to avoid sibling overlap
    void adjustTreePositions();
    void buildTimelineHeights();

    // Dynamic pseudocode loading
    void loadPseudocode(RecursionAlgorithm algo);
};

} // namespace core

#endif // RECURSION_VISUALIZER_SCREEN_HPP
