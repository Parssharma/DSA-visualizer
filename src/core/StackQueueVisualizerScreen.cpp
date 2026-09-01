#include "core/StackQueueVisualizerScreen.hpp"
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

StackQueueVisualizerScreen::StackQueueVisualizerScreen(sf::RenderWindow& window, StackQueueMode mode)
    : VisualizerBase(mode == StackQueueMode::Stack ? "Stack Visualizer" : "Queue Visualizer"),
      m_mode(mode),
      m_inputValue(15),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_isAnimating(false),
      m_animVal(0),
      m_animProgress(0.f),
      m_highlightedIdx(-1),
      m_isInputActive(false),
      m_cursorBlinkTime(0.f),
      m_capacity(7),
      m_totalOperations(0),
      m_pushCount(0),
      m_popCount(0),
      m_enqueueCount(0),
      m_dequeueCount(0),
      m_elapsedTime(0.f),
      m_showAlert(false),
      m_alertDisplayTime(0.f),
      m_activePseudocodeLine(-1) {
    
    initCommonUI(window);
    initUIComponents();
    configureMode(mode);
    onReset();
}

void StackQueueVisualizerScreen::initUIComponents() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(12);
    m_explanationText.setFillColor(colors.textSecondary);
    m_explanationText.setString("Initialize operations to begin.");

    // 1. Value adjusts [-] [Value: X] [+] [Random Value]
    m_decValBtn = std::make_unique<ui::Button>("-", sf::Vector2f(24.f, 24.f), 12);
    m_decValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_decValBtn->setCallback([this]() { adjustInputValue(-5); });

    m_incValBtn = std::make_unique<ui::Button>("+", sf::Vector2f(24.f, 24.f), 12);
    m_incValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_incValBtn->setCallback([this]() { adjustInputValue(5); });

    m_randValBtn = std::make_unique<ui::Button>("Random Value", sf::Vector2f(110.f, 24.f), 10);
    m_randValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_randValBtn->setCallback([this]() { generateRandomValue(); });

    m_inputFieldFrame = std::make_unique<ui::Button>("Value: 15", sf::Vector2f(100.f, 24.f), 10);
    m_inputFieldFrame->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_inputFieldFrame->setCallback([this]() {
        m_isInputActive = true;
        m_inputTextBuffer = "";
    });

    m_valLabelText.setFont(regularFont);
    m_valLabelText.setCharacterSize(11);
    m_valLabelText.setFillColor(colors.textPrimary);

    // 2. Action Operations
    if (m_mode == StackQueueMode::Stack) {
        std::vector<std::string> stackOps = {"Push", "Pop", "Peek", "Clear", "Random Stack"};
        for (size_t i = 0; i < stackOps.size(); ++i) {
            auto btn = std::make_unique<ui::Button>(stackOps[i], sf::Vector2f(100.f, 26.f), 11);
            btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
            
            if (i == 0) btn->setCallback([this]() { triggerPush(); });
            else if (i == 1) btn->setCallback([this]() { triggerPop(); });
            else if (i == 2) btn->setCallback([this]() { triggerPeek(); });
            else if (i == 3) btn->setCallback([this]() { triggerClear(); });
            else btn->setCallback([this]() { triggerGenRandom(); });

            m_opButtons.push_back(std::move(btn));
        }
    } else {
        std::vector<std::string> queueOps = {"Enqueue", "Dequeue", "Front", "Rear", "Clear", "Random Queue"};
        for (size_t i = 0; i < queueOps.size(); ++i) {
            auto btn = std::make_unique<ui::Button>(queueOps[i], sf::Vector2f(100.f, 26.f), 11);
            btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
            
            if (i == 0) btn->setCallback([this]() { triggerEnqueue(); });
            else if (i == 1) btn->setCallback([this]() { triggerDequeue(); });
            else if (i == 2) btn->setCallback([this]() { triggerFront(); });
            else if (i == 3) btn->setCallback([this]() { triggerRear(); });
            else if (i == 4) btn->setCallback([this]() { triggerClear(); });
            else btn->setCallback([this]() { triggerGenRandom(); });

            m_opButtons.push_back(std::move(btn));
        }
    }

    // 3. Live stats labels
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("STRUCTURE METRICS");
    m_statsTitleText.setCharacterSize(14);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsSizeText.setFont(regularFont);
    m_statsSizeText.setCharacterSize(12);
    m_statsSizeText.setFillColor(colors.textPrimary);

    m_statsCapText.setFont(regularFont);
    m_statsCapText.setCharacterSize(12);
    m_statsCapText.setFillColor(colors.textPrimary);

    m_statsTotalOpsText.setFont(regularFont);
    m_statsTotalOpsText.setCharacterSize(12);
    m_statsTotalOpsText.setFillColor(colors.textPrimary);

    m_statsPushEnqueueText.setFont(regularFont);
    m_statsPushEnqueueText.setCharacterSize(12);
    m_statsPushEnqueueText.setFillColor(colors.textPrimary);

    m_statsPopDequeueText.setFont(regularFont);
    m_statsPopDequeueText.setCharacterSize(12);
    m_statsPopDequeueText.setFillColor(colors.textPrimary);

    m_statsTimeText.setFont(regularFont);
    m_statsTimeText.setCharacterSize(12);
    m_statsTimeText.setFillColor(colors.textPrimary);

    // 4. Alert panel overlay (for overflow/underflow alerts)
    m_alertOverlay = std::make_unique<ui::Panel>(sf::Vector2f(320.f, 150.f), "Alert!");
    m_alertOverlay->setBackgroundColor(sf::Color(26, 26, 36, 242));
    m_alertOverlay->setBorderThickness(2.f);

    m_alertTitleText.setFont(boldFont);
    m_alertTitleText.setCharacterSize(16);
    m_alertTitleText.setFillColor(colors.danger);

    m_alertMsgText.setFont(regularFont);
    m_alertMsgText.setCharacterSize(12);
    m_alertMsgText.setFillColor(colors.textPrimary);

    m_alertResetBtn = std::make_unique<ui::Button>("Close", sf::Vector2f(90.f, 28.f), 11);
    m_alertResetBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_alertResetBtn->setCallback([this]() {
        m_showAlert = false;
    });
}

