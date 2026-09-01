#ifndef STACK_QUEUE_VISUALIZER_SCREEN_HPP
#define STACK_QUEUE_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

enum class StackQueueMode {
    Stack,
    Queue
};

struct StackQueueStep {
    enum class Type {
        Compare, // capacity checks
        SpawnNode,
        MoveNode,
        CommitNode,
        PopNode,
        HighlightLine,
        Explain,
        OverflowAlert,
        UnderflowAlert,
        HighlightPointer,
        Clear
    };
    Type type;
    int value; // element value
    int index; // position index
    int line;  // pseudocode line
    std::string explanation;
};

class StackQueueVisualizerScreen : public VisualizerBase {
public:
    StackQueueVisualizerScreen(sf::RenderWindow& window, StackQueueMode mode);
    virtual ~StackQueueVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    StackQueueMode m_mode;
    std::vector<int> m_elements; // committed values
    int m_inputValue;
    std::vector<StackQueueStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Animation states
    bool m_isAnimating;
    int m_animVal;
    sf::Vector2f m_animStartPos;
    sf::Vector2f m_animCurrentPos;
    sf::Vector2f m_animTargetPos;
    float m_animProgress;
    std::string m_animType; // "push", "pop", "enqueue", "dequeue"

    // Highlight pointer index
    int m_highlightedIdx;
    std::string m_highlightedPointerName; // "top", "front", "rear"

    // UI Buttons
    std::vector<std::unique_ptr<ui::Button>> m_opButtons;
    std::unique_ptr<ui::Button> m_decValBtn;
    std::unique_ptr<ui::Button> m_incValBtn;
    std::unique_ptr<ui::Button> m_randValBtn;
    std::unique_ptr<ui::Button> m_inputFieldFrame;
    sf::Text m_valLabelText;

    // Manual Input State
    bool m_isInputActive;
    std::string m_inputTextBuffer;
    float m_cursorBlinkTime;

    // Statistics
    int m_capacity; // max capacity = 7
    int m_totalOperations;
    int m_pushCount;
    int m_popCount;
    int m_enqueueCount;
    int m_dequeueCount;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsSizeText;
    sf::Text m_statsCapText;
    sf::Text m_statsTotalOpsText;
    sf::Text m_statsPushEnqueueText;
    sf::Text m_statsPopDequeueText;
    sf::Text m_statsTimeText;

    // Alert overlays (Overflow, Underflow, Operation Complete)
    bool m_showAlert;
    std::string m_alertTitle;
    std::string m_alertMessage;
    float m_alertDisplayTime;
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
    void updateUILayout();
    void configureMode(StackQueueMode mode);

    // Target Value adjustment helpers
    void adjustInputValue(int delta);
    void generateRandomValue();
    void commitValueInput();

    // ADT operational triggers
    void triggerPush();
    void triggerPop();
    void triggerPeek();
    void triggerEnqueue();
    void triggerDequeue();
    void triggerFront();
    void triggerRear();
    void triggerClear();
    void triggerGenRandom();

    // Recorded Queue step helpers
    void addCompareStep(int val, int line, const std::string& explanation);
    void addSpawnNodeStep(int val, int line, const std::string& explanation);
    void addMoveNodeStep(int val, int idx, const std::string& animType, int line, const std::string& explanation);
    void addCommitNodeStep(int val, int line, const std::string& explanation);
    void addPopNodeStep(int val, int idx, const std::string& animType, int line, const std::string& explanation);
    void addHighlightStep(int line);
    void addExplainStep(const std::string& explanation);
    void addAlertStep(const std::string& alertTitle, const std::string& alertMsg, int line, const std::string& explanation);
    void addHighlightPointerStep(int idx, const std::string& ptrName, int line, const std::string& explanation);
    void addClearStep(int line, const std::string& explanation);

    // Dynamic pseudocode loading
    void loadPseudocodeLines();
};

} // namespace core

#endif // STACK_QUEUE_VISUALIZER_SCREEN_HPP
