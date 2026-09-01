#ifndef LINKED_LIST_VISUALIZER_SCREEN_HPP
#define LINKED_LIST_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

enum class LinkedListType {
    Singly,
    Doubly,
    Circular
};

struct ListStep {
    enum class Type {
        Compare,
        TraversalHighlight,
        SetPointers,
        PointerRedirect,
        SpawnNode,
        PopNode,
        Explain,
        SuccessAlert,
        FailAlert,
        Clear,
        HighlightLine,
        CommitNode
    };
    Type type;
    int index1; // current idx / head
    int index2; // prev idx / tail
    int value;  // element value / target
    int line;   // pseudocode line
    std::string explanation;
};

class LinkedListVisualizerScreen : public VisualizerBase {
public:
    explicit LinkedListVisualizerScreen(sf::RenderWindow& window);
    virtual ~LinkedListVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    LinkedListType m_listType;
    std::vector<int> m_nodes; // committed node values
    std::vector<std::string> m_nodeAddresses; // simulated hex address per node (0x1000, 0x1008...)
    
    int m_inputValue;
    int m_inputPos;
    bool m_isEducationalMode;

    std::vector<ListStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Highlights state
    int m_activeIdx;
    int m_prevIdx;
    int m_nextIdx;
    int m_headIdx;
    int m_tailIdx;

    // Pointer target coordinates for animations
    sf::Vector2f m_nextArrowTargets[7];
    sf::Vector2f m_prevArrowTargets[7];
    sf::Vector2f m_nextArrowCurrent[7];
    sf::Vector2f m_prevArrowCurrent[7];

    // Live Action History Panel
    std::vector<std::string> m_historyList;
    sf::Text m_historyTitleText;
    std::vector<sf::Text> m_historyTexts;

    // Selector and Control Buttons
    std::vector<std::unique_ptr<ui::Button>> m_typeButtons; // Singly, Doubly, Circular
    std::vector<std::unique_ptr<ui::Button>> m_opButtons;   // Insert Head, Tail, Pos, Del Head, Tail, Pos, Val, Traverse, Search, Reverse, Clear, Gen
    
    std::unique_ptr<ui::Button> m_decValBtn;
    std::unique_ptr<ui::Button> m_incValBtn;
    std::unique_ptr<ui::Button> m_randValBtn;
    std::unique_ptr<ui::Button> m_valInputFrame;

    std::unique_ptr<ui::Button> m_decPosBtn;
    std::unique_ptr<ui::Button> m_incPosBtn;
    std::unique_ptr<ui::Button> m_posInputFrame;

    std::unique_ptr<ui::Button> m_eduModeBtn;

    // Manual typing input states
    bool m_isValActive;
    bool m_isPosActive;
    std::string m_inputTextBuffer;
    float m_cursorBlinkTime;

    // Live Statistics
    int m_capacity; // cap = 6
    int m_operationCount;
    int m_traversalCount;
    int m_insertCount;
    int m_deleteCount;
    int m_searchComparisons;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsSizeText;
    sf::Text m_statsOpsText;
    sf::Text m_statsTraversalsText;
    sf::Text m_statsInsText;
    sf::Text m_statsDelText;
    sf::Text m_statsCompsText;
    sf::Text m_statsTimeText;

    // Overlay panels
    bool m_showAlert;
    std::string m_alertTitle;
    std::string m_alertMessage;
    std::unique_ptr<ui::Panel> m_alertOverlay;
    sf::Text m_alertTitleText;
    sf::Text m_alertMsgText;
    std::unique_ptr<ui::Button> m_alertResetBtn;

    // Explanations & Pseudocode
    sf::Text m_explanationText;
    std::vector<std::string> m_pseudocodeLines;
    std::vector<sf::Text> m_pseudocodeTexts;
    int m_activePseudocodeLine;

    void initUIComponents();
    void updateUILayouts();
    void loadTypeMode(LinkedListType type);

    // Value adjustment helpers
    void adjustVal(int delta);
    void adjustPos(int delta);
    void pickRandomValue();
    void commitInputText();

    // Linked List operational step recorders
    void triggerInsertHead();
    void triggerInsertTail();
    void triggerInsertPos();
    void triggerDeleteHead();
    void triggerDeleteTail();
    void triggerDeletePos();
    void triggerDeleteVal();
    void triggerTraverse();
    void triggerSearch();
    void triggerReverse();
    void triggerClear();
    void triggerGenRandom();

    // Steps queue helpers
    void addCompareStep(int val, int line, const std::string& explanation);
    void addTraversalStep(int idx, int line, const std::string& explanation);
    void addSetPointersStep(int prev, int curr, int next, int line, const std::string& explanation);
    void addPointerRedirectStep(int fromIdx, int toIdx, const std::string& ptrName, int line, const std::string& explanation);
    void addSpawnNodeStep(int val, int line, const std::string& explanation);
    void addPopNodeStep(int val, int idx, int line, const std::string& explanation);
    void addExplainStep(const std::string& explanation);
    void addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addClearStep(int line, const std::string& explanation);
    void addHighlightStep(int line);
    void addCommitNodeStep(int val, int line, const std::string& explanation);

    void pushHistoryLog(const std::string& log);

    // Dynamic pseudocode loading
    void loadPseudocode(LinkedListType type);
};

} // namespace core

#endif // LINKED_LIST_VISUALIZER_SCREEN_HPP