void StackQueueVisualizerScreen::updateUILayout() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    // Value input controls layout (row 1)
    m_decValBtn->setPosition(sf::Vector2f(startX, startY));
    m_inputFieldFrame->setPosition(sf::Vector2f(startX + 24.f + spacing, startY));
    m_incValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 100.f + spacing, startY));
    m_randValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 100.f + spacing + 24.f + spacing, startY));

    // Operations buttons layout (row 2)
    float opY = startY + 36.f;
    for (size_t i = 0; i < m_opButtons.size(); ++i) {
        m_opButtons[i]->setPosition(sf::Vector2f(startX + i * (100.f + spacing), opY));
    }

    // Explanation Y
    m_explanationText.setPosition(bounds.left + 15.f, opY + 35.f);

    // Live Stats right column layout
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 75.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsSizeText.setPosition(infoBounds.left, statsStartY + 22.f);
    m_statsCapText.setPosition(infoBounds.left, statsStartY + 40.f);
    m_statsTotalOpsText.setPosition(infoBounds.left, statsStartY + 58.f);
    m_statsPushEnqueueText.setPosition(infoBounds.left, statsStartY + 76.f);
    m_statsPopDequeueText.setPosition(infoBounds.left, statsStartY + 94.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 112.f);

    // Dynamic complexities push-downs
    m_theoryTitle.setPosition(infoBounds.left, statsStartY + 150.f);
    m_theoryBody.setPosition(infoBounds.left, statsStartY + 175.f);

    // Stacking pseudocode text rows
    float pseudocodeStartY = statsStartY + 280.f;
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        m_pseudocodeTexts[i].setPosition(infoBounds.left + 10.f, pseudocodeStartY + i * 16.f);
    }

    // Centering Alert overlay
    float ax = bounds.left + (bounds.width - 320.f) / 2.f;
    float ay = bounds.top + (bounds.height - 150.f) / 2.f;
    m_alertOverlay->setPosition(sf::Vector2f(ax, ay));

    sf::FloatRect abounds = m_alertOverlay->getContentBounds();
    m_alertTitleText.setPosition(abounds.left, abounds.top + 5.f);
    m_alertMsgText.setPosition(abounds.left, abounds.top + 32.f);
    m_alertResetBtn->setPosition(sf::Vector2f(abounds.left + (abounds.width - 90.f) / 2.f, abounds.top + 80.f));
}

void StackQueueVisualizerScreen::configureMode(StackQueueMode mode) {
    if (mode == StackQueueMode::Stack) {
        m_theoryTitle.setString("Stack LIFO Paradigm");
        m_theoryBody.setString("A Stack is a Last-In, First-Out (LIFO) abstract data type. Element insertions (Push) and deletions (Pop) occur at the Top. Commonly used for recursive trackings and backtracking.");
        m_complexityTitle.setString("Stack Complexities");
        m_timeComplexity.setString("Push / Pop: O(1) Time");
        m_spaceComplexity.setString("Capacity: O(N) Space");
    } else {
        m_theoryTitle.setString("Queue FIFO Paradigm");
        m_theoryBody.setString("A Queue is a First-In, First-Out (FIFO) abstract data type. Elements enter at the Rear (Enqueue) and leave at the Front (Dequeue). Commonly used for buffers, scheduling, and breadth-first search.");
        m_complexityTitle.setString("Queue Complexities");
        m_timeComplexity.setString("Enqueue / Dequeue: O(1) Time");
        m_spaceComplexity.setString("Capacity: O(N) Space");
    }

    loadPseudocodeLines();
}

void StackQueueVisualizerScreen::loadPseudocodeLines() {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    if (m_mode == StackQueueMode::Stack) {
        m_pseudocodeLines = {
            "procedure push(val):",
            "  if size == capacity: overflow()",
            "  top = top + 1; A[top] = val",
            "procedure pop():",
            "  if size == 0: underflow()",
            "  val = A[top]; top = top - 1"
        };
    } else {
        m_pseudocodeLines = {
            "procedure enqueue(val):",
            "  if size == capacity: overflow()",
            "  rear = (rear + 1) % size; A[rear] = val",
            "procedure dequeue():",
            "  if size == 0: underflow()",
            "  val = A[front]; front = (front + 1) % size"
        };
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
    updateUILayout();
}

// -------------------------------------------------------------
// VALUE ADJUSTS
// -------------------------------------------------------------
void StackQueueVisualizerScreen::adjustInputValue(int delta) {
    m_inputValue += delta;
    if (m_inputValue < 1) m_inputValue = 1;
    if (m_inputValue > 99) m_inputValue = 99;
}

void StackQueueVisualizerScreen::generateRandomValue() {
    m_inputValue = 5 + rand() % 90;
}

void StackQueueVisualizerScreen::commitValueInput() {
    m_isInputActive = false;
    if (!m_inputTextBuffer.empty()) {
        int val = std::atoi(m_inputTextBuffer.c_str());
        if (val >= 1 && val <= 99) {
            m_inputValue = val;
        }
    }
}

// -------------------------------------------------------------
// OPERATIONS RECORDING TRIGGERS
// -------------------------------------------------------------
void StackQueueVisualizerScreen::triggerPush() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Push operation.");
    addCompareStep(m_inputValue, 0, "Checking stack capacity: size (" + std::to_string(m_elements.size()) + ") vs capacity (" + std::to_string(m_capacity) + ")");
    
    if (m_elements.size() >= static_cast<size_t>(m_capacity)) {
        addAlertStep("Stack Overflow", "Stack size has reached maximum capacity (7). Push blocked.", 1, "Stack Overflow!");
    } else {
        addHighlightStep(1); // if size == capacity
        addSpawnNodeStep(m_inputValue, 2, "Top index increments. Spawning element " + std::to_string(m_inputValue) + " at entry.");
        addMoveNodeStep(m_inputValue, m_elements.size(), "push", 2, "Inserting element into stack index " + std::to_string(m_elements.size()));
        addCommitNodeStep(m_inputValue, 2, "Element committed. Top element is now: " + std::to_string(m_inputValue));
        addHighlightPointerStep(m_elements.size(), "top", 2, "Pointer top now points to new element.");
    }
}

void StackQueueVisualizerScreen::triggerPop() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Pop operation.");
    addCompareStep(-1, 3, "Checking stack capacity: is stack empty (size == 0)?");

    if (m_elements.empty()) {
        addAlertStep("Stack Underflow", "Stack is empty. Cannot pop.", 4, "Stack Underflow!");
    } else {
        addHighlightStep(4); // if size == 0
        int val = m_elements.back();
        addPopNodeStep(val, m_elements.size() - 1, "pop", 5, "Popping top element " + std::to_string(val) + " from stack.");
        addCommitNodeStep(-1, 5, "Popped element " + std::to_string(val) + " successfully. Stack top updated.");
        if (m_elements.size() > 1) {
            addHighlightPointerStep(m_elements.size() - 2, "top", 5, "Pointer top updated to index " + std::to_string(m_elements.size() - 2));
        } else {
            addHighlightPointerStep(-1, "top", 5, "Stack is now empty. top pointer is null.");
        }
    }
}

void StackQueueVisualizerScreen::triggerPeek() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    if (m_elements.empty()) {
        addAlertStep("Stack Empty", "Stack is empty. Cannot peek.", -1, "Stack is empty.");
    } else {
        int val = m_elements.back();
        addHighlightPointerStep(m_elements.size() - 1, "top", -1, "Peek: Top element is " + std::to_string(val) + " at index " + std::to_string(m_elements.size() - 1));
    }
}

void StackQueueVisualizerScreen::triggerEnqueue() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Enqueue operation.");
    addCompareStep(m_inputValue, 0, "Checking queue capacity: size (" + std::to_string(m_elements.size()) + ") vs capacity (" + std::to_string(m_capacity) + ")");

    if (m_elements.size() >= static_cast<size_t>(m_capacity)) {
        addAlertStep("Queue Overflow", "Queue size has reached maximum capacity (7). Enqueue blocked.", 1, "Queue Overflow!");
    } else {
        addHighlightStep(1); // if size == capacity
        addSpawnNodeStep(m_inputValue, 2, "Spawning element " + std::to_string(m_inputValue) + " at entry.");
        addMoveNodeStep(m_inputValue, m_elements.size(), "enqueue", 2, "Inserting element into queue rear slot " + std::to_string(m_elements.size()));
        addCommitNodeStep(m_inputValue, 2, "Element committed. Rear is now: " + std::to_string(m_inputValue));
        addHighlightPointerStep(m_elements.size(), "rear", 2, "Pointer rear updated.");
    }
}

void StackQueueVisualizerScreen::triggerDequeue() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Dequeue operation.");
    addCompareStep(-1, 3, "Checking queue capacity: size == 0?");

    if (m_elements.empty()) {
        addAlertStep("Queue Underflow", "Queue is empty. Cannot dequeue.", 4, "Queue Underflow!");
    } else {
        addHighlightStep(4);
        int val = m_elements[0];
        addPopNodeStep(val, 0, "dequeue", 5, "Dequeuing front element " + std::to_string(val) + " from queue front.");
        addCommitNodeStep(-1, 5, "Dequeued element " + std::to_string(val) + " successfully. Queue front elements shifted left.");
    }
}

void StackQueueVisualizerScreen::triggerFront() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    if (m_elements.empty()) {
        addAlertStep("Queue Empty", "Queue is empty. No front element.", -1, "Queue is empty.");
    } else {
        int val = m_elements[0];
        addHighlightPointerStep(0, "front", -1, "Front element is " + std::to_string(val) + " at index 0");
    }
}

void StackQueueVisualizerScreen::triggerRear() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    if (m_elements.empty()) {
        addAlertStep("Queue Empty", "Queue is empty. No rear element.", -1, "Queue is empty.");
    } else {
        int val = m_elements.back();
        addHighlightPointerStep(m_elements.size() - 1, "rear", -1, "Rear element is " + std::to_string(val) + " at index " + std::to_string(m_elements.size() - 1));
    }
}

void StackQueueVisualizerScreen::triggerClear() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addClearStep(-1, "Resetting structural elements.");
}

void StackQueueVisualizerScreen::triggerGenRandom() {
    m_elements.clear();
    int size = 2 + rand() % 4; // generate 2 to 5 elements
    for (int i = 0; i < size; ++i) {
        m_elements.push_back(10 + rand() % 85);
    }
    onReset();
}

// -------------------------------------------------------------
// PLAYBACK TICKS & ANIMATION delta
// -------------------------------------------------------------
void StackQueueVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_isAnimating = false;
    m_highlightedIdx = -1;
    m_highlightedPointerName = "";

    m_totalOperations = 0;
    m_pushCount = 0;
    m_popCount = 0;
    m_enqueueCount = 0;
    m_dequeueCount = 0;
    m_elapsedTime = 0.f;
    m_showAlert = false;

    m_explanationText.setString("Structure ready. Select operations above.");
    m_activePseudocodeLine = -1;

    if (m_window) {
        updateUILayout();
    }
}

void StackQueueVisualizerScreen::onSizeChanged(int newSize) {
    // Capacity remains fixed at 7 for screen visualization constraints
    onReset();
}

void StackQueueVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        
        m_isAnimating = false;
        m_activePseudocodeLine = -1;
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);

    // Setup coordinates panel width/height
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    float cellW = m_mode == StackQueueMode::Stack ? 140.f : 60.f;
    float cellH = m_mode == StackQueueMode::Stack ? 35.f : 45.f;
    
    // Stack dimensions
    float containerCenter = bounds.left + bounds.width / 2.f;
    float bottomY = bounds.top + bounds.height - 30.f;
    
    // Queue dimensions
    float queueW = 7 * 60.f + 6 * 10.f; // 480px
    float qStartX = bounds.left + (bounds.width - queueW) / 2.f;
    float qCenterY = bounds.top + bounds.height / 2.f - cellH / 2.f;

    switch (step.type) {
        case StackQueueStep::Type::Compare:
            m_highlightedIdx = -1;
            m_highlightedPointerName = "";
            m_isAnimating = false;
            break;
        case StackQueueStep::Type::SpawnNode:
            m_isAnimating = true;
            m_animVal = step.value;
            m_animProgress = 0.f;
            m_animType = "spawn";
            
            if (m_mode == StackQueueMode::Stack) {
                m_animStartPos = sf::Vector2f(containerCenter - cellW / 2.f, bounds.top + 5.f);
            } else {
                m_animStartPos = sf::Vector2f(bounds.left + bounds.width + 10.f, qCenterY);
            }
            m_animTargetPos = m_animStartPos;
            m_animCurrentPos = m_animStartPos;
            break;
        case StackQueueStep::Type::MoveNode:
            m_isAnimating = true;
            m_animVal = step.value;
            m_animProgress = 0.f;
            m_animType = step.explanation; // "push" or "enqueue"

            if (m_mode == StackQueueMode::Stack) {
                m_animStartPos = sf::Vector2f(containerCenter - cellW / 2.f, bounds.top + 5.f);
                m_animTargetPos = sf::Vector2f(containerCenter - cellW / 2.f, bottomY - (step.index + 1) * cellH);
            } else {
                m_animStartPos = sf::Vector2f(bounds.left + bounds.width + 10.f, qCenterY);
                m_animTargetPos = sf::Vector2f(qStartX + step.index * (60.f + 10.f), qCenterY);
            }
            m_animCurrentPos = m_animStartPos;
            break;
        case StackQueueStep::Type::CommitNode:
            m_isAnimating = false;
            m_totalOperations++;
            if (step.value != -1) {
                m_elements.push_back(step.value);
                if (m_mode == StackQueueMode::Stack) m_pushCount++;
                else m_enqueueCount++;
            } else {
                if (m_mode == StackQueueMode::Stack) {
                    if (!m_elements.empty()) m_elements.pop_back();
                    m_popCount++;
                } else {
                    if (!m_elements.empty()) m_elements.erase(m_elements.begin());
                    m_dequeueCount++;
                }
            }
            break;
        case StackQueueStep::Type::PopNode:
            m_isAnimating = true;
            m_animVal = step.value;
            m_animProgress = 0.f;
            m_animType = step.explanation; // "pop" or "dequeue"

            if (m_mode == StackQueueMode::Stack) {
                m_animStartPos = sf::Vector2f(containerCenter - cellW / 2.f, bottomY - (step.index + 1) * cellH);
                m_animTargetPos = sf::Vector2f(containerCenter - cellW / 2.f, bounds.top - cellH - 10.f);
            } else {
                m_animStartPos = sf::Vector2f(qStartX, qCenterY);
                m_animTargetPos = sf::Vector2f(bounds.left - cellW - 10.f, qCenterY);
            }
            m_animCurrentPos = m_animStartPos;
            break;
        case StackQueueStep::Type::HighlightLine:
            break;
        case StackQueueStep::Type::Explain:
            break;
        case StackQueueStep::Type::OverflowAlert:
        case StackQueueStep::Type::UnderflowAlert:
            m_showAlert = true;
            m_alertTitle = step.value == 1 ? "Overflow Warning!" : "Underflow Warning!";
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_totalOperations++;
            break;
        case StackQueueStep::Type::HighlightPointer:
            m_highlightedIdx = step.index;
            m_highlightedPointerName = step.explanation;
            m_isAnimating = false;
            break;
        case StackQueueStep::Type::Clear:
            m_elements.clear();
            m_isAnimating = false;
            m_totalOperations++;
            break;
    }

    m_currentStepIndex++;
}

void StackQueueVisualizerScreen::updateVisuals(float deltaTime) {
    if (m_window) {
        if (m_showAlert) {
            m_alertResetBtn->update(deltaTime, *m_window);
        } else {
            for (auto& btn : m_opButtons) btn->update(deltaTime, *m_window);
            m_decValBtn->update(deltaTime, *m_window);
            m_incValBtn->update(deltaTime, *m_window);
            m_randValBtn->update(deltaTime, *m_window);
            m_inputFieldFrame->update(deltaTime, *m_window);
        }

        if (m_isInputActive) {
            m_cursorBlinkTime += deltaTime;
        }
    }

    if (m_isPlaying && !m_showAlert) {
        m_elapsedTime += deltaTime;

        if (m_isAnimating) {
            // Speed factor
            m_animProgress += deltaTime * m_animationSpeed * 5.f;
            if (m_animProgress >= 1.f) {
                m_animProgress = 1.f;
                m_isAnimating = false;
                m_animCurrentPos = m_animTargetPos;
            } else {
                m_animCurrentPos = m_animStartPos + (m_animTargetPos - m_animStartPos) * m_animProgress;
            }
        } else {
            m_stepAccumulator += deltaTime * m_animationSpeed;
            float stepDelay = 0.5f; // playback step speed
            if (m_stepAccumulator >= stepDelay) {
                m_stepAccumulator = 0.f;
                onStep();
            }
        }
    }

    // Refresh live stats text labels
    m_statsSizeText.setString("Current Size: " + std::to_string(m_elements.size()));
    m_statsCapText.setString("Max Capacity: " + std::to_string(m_capacity));
    m_statsTotalOpsText.setString("Total Operations: " + std::to_string(m_totalOperations));
    
    if (m_mode == StackQueueMode::Stack) {
        m_statsPushEnqueueText.setString("Pushes: " + std::to_string(m_pushCount));
        m_statsPopDequeueText.setString("Pops: " + std::to_string(m_popCount));
    } else {
        m_statsPushEnqueueText.setString("Enqueues: " + std::to_string(m_enqueueCount));
        m_statsPopDequeueText.setString("Dequeues: " + std::to_string(m_dequeueCount));
    }

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Elapsed Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);

    // Sync input frame button text
    if (m_isInputActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_inputFieldFrame->setLabel(displayStr);
    } else {
        m_inputFieldFrame->setLabel("Value: " + std::to_string(m_inputValue));
    }
}

void StackQueueVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    if (m_showAlert) {
        m_alertResetBtn->handleEvent(event, window);
        return;
    }

    // Manual input typing buffer
    if (m_isInputActive && event.type == sf::Event::TextEntered) {
        sf::Uint32 code = event.text.unicode;
        if (code == 8) {
            if (!m_inputTextBuffer.empty()) m_inputTextBuffer.pop_back();
        } else if (code == 13) {
            commitValueInput();
        } else if (code >= '0' && code <= '9') {
            if (m_inputTextBuffer.size() < 2) {
                m_inputTextBuffer += static_cast<char>(code);
            }
        }
        return;
    }

    // Dismiss focus if click away
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f bounds = m_inputFieldFrame->getSize();
        sf::Vector2f pos = m_inputFieldFrame->getPosition();
        sf::FloatRect abounds(pos.x, pos.y, bounds.x, bounds.y);
        
        if (!abounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if (m_isInputActive) {
                commitValueInput();
            }
        }
    }

    for (auto& btn : m_opButtons) btn->handleEvent(event, window);
    m_decValBtn->handleEvent(event, window);
    m_incValBtn->handleEvent(event, window);
    m_randValBtn->handleEvent(event, window);
    m_inputFieldFrame->handleEvent(event, window);
}

void StackQueueVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");

    // 1. Draw controls
    window.draw(*m_decValBtn);
    window.draw(*m_incValBtn);
    window.draw(*m_randValBtn);
    window.draw(*m_inputFieldFrame);
    for (auto& btn : m_opButtons) window.draw(*btn);
    window.draw(m_explanationText);

    // 2. Draw Stack or Queue Structure
    float cellW = m_mode == StackQueueMode::Stack ? 140.f : 60.f;
    float cellH = m_mode == StackQueueMode::Stack ? 35.f : 45.f;
    float containerCenter = bounds.left + bounds.width / 2.f;
    float bottomY = bounds.top + bounds.height - 30.f;

    float queueW = 7 * 60.f + 6 * 10.f; // 480px
    float qStartX = bounds.left + (bounds.width - queueW) / 2.f;
    float qCenterY = bounds.top + bounds.height / 2.f - cellH / 2.f;

    if (m_mode == StackQueueMode::Stack) {
        // --- DRAW STACK ---
        // Stack container brackets
        sf::RectangleShape leftBorder(sf::Vector2f(4.f, m_capacity * cellH + 10.f));
        leftBorder.setPosition(containerCenter - cellW / 2.f - 8.f, bottomY - m_capacity * cellH - 5.f);
        leftBorder.setFillColor(colors.border);
        window.draw(leftBorder);

        sf::RectangleShape rightBorder(sf::Vector2f(4.f, m_capacity * cellH + 10.f));
        rightBorder.setPosition(containerCenter + cellW / 2.f + 4.f, bottomY - m_capacity * cellH - 5.f);
        rightBorder.setFillColor(colors.border);
        window.draw(rightBorder);

        sf::RectangleShape bottomBorder(sf::Vector2f(cellW + 20.f, 4.f));
        bottomBorder.setPosition(containerCenter - cellW / 2.f - 8.f, bottomY + 2.f);
        bottomBorder.setFillColor(colors.border);
        window.draw(bottomBorder);

        // Draw static stack elements
        for (size_t i = 0; i < m_elements.size(); ++i) {
            float cx = containerCenter - cellW / 2.f;
            float cy = bottomY - (i + 1) * cellH;

            ui::RoundedRectangleShape box(sf::Vector2f(cellW, cellH), 4.f, 4);
            box.setPosition(cx, cy);
            box.setFillColor(colors.cardBg);
            box.setOutlineThickness(1.5f);

            bool isHighlighted = m_highlightedIdx == static_cast<int>(i) && m_highlightedPointerName == "top";
            box.setOutlineColor(isHighlighted ? colors.success : colors.border);
            window.draw(box);

            sf::Text boxText;
            boxText.setFont(boldFont);
            boxText.setString(std::to_string(m_elements[i]));
            boxText.setCharacterSize(14);
            boxText.setFillColor(isHighlighted ? colors.success : colors.textPrimary);
            
            sf::FloatRect textBounds = boxText.getLocalBounds();
            boxText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            boxText.setPosition(cx + cellW / 2.f, cy + cellH / 2.f);
            window.draw(boxText);

            // Draw Pointer arrow pointing to top element
            if (i == m_elements.size() - 1) {
                sf::Text ptrText;
                ptrText.setFont(boldFont);
                ptrText.setString("<- TOP");
                ptrText.setCharacterSize(11);
                ptrText.setFillColor(colors.warning);
                ptrText.setPosition(cx + cellW + 15.f, cy + cellH / 2.f - 8.f);
                window.draw(ptrText);
            }
        }

        // Draw active animating node
        if (m_isAnimating && (m_animType == "push" || m_animType == "pop" || m_animType == "spawn")) {
            ui::RoundedRectangleShape box(sf::Vector2f(cellW, cellH), 4.f, 4);
            box.setPosition(m_animCurrentPos);
            box.setFillColor(colors.accent);
            box.setOutlineThickness(1.5f);
            box.setOutlineColor(colors.accentHover);
            window.draw(box);

            sf::Text boxText;
            boxText.setFont(boldFont);
            boxText.setString(std::to_string(m_animVal));
            boxText.setCharacterSize(14);
            boxText.setFillColor(colors.textPrimary);
            
            sf::FloatRect textBounds = boxText.getLocalBounds();
            boxText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            boxText.setPosition(m_animCurrentPos.x + cellW / 2.f, m_animCurrentPos.y + cellH / 2.f);
            window.draw(boxText);
        }

    } else {
        // --- DRAW QUEUE ---
        // Queue horizontal pipes
        sf::RectangleShape topPipe(sf::Vector2f(queueW + 20.f, 4.f));
        topPipe.setPosition(qStartX - 10.f, qCenterY - 6.f);
        topPipe.setFillColor(colors.border);
        window.draw(topPipe);

        sf::RectangleShape bottomPipe(sf::Vector2f(queueW + 20.f, 4.f));
        bottomPipe.setPosition(qStartX - 10.f, qCenterY + cellH + 2.f);
        bottomPipe.setFillColor(colors.border);
        window.draw(bottomPipe);

        // Draw static queue elements
        for (size_t i = 0; i < m_elements.size(); ++i) {
            float cx = qStartX + i * (cellW + 10.f);
            float cy = qCenterY;

            ui::RoundedRectangleShape box(sf::Vector2f(cellW, cellH), 4.f, 4);
            box.setPosition(cx, cy);
            box.setFillColor(colors.cardBg);
            box.setOutlineThickness(1.5f);

            bool isFront = (i == 0);
            bool isRear = (i == m_elements.size() - 1);
            
            bool isHighlighted = m_highlightedIdx == static_cast<int>(i) && 
                                 (m_highlightedPointerName == "front" || m_highlightedPointerName == "rear");

            box.setOutlineColor(isHighlighted ? colors.success : colors.border);
            window.draw(box);

            sf::Text boxText;
            boxText.setFont(boldFont);
            boxText.setString(std::to_string(m_elements[i]));
            boxText.setCharacterSize(14);
            boxText.setFillColor(isHighlighted ? colors.success : colors.textPrimary);
            
            sf::FloatRect textBounds = boxText.getLocalBounds();
            boxText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            boxText.setPosition(cx + cellW / 2.f, cy + cellH / 2.f);
            window.draw(boxText);

            // Draw Pointer labels below nodes
            if (isFront) {
                sf::Text frontText;
                frontText.setFont(boldFont);
                frontText.setString("FRONT");
                frontText.setCharacterSize(10);
                frontText.setFillColor(colors.warning);
                
                sf::FloatRect ftBounds = frontText.getLocalBounds();
                frontText.setOrigin(ftBounds.left + ftBounds.width / 2.f, 0.f);
                frontText.setPosition(cx + cellW / 2.f, cy + cellH + 10.f);
                window.draw(frontText);
            }
            if (isRear) {
                sf::Text rearText;
                rearText.setFont(boldFont);
                rearText.setString("REAR");
                rearText.setCharacterSize(10);
                rearText.setFillColor(colors.warning);

                sf::FloatRect rtBounds = rearText.getLocalBounds();
                rearText.setOrigin(rtBounds.left + rtBounds.width / 2.f, 0.f);
                rearText.setPosition(cx + cellW / 2.f, cy + cellH + 22.f);
                window.draw(rearText);
            }
        }

        // Draw active animating node
        if (m_isAnimating && (m_animType == "enqueue" || m_animType == "dequeue" || m_animType == "spawn")) {
            ui::RoundedRectangleShape box(sf::Vector2f(cellW, cellH), 4.f, 4);
            box.setPosition(m_animCurrentPos);
            box.setFillColor(colors.accent);
            box.setOutlineThickness(1.5f);
            box.setOutlineColor(colors.accentHover);
            window.draw(box);

            sf::Text boxText;
            boxText.setFont(boldFont);
            boxText.setString(std::to_string(m_animVal));
            boxText.setCharacterSize(14);
            boxText.setFillColor(colors.textPrimary);
            
            sf::FloatRect textBounds = boxText.getLocalBounds();
            boxText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            boxText.setPosition(m_animCurrentPos.x + cellW / 2.f, m_animCurrentPos.y + cellH / 2.f);
            window.draw(boxText);
        }
    }

    // 3. Draw statistics in Info panel
    window.draw(m_statsTitleText);
    window.draw(m_statsSizeText);
    window.draw(m_statsCapText);
    window.draw(m_statsTotalOpsText);
    window.draw(m_statsPushEnqueueText);
    window.draw(m_statsPopDequeueText);
    window.draw(m_statsTimeText);

    // 4. Render pseudocode texts
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

    // 5. Draw Alert Overlay
    if (m_showAlert) {
        window.draw(*m_alertOverlay);
        window.draw(m_alertTitleText);
        window.draw(m_alertMsgText);
        window.draw(*m_alertResetBtn);
    }
}

// -------------------------------------------------------------
// STEP INSERTER LOOPS
// -------------------------------------------------------------
void StackQueueVisualizerScreen::addCompareStep(int val, int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::Compare;
    step.value = val;
    step.index = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addSpawnNodeStep(int val, int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::SpawnNode;
    step.value = val;
    step.index = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addMoveNodeStep(int val, int idx, const std::string& animType, int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::MoveNode;
    step.value = val;
    step.index = idx;
    step.line = line;
    step.explanation = animType;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addCommitNodeStep(int val, int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::CommitNode;
    step.value = val;
    step.index = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addPopNodeStep(int val, int idx, const std::string& animType, int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::PopNode;
    step.value = val;
    step.index = idx;
    step.line = line;
    step.explanation = animType;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addHighlightStep(int line) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::HighlightLine;
    step.value = -1;
    step.index = -1;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addExplainStep(const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::Explain;
    step.value = -1;
    step.index = -1;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addAlertStep(const std::string& alertTitle, const std::string& alertMsg, int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = alertTitle.find("Overflow") != std::string::npos ? StackQueueStep::Type::OverflowAlert : StackQueueStep::Type::UnderflowAlert;
    step.value = alertTitle.find("Overflow") != std::string::npos ? 1 : 0;
    step.index = -1;
    step.line = line;
    step.explanation = alertMsg;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addHighlightPointerStep(int idx, const std::string& ptrName, int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::HighlightPointer;
    step.value = -1;
    step.index = idx;
    step.line = line;
    step.explanation = ptrName;
    m_steps.push_back(step);
}

void StackQueueVisualizerScreen::addClearStep(int line, const std::string& explanation) {
    StackQueueStep step;
    step.type = StackQueueStep::Type::Clear;
    step.value = -1;
    step.index = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

} // namespace core
