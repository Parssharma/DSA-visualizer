#include "core/LinkedListVisualizerScreen.hpp"
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

LinkedListVisualizerScreen::LinkedListVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("Linked List Visualizer"),
      m_listType(LinkedListType::Singly),
      m_inputValue(25),
      m_inputPos(0),
      m_isEducationalMode(true),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_activeIdx(-1),
      m_prevIdx(-1),
      m_nextIdx(-1),
      m_headIdx(-1),
      m_tailIdx(-1),
      m_isValActive(false),
      m_isPosActive(false),
      m_cursorBlinkTime(0.f),
      m_capacity(6),
      m_operationCount(0),
      m_traversalCount(0),
      m_insertCount(0),
      m_deleteCount(0),
      m_searchComparisons(0),
      m_elapsedTime(0.f),
      m_showAlert(false),
      m_activePseudocodeLine(-1) {
    
    srand(static_cast<unsigned int>(time(nullptr)));
    initCommonUI(window);
    initUIComponents();
    loadTypeMode(LinkedListType::Singly);
    triggerGenRandom();
}

void LinkedListVisualizerScreen::initUIComponents() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(13);
    m_explanationText.setFillColor(colors.textSecondary);

    // 1. Selector type tabs
    std::vector<std::string> typeNames = {"Singly List", "Doubly List", "Circular List"};
    for (size_t i = 0; i < typeNames.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(typeNames[i], sf::Vector2f(105.f, 26.f), 11);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        LinkedListType type = static_cast<LinkedListType>(i);
        btn->setCallback([this, type]() { loadTypeMode(type); });
        m_typeButtons.push_back(std::move(btn));
    }

    // 2. Adjusters & Educational Mode
    m_decValBtn = std::make_unique<ui::Button>("-", sf::Vector2f(24.f, 24.f), 12);
    m_decValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_decValBtn->setCallback([this]() { adjustVal(-5); });

    m_incValBtn = std::make_unique<ui::Button>("+", sf::Vector2f(24.f, 24.f), 12);
    m_incValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_incValBtn->setCallback([this]() { adjustVal(5); });

    m_randValBtn = std::make_unique<ui::Button>("Random", sf::Vector2f(65.f, 24.f), 10);
    m_randValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_randValBtn->setCallback([this]() { pickRandomValue(); });

    m_valInputFrame = std::make_unique<ui::Button>("Value: 25", sf::Vector2f(90.f, 24.f), 10);
    m_valInputFrame->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_valInputFrame->setCallback([this]() {
        m_isValActive = true;
        m_isPosActive = false;
        m_inputTextBuffer = "";
    });

    m_decPosBtn = std::make_unique<ui::Button>("-", sf::Vector2f(24.f, 24.f), 12);
    m_decPosBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_decPosBtn->setCallback([this]() { adjustPos(-1); });

    m_incPosBtn = std::make_unique<ui::Button>("+", sf::Vector2f(24.f, 24.f), 12);
    m_incPosBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_incPosBtn->setCallback([this]() { adjustPos(1); });

    m_posInputFrame = std::make_unique<ui::Button>("Pos: 0", sf::Vector2f(75.f, 24.f), 10);
    m_posInputFrame->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_posInputFrame->setCallback([this]() {
        m_isPosActive = true;
        m_isValActive = false;
        m_inputTextBuffer = "";
    });

    m_eduModeBtn = std::make_unique<ui::Button>("Edu Mode: ON", sf::Vector2f(105.f, 24.f), 10);
    m_eduModeBtn->setColors(colors.success, colors.accentHover, colors.textPrimary);
    m_eduModeBtn->setCallback([this]() {
        m_isEducationalMode = !m_isEducationalMode;
        const auto& colors = ThemeManager::getInstance().getColors();
        if (m_isEducationalMode) {
            m_eduModeBtn->setLabel("Edu Mode: ON");
            m_eduModeBtn->setColors(colors.success, colors.accentHover, colors.textPrimary);
        } else {
            m_eduModeBtn->setLabel("Edu Mode: OFF");
            m_eduModeBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        }
    });

    // 3. Actions Row 1 (Insertions & Gen)
    std::vector<std::string> opNames1 = {"Insert Head", "Insert Tail", "Insert Pos", "Gen Random"};
    for (size_t i = 0; i < opNames1.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(opNames1[i], sf::Vector2f(110.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        if (i == 0) btn->setCallback([this]() { triggerInsertHead(); });
        else if (i == 1) btn->setCallback([this]() { triggerInsertTail(); });
        else if (i == 2) btn->setCallback([this]() { triggerInsertPos(); });
        else btn->setCallback([this]() { triggerGenRandom(); });
        
        m_opButtons.push_back(std::move(btn));
    }

    // 4. Actions Row 2 (Deletions, traversals)
    std::vector<std::string> opNames2 = {"Del Head", "Del Tail", "Del Pos", "Del Val", "Search", "Traverse", "Reverse", "Clear"};
    for (size_t i = 0; i < opNames2.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(opNames2[i], sf::Vector2f(85.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        if (i == 0) btn->setCallback([this]() { triggerDeleteHead(); });
        else if (i == 1) btn->setCallback([this]() { triggerDeleteTail(); });
        else if (i == 2) btn->setCallback([this]() { triggerDeletePos(); });
        else if (i == 3) btn->setCallback([this]() { triggerDeleteVal(); });
        else if (i == 4) btn->setCallback([this]() { triggerSearch(); });
        else if (i == 5) btn->setCallback([this]() { triggerTraverse(); });
        else if (i == 6) btn->setCallback([this]() { triggerReverse(); });
        else btn->setCallback([this]() { triggerClear(); });

        m_opButtons.push_back(std::move(btn));
    }

    // 5. Statistics Texts
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("LIST METRICS");
    m_statsTitleText.setCharacterSize(15);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsSizeText.setFont(regularFont);
    m_statsSizeText.setCharacterSize(13);
    m_statsSizeText.setFillColor(colors.textPrimary);

    m_statsOpsText.setFont(regularFont);
    m_statsOpsText.setCharacterSize(13);
    m_statsOpsText.setFillColor(colors.textPrimary);

    m_statsTraversalsText.setFont(regularFont);
    m_statsTraversalsText.setCharacterSize(13);
    m_statsTraversalsText.setFillColor(colors.textPrimary);

    m_statsInsText.setFont(regularFont);
    m_statsInsText.setCharacterSize(13);
    m_statsInsText.setFillColor(colors.textPrimary);

    m_statsDelText.setFont(regularFont);
    m_statsDelText.setCharacterSize(13);
    m_statsDelText.setFillColor(colors.textPrimary);

    m_statsCompsText.setFont(regularFont);
    m_statsCompsText.setCharacterSize(13);
    m_statsCompsText.setFillColor(colors.textPrimary);

    m_statsTimeText.setFont(regularFont);
    m_statsTimeText.setCharacterSize(13);
    m_statsTimeText.setFillColor(colors.textPrimary);

    // 6. Action History log
    m_historyTitleText.setFont(boldFont);
    m_historyTitleText.setString("ACTION HISTORY LOG");
    m_historyTitleText.setCharacterSize(15);
    m_historyTitleText.setFillColor(colors.accent);

    for (int i = 0; i < 5; ++i) {
        sf::Text txt;
        txt.setFont(regularFont);
        txt.setCharacterSize(12);
        txt.setFillColor(colors.textSecondary);
        txt.setString("-");
        m_historyTexts.push_back(txt);
    }

    // 7. Alert overlay
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

void LinkedListVisualizerScreen::updateUILayouts() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    // Row 1
    for (size_t i = 0; i < m_typeButtons.size(); ++i) {
        m_typeButtons[i]->setPosition(sf::Vector2f(startX + i * (105.f + spacing), startY));
    }

    // Row 2
    float row2Y = startY + 36.f;
    m_decValBtn->setPosition(sf::Vector2f(startX, row2Y));
    m_valInputFrame->setPosition(sf::Vector2f(startX + 24.f + spacing, row2Y));
    m_incValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing, row2Y));
    m_randValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing + 24.f + spacing, row2Y));

    float offsetPos = startX + 24.f + spacing + 90.f + spacing + 24.f + spacing + 65.f + 30.f;
    m_decPosBtn->setPosition(sf::Vector2f(offsetPos, row2Y));
    m_posInputFrame->setPosition(sf::Vector2f(offsetPos + 24.f + spacing, row2Y));
    m_incPosBtn->setPosition(sf::Vector2f(offsetPos + 24.f + spacing + 75.f + spacing, row2Y));
    m_eduModeBtn->setPosition(sf::Vector2f(offsetPos + 24.f + spacing + 75.f + spacing + 24.f + spacing + 10.f, row2Y));

    // Row 3 (Push/Insert operations)
    float row3Y = row2Y + 34.f;
    for (size_t i = 0; i < 4; ++i) {
        m_opButtons[i]->setPosition(sf::Vector2f(startX + i * (110.f + spacing), row3Y));
    }

    // Row 4 (Deletions/traversals operations)
    float row4Y = row3Y + 32.f;
    for (size_t i = 4; i < m_opButtons.size(); ++i) {
        m_opButtons[i]->setPosition(sf::Vector2f(startX + (i - 4) * (85.f + spacing), row4Y));
    }

    // Explanation text Y
    m_explanationText.setPosition(bounds.left + 15.f, row4Y + 35.f);

    // Live Stats right column layout
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 70.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsSizeText.setPosition(infoBounds.left, statsStartY + 22.f);
    m_statsOpsText.setPosition(infoBounds.left, statsStartY + 40.f);
    m_statsTraversalsText.setPosition(infoBounds.left, statsStartY + 58.f);
    m_statsInsText.setPosition(infoBounds.left, statsStartY + 76.f);
    m_statsDelText.setPosition(infoBounds.left, statsStartY + 94.f);
    m_statsCompsText.setPosition(infoBounds.left, statsStartY + 112.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 130.f);

    // Live History Log
    float historyStartY = statsStartY + 160.f;
    m_historyTitleText.setPosition(infoBounds.left, historyStartY);
    for (size_t i = 0; i < m_historyTexts.size(); ++i) {
        m_historyTexts[i].setPosition(infoBounds.left + 5.f, historyStartY + 22.f + i * 16.f);
    }

    // Complexity Theory details
    m_theoryTitle.setPosition(infoBounds.left, historyStartY + 120.f);
    m_theoryBody.setPosition(infoBounds.left, historyStartY + 145.f);

    // Stacking pseudocode rows
    float pseudocodeStartY = historyStartY + 265.f;
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        m_pseudocodeTexts[i].setPosition(infoBounds.left + 10.f, pseudocodeStartY + i * 16.f);
    }

    // Centering Alert Panel
    float ax = bounds.left + (bounds.width - 320.f) / 2.f;
    float ay = bounds.top + (bounds.height - 150.f) / 2.f;
    m_alertOverlay->setPosition(sf::Vector2f(ax, ay));

    sf::FloatRect abounds = m_alertOverlay->getContentBounds();
    m_alertTitleText.setPosition(abounds.left, abounds.top + 5.f);
    m_alertMsgText.setPosition(abounds.left, abounds.top + 32.f);
    m_alertResetBtn->setPosition(sf::Vector2f(abounds.left + (abounds.width - 90.f) / 2.f, abounds.top + 80.f));
}

void LinkedListVisualizerScreen::loadTypeMode(LinkedListType type) {
    m_listType = type;
    m_isPlaying = false;
    m_playPauseButton->setLabel("Play");

    const auto& colors = ThemeManager::getInstance().getColors();
    m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);

    // Tabs update
    for (size_t i = 0; i < m_typeButtons.size(); ++i) {
        if (i == static_cast<size_t>(type)) {
            m_typeButtons[i]->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        } else {
            m_typeButtons[i]->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        }
    }

    // Theory complex details
    switch (type) {
        case LinkedListType::Singly:
            m_theoryTitle.setString("Singly Linked List");
            m_theoryBody.setString("A Singly Linked List consists of nodes where each node contains a data field and a reference ('next') to the next node in sequence. Traversal is only forward.");
            break;
        case LinkedListType::Doubly:
            m_theoryTitle.setString("Doubly Linked List");
            m_theoryBody.setString("A Doubly Linked List nodes contain data and two pointer fields ('prev' and 'next'), referencing the previous and next nodes. Allows bi-directional traversal.");
            break;
        case LinkedListType::Circular:
            m_theoryTitle.setString("Circular Linked List");
            m_theoryBody.setString("In a Circular Linked List, the next pointer of the last node points back to the first node, forming a closed loop. No null endpoints exist.");
            break;
    }

    loadPseudocode(type);
    onReset();
}

void LinkedListVisualizerScreen::loadPseudocode(LinkedListType type) {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    switch (type) {
        case LinkedListType::Singly:
            m_pseudocodeLines = {
                "insertHead(val):",
                "  new = Node(val); new.next = head",
                "  head = new",
                "deleteHead():",
                "  temp = head; head = head.next",
                "  free temp"
            };
            break;
        case LinkedListType::Doubly:
            m_pseudocodeLines = {
                "insertHead(val):",
                "  new = Node(val); new.next = head",
                "  if head != null: head.prev = new",
                "  head = new",
                "deleteHead():",
                "  temp = head; head = head.next",
                "  if head != null: head.prev = null"
            };
            break;
        case LinkedListType::Circular:
            m_pseudocodeLines = {
                "insertHead(val):",
                "  new = Node(val); new.next = head",
                "  tail = getTail(); tail.next = new",
                "  head = new",
                "deleteHead():",
                "  tail = getTail(); head = head.next",
                "  tail.next = head"
            };
            break;
    }

    sf::Font& monoFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");
    const auto& colors = ThemeManager::getInstance().getColors();

    for (const auto& line : m_pseudocodeLines) {
        sf::Text text;
        text.setFont(monoFont);
        text.setString(line);
        text.setCharacterSize(12);
        text.setFillColor(colors.textSecondary);
        m_pseudocodeTexts.push_back(text);
    }

    m_activePseudocodeLine = -1;
    updateUILayouts();
}

// -------------------------------------------------------------
// VALUE ADJUSTS
// -------------------------------------------------------------
void LinkedListVisualizerScreen::adjustVal(int delta) {
    m_inputValue += delta;
    if (m_inputValue < 1) m_inputValue = 1;
    if (m_inputValue > 99) m_inputValue = 99;
}

void LinkedListVisualizerScreen::adjustPos(int delta) {
    m_inputPos += delta;
    if (m_inputPos < 0) m_inputPos = 0;
    if (m_inputPos > static_cast<int>(m_nodes.size())) m_inputPos = m_nodes.size();
}

void LinkedListVisualizerScreen::pickRandomValue() {
    m_inputValue = 5 + rand() % 90;
}

void LinkedListVisualizerScreen::commitInputText() {
    m_isValActive = false;
    m_isPosActive = false;
    if (!m_inputTextBuffer.empty()) {
        int val = std::atoi(m_inputTextBuffer.c_str());
        if (m_isValActive) {
            if (val >= 1 && val <= 99) m_inputValue = val;
        } else {
            if (val >= 0 && val <= static_cast<int>(m_nodes.size())) m_inputPos = val;
        }
    }
}

void LinkedListVisualizerScreen::pushHistoryLog(const std::string& log) {
    m_historyList.push_back(log);
    if (m_historyList.size() > 5) {
        m_historyList.erase(m_historyList.begin());
    }
    for (size_t i = 0; i < m_historyTexts.size(); ++i) {
        if (i < m_historyList.size()) {
            m_historyTexts[i].setString(m_historyList[i]);
        } else {
            m_historyTexts[i].setString("-");
        }
    }
}

// -------------------------------------------------------------
// SIMULATION CONTROL
// -------------------------------------------------------------
void LinkedListVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_activeIdx = -1;
    m_prevIdx = -1;
    m_nextIdx = -1;
    
    if (!m_nodes.empty()) {
        m_headIdx = 0;
        m_tailIdx = m_nodes.size() - 1;
    } else {
        m_headIdx = -1;
        m_tailIdx = -1;
    }

    m_isValActive = false;
    m_isPosActive = false;
    m_showAlert = false;

    // Reset Pointer Arrow Current coordinates to node start centers to avoid visual jumps
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    float startX = bounds.left + 40.f;
    float centerY = bounds.top + 180.f + 22.f;

    for (int i = 0; i < 7; ++i) {
        float cx = startX + i * 110.f;
        m_nextArrowCurrent[i] = sf::Vector2f(cx + 65.f, centerY);
        m_prevArrowCurrent[i] = sf::Vector2f(cx, centerY);
        m_nextArrowTargets[i] = m_nextArrowCurrent[i];
        m_prevArrowTargets[i] = m_prevArrowCurrent[i];
    }

    m_explanationText.setString("List ready. Select operations above.");
    m_activePseudocodeLine = -1;

    if (m_window) {
        updateUILayouts();
    }
}

void LinkedListVisualizerScreen::onSizeChanged(int newSize) {
    // Linked List constraints capacity = 6
    onReset();
}

void LinkedListVisualizerScreen::triggerGenRandom() {
    m_nodes.clear();
    m_nodeAddresses.clear();
    m_historyList.clear();

    int size = 3 + rand() % 3; // 3 to 5 nodes
    for (int i = 0; i < size; ++i) {
        m_nodes.push_back(10 + rand() % 85);
        
        char addr[16];
        std::sprintf(addr, "0x%04X", 0x1A40 + i * 8);
        m_nodeAddresses.push_back(addr);
    }
    
    pushHistoryLog("Generate Random List of size " + std::to_string(size));
    onReset();
}

// -------------------------------------------------------------
// LIST ACTIONS RECORDEES
// -------------------------------------------------------------
void LinkedListVisualizerScreen::triggerInsertHead() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Insert at Head.");
    addCompareStep(m_inputValue, 0, "Checking max capacity limit...");

    if (m_nodes.size() >= static_cast<size_t>(m_capacity)) {
        addFailAlertStep("List Overflow", "List capacity has reached max limits (6). Insert blocked.", -1, "Insert failed!");
    } else {
        addHighlightStep(1); // new = Node(val); new.next = head
        addSpawnNodeStep(m_inputValue, 1, "Spawned node (" + std::to_string(m_inputValue) + ") with memory address.");
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Concept: We assign next pointer of new node to target the current Head node.");
        }
        addPointerRedirectStep(-2, 0, "next", 1, "Redirecting new node pointer to target current Head.");
        addHighlightStep(2); // head = new
        addCommitNodeStep(m_inputValue, 2, "Insert at Head completed successfully.");
    }
}

void LinkedListVisualizerScreen::triggerInsertTail() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Insert at Tail.");
    addCompareStep(m_inputValue, -1, "Checking max capacity limit...");

    if (m_nodes.size() >= static_cast<size_t>(m_capacity)) {
        addFailAlertStep("List Overflow", "List capacity has reached max limits (6). Insert blocked.", -1, "Insert failed!");
    } else {
        if (m_nodes.empty()) {
            addSpawnNodeStep(m_inputValue, 0, "List empty. Spawning head node.");
            addCommitNodeStep(m_inputValue, 0, "Insert Head completed.");
        } else {
            // Traverse to tail
            int tailIdx = m_nodes.size() - 1;
            for (int i = 0; i <= tailIdx; ++i) {
                addTraversalStep(i, -1, "Traversing list... current index: " + std::to_string(i));
                if (m_isEducationalMode && i < tailIdx) {
                    addExplainStep("[Teacher] Traversal: Dereferencing current node next pointer to advance pointer.");
                }
            }
            addSpawnNodeStep(m_inputValue, -1, "Reached tail node. Spawning new tail node.");
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Concept: Reconnect tail node's next pointer to target the new tail node.");
            }
            addPointerRedirectStep(tailIdx, -2, "next", -1, "Redirecting tail next pointer to new node.");
            addCommitNodeStep(m_inputValue, -1, "Insert at Tail completed.");
        }
    }
}

void LinkedListVisualizerScreen::triggerInsertPos() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Insert at Position " + std::to_string(m_inputPos));

    if (m_nodes.size() >= static_cast<size_t>(m_capacity)) {
        addFailAlertStep("List Overflow", "List capacity reached.", -1, "Insert blocked.");
        return;
    }
    if (m_inputPos < 0 || m_inputPos > static_cast<int>(m_nodes.size())) {
        addFailAlertStep("Invalid Position", "Position index is out of list size bounds.", -1, "Invalid Position!");
        return;
    }

    if (m_inputPos == 0) {
        triggerInsertHead();
    } else if (m_inputPos == static_cast<int>(m_nodes.size())) {
        triggerInsertTail();
    } else {
        // General position insertion
        int targetIdx = m_inputPos;
        for (int i = 0; i < targetIdx; ++i) {
            addTraversalStep(i, -1, "Traversing nodes... index: " + std::to_string(i));
        }
        
        addSpawnNodeStep(m_inputValue, -1, "Reached position. Spawning new node.");
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Concept: 1. Set new node next pointer to point to current node at position Y. 2. Set previous node next pointer to point to new node.");
        }
        addPointerRedirectStep(-2, targetIdx, "next", -1, "Setting new node next pointer to target index " + std::to_string(targetIdx));
        addPointerRedirectStep(targetIdx - 1, -2, "next", -1, "Setting preceding node next pointer to target new node.");
        addCommitNodeStep(m_inputValue, -1, "Insert Position completed.");
    }
}

void LinkedListVisualizerScreen::triggerDeleteHead() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Delete Head.");
    if (m_nodes.empty()) {
        addFailAlertStep("Empty List", "List is empty. Cannot delete.", -1, "Delete failed!");
    } else {
        addHighlightStep(3); // deleteHead()
        addSetPointersStep(-1, 0, 1, 4, "Setting temp pointer to current Head node.");
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Concept: We move the Head pointer to point to the second node, then free the old Head node.");
        }
        addPointerRedirectStep(0, -1, "head", 4, "Redirecting Head pointer to next node.");
        addPopNodeStep(m_nodes[0], 0, 5, "Freeing old Head node.");
        addCommitNodeStep(-1, 5, "Delete Head completed successfully.");
    }
}

void LinkedListVisualizerScreen::triggerDeleteTail() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Delete Tail.");
    if (m_nodes.empty()) {
        addFailAlertStep("Empty List", "List is empty. Cannot delete.", -1, "Delete failed!");
    } else if (m_nodes.size() == 1) {
        triggerDeleteHead();
    } else {
        int tailIdx = m_nodes.size() - 1;
        for (int i = 0; i < tailIdx; ++i) {
            addTraversalStep(i, -1, "Traversing list to tail... index: " + std::to_string(i));
        }
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Concept: Redirect the next pointer of the second-to-last node to NULL (or first node if Circular), then free tail.");
        }
        addPointerRedirectStep(tailIdx - 1, -1, "next", -1, "Redirecting second-to-last node next pointer to NULL.");
        addPopNodeStep(m_nodes[tailIdx], tailIdx, -1, "Freeing tail node.");
        addCommitNodeStep(-1, -1, "Delete Tail completed successfully.");
    }
}

void LinkedListVisualizerScreen::triggerDeletePos() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Delete at Position " + std::to_string(m_inputPos));

    if (m_nodes.empty()) {
        addFailAlertStep("Empty List", "List is empty.", -1, "Delete failed!");
        return;
    }
    if (m_inputPos < 0 || m_inputPos >= static_cast<int>(m_nodes.size())) {
        addFailAlertStep("Invalid Position", "Position index is out of list size bounds.", -1, "Invalid Position!");
        return;
    }

    if (m_inputPos == 0) {
        triggerDeleteHead();
    } else if (m_inputPos == static_cast<int>(m_nodes.size() - 1)) {
        triggerDeleteTail();
    } else {
        int targetIdx = m_inputPos;
        for (int i = 0; i <= targetIdx; ++i) {
            addTraversalStep(i, -1, "Traversing nodes... index: " + std::to_string(i));
        }
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Concept: Connect the preceding node's next pointer directly to target node's next pointer, bypass target node, then free target.");
        }
        addPointerRedirectStep(targetIdx - 1, targetIdx + 1, "next", -1, "Bypassing index " + std::to_string(targetIdx) + ". Connecting preceding node next pointer to index " + std::to_string(targetIdx + 1));
        addPopNodeStep(m_nodes[targetIdx], targetIdx, -1, "Freeing node at index " + std::to_string(targetIdx));
        addCommitNodeStep(-1, -1, "Delete Position completed.");
    }
}

void LinkedListVisualizerScreen::triggerDeleteVal() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Delete by Value " + std::to_string(m_inputValue));

    if (m_nodes.empty()) {
        addFailAlertStep("Empty List", "List is empty.", -1, "Delete failed!");
        return;
    }

    // Find index of value
    auto it = std::find(m_nodes.begin(), m_nodes.end(), m_inputValue);
    if (it == m_nodes.end()) {
        addFailAlertStep("Value Not Found", "Target value " + std::to_string(m_inputValue) + " is not in the list.", -1, "Delete failed!");
    } else {
        int idx = std::distance(m_nodes.begin(), it);
        m_inputPos = idx;
        triggerDeletePos();
    }
}

void LinkedListVisualizerScreen::triggerTraverse() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Traversal.");
    if (m_nodes.empty()) {
        addFailAlertStep("Empty List", "List is empty. Cannot traverse.", -1, "Traversal empty.");
    } else {
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            addTraversalStep(i, -1, "Traversing Node index: " + std::to_string(i) + ", value: " + std::to_string(m_nodes[i]));
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Traversal: Read value " + std::to_string(m_nodes[i]) + " at address " + m_nodeAddresses[i] + ". Following next pointer reference.");
            }
        }
        addSuccessAlertStep("Traversal Completed", "All elements traversed successfully.", -1, "Traversal finished.");
    }
}

void LinkedListVisualizerScreen::triggerSearch() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Target Search: " + std::to_string(m_inputValue));
    if (m_nodes.empty()) {
        addFailAlertStep("Empty List", "List is empty. Search failed.", -1, "Search failed.");
    } else {
        bool found = false;
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            addCompareStep(i, -1, "Comparing node value " + std::to_string(m_nodes[i]) + " with search target " + std::to_string(m_inputValue));
            if (m_nodes[i] == m_inputValue) {
                addSuccessAlertStep("Search Found", "Target element " + std::to_string(m_inputValue) + " found at index " + std::to_string(i), -1, "Search Found!");
                found = true;
                break;
            }
            if (m_isEducationalMode && i < m_nodes.size() - 1) {
                addExplainStep("[Teacher] Search Mismatch. Advancing next pointer to index " + std::to_string(i+1));
            }
        }
        if (!found) {
            addFailAlertStep("Search Not Found", "Target " + std::to_string(m_inputValue) + " is not in the list.", -1, "Search Not Found.");
        }
    }
}

void LinkedListVisualizerScreen::triggerReverse() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Reverse List.");
    if (m_nodes.empty()) {
        addFailAlertStep("Empty List", "List is empty.", -1, "Reverse failed.");
    } else if (m_nodes.size() == 1) {
        addSuccessAlertStep("Reverse Completed", "List reversed successfully.", -1, "Reverse finished.");
    } else {
        int n = m_nodes.size();
        addSetPointersStep(-1, 0, 1, -1, "Setting prev = NULL, curr = Head, next = current.next");
        
        for (int i = 0; i < n; ++i) {
            addTraversalStep(i, -1, "Reversing pointers at index " + std::to_string(i));
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Reverse step: 1. Save curr.next into temp. 2. Redirect curr.next backwards to prev. 3. Shift prev to curr, and curr to temp.");
            }
            // Redirect node i next pointer backwards to point to node i-1
            addPointerRedirectStep(i, i - 1, "next", -1, "Redirecting index " + std::to_string(i) + " next pointer to point back to " + std::to_string(i - 1));
        }
        addSuccessAlertStep("Reverse Completed", "List reversed successfully. Head pointer updated.", -1, "Reverse finished.");
    }
}

void LinkedListVisualizerScreen::triggerClear() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addClearStep(-1, "Resetting list elements.");
}

// -------------------------------------------------------------
// PLAYBACK STEP ENGINE
// -------------------------------------------------------------
void LinkedListVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        
        m_activeIdx = -1;
        m_prevIdx = -1;
        m_nextIdx = -1;
        m_activePseudocodeLine = -1;
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);
    pushHistoryLog(step.explanation);

    // Setup arrow coordinates panel width/height
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    float startX = bounds.left + 40.f;
    float centerY = bounds.top + 180.f + 22.f;
    float containerCenter = bounds.left + bounds.width / 2.f;

    switch (step.type) {
        case ListStep::Type::Compare:
            m_searchComparisons++;
            m_activeIdx = step.index1;
            m_prevIdx = -1;
            m_nextIdx = -1;
            break;
        case ListStep::Type::TraversalHighlight:
            m_activeIdx = step.index1;
            if (step.index1 > 0) m_prevIdx = step.index1 - 1;
            else m_prevIdx = -1;
            if (step.index1 < static_cast<int>(m_nodes.size() - 1)) m_nextIdx = step.index1 + 1;
            else m_nextIdx = -1;
            break;
        case ListStep::Type::SetPointers:
            m_prevIdx = step.index1;
            m_activeIdx = step.index2;
            m_nextIdx = step.value;
            break;
        case ListStep::Type::PointerRedirect:
            // Pointer redirect target setting (visual slide pointers animation)
            if (step.index1 >= 0 && step.index1 < 7) {
                if (step.index2 >= 0 && step.index2 < 7) {
                    float targetX = startX + step.index2 * 110.f;
                    m_nextArrowTargets[step.index1] = sf::Vector2f(targetX, centerY);
                } else if (step.index2 == -1) {
                    // Points to NULL (pointing downward)
                    m_nextArrowTargets[step.index1] = sf::Vector2f(startX + step.index1 * 110.f + 65.f, centerY + 40.f);
                } else if (step.index2 == -2) {
                    // Points to newly spawned node area
                    m_nextArrowTargets[step.index1] = sf::Vector2f(containerCenter - 30.f, bounds.top + 180.f);
                }
            }
            break;
        case ListStep::Type::SpawnNode:
            break;
        case ListStep::Type::PopNode:
            break;
        case ListStep::Type::Explain:
            break;
        case ListStep::Type::SuccessAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().success);
            m_operationCount++;
            if (m_steps.back().explanation.find("Reverse") != std::string::npos) {
                // Perform actual reverse on committed array
                std::reverse(m_nodes.begin(), m_nodes.end());
                std::reverse(m_nodeAddresses.begin(), m_nodeAddresses.end());
            }
            break;
        case ListStep::Type::FailAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().danger);
            break;
        case ListStep::Type::CommitNode:
            m_operationCount++;
            if (step.value != -1) {
                m_insertCount++;
                // Commit Node inserting Head/Tail/Pos
                int pos = m_inputPos;
                if (m_steps[0].explanation.find("Head") != std::string::npos) pos = 0;
                else if (m_steps[0].explanation.find("Tail") != std::string::npos) pos = m_nodes.size();
                
                m_nodes.insert(m_nodes.begin() + pos, step.value);
                
                char addr[16];
                std::sprintf(addr, "0x%04X", 0x1A40 + rand() % 500);
                m_nodeAddresses.insert(m_nodeAddresses.begin() + pos, addr);
            } else {
                m_deleteCount++;
                // Commit Node deleting Head/Tail/Pos
                int pos = m_inputPos;
                if (m_steps[0].explanation.find("Head") != std::string::npos) pos = 0;
                else if (m_steps[0].explanation.find("Tail") != std::string::npos) pos = m_nodes.size() - 1;

                if (pos >= 0 && pos < static_cast<int>(m_nodes.size())) {
                    m_nodes.erase(m_nodes.begin() + pos);
                    m_nodeAddresses.erase(m_nodeAddresses.begin() + pos);
                }
            }
            
            // Re-eval Head and Tail pointers
            if (!m_nodes.empty()) {
                m_headIdx = 0;
                m_tailIdx = m_nodes.size() - 1;
            } else {
                m_headIdx = -1;
                m_tailIdx = -1;
            }
            break;
        case ListStep::Type::Clear:
            m_operationCount++;
            m_nodes.clear();
            m_nodeAddresses.clear();
            m_headIdx = -1;
            m_tailIdx = -1;
            break;
    }

    m_currentStepIndex++;
}

void LinkedListVisualizerScreen::updateVisuals(float deltaTime) {
    if (m_window) {
        if (m_showAlert) {
            m_alertResetBtn->update(deltaTime, *m_window);
        } else {
            for (auto& btn : m_typeButtons) btn->update(deltaTime, *m_window);
            for (auto& btn : m_opButtons) btn->update(deltaTime, *m_window);
            m_decValBtn->update(deltaTime, *m_window);
            m_incValBtn->update(deltaTime, *m_window);
            m_randValBtn->update(deltaTime, *m_window);
            m_valInputFrame->update(deltaTime, *m_window);
            m_decPosBtn->update(deltaTime, *m_window);
            m_incPosBtn->update(deltaTime, *m_window);
            m_posInputFrame->update(deltaTime, *m_window);
            m_eduModeBtn->update(deltaTime, *m_window);
        }

        if (m_isValActive || m_isPosActive) {
            m_cursorBlinkTime += deltaTime;
        }
    }

    if (m_isPlaying && !m_showAlert) {
        m_elapsedTime += deltaTime;

        // Smooth Pointer target coordinate interpolation
        sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
        float startX = bounds.left + 40.f;
        float centerY = bounds.top + 180.f + 22.f;

        for (int i = 0; i < 7; ++i) {
            // Update actual target pointer destinations based on current model lists
            if (i < static_cast<int>(m_nodes.size())) {
                if (m_listType == LinkedListType::Circular && i == static_cast<int>(m_nodes.size() - 1)) {
                    // Circular return points to head node index 0
                    m_nextArrowTargets[i] = sf::Vector2f(startX, centerY);
                } else if (i < static_cast<int>(m_nodes.size() - 1)) {
                    m_nextArrowTargets[i] = sf::Vector2f(startX + (i + 1) * 110.f, centerY);
                } else {
                    // Null target (descend downward)
                    m_nextArrowTargets[i] = sf::Vector2f(startX + i * 110.f + 65.f, centerY + 40.f);
                }

                if (i > 0) {
                    m_prevArrowTargets[i] = sf::Vector2f(startX + (i - 1) * 110.f + 65.f, centerY);
                } else {
                    m_prevArrowTargets[i] = sf::Vector2f(startX, centerY + 40.f);
                }
            }

            // Lerp target positions smoothly
            m_nextArrowCurrent[i] += (m_nextArrowTargets[i] - m_nextArrowCurrent[i]) * deltaTime * m_animationSpeed * 6.f;
            m_prevArrowCurrent[i] += (m_prevArrowTargets[i] - m_prevArrowCurrent[i]) * deltaTime * m_animationSpeed * 6.f;
        }

        m_stepAccumulator += deltaTime * m_animationSpeed;
        float stepDelay = 0.5f;
        if (m_stepAccumulator >= stepDelay) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }

    // Refresh live stats text labels
    m_statsSizeText.setString("Current Size: " + std::to_string(m_nodes.size()));
    m_statsOpsText.setString("Operation Count: " + std::to_string(m_operationCount));
    m_statsTraversalsText.setString("Traversal Count: " + std::to_string(m_traversalCount));
    m_statsInsText.setString("Insertions: " + std::to_string(m_insertCount));
    m_statsDelText.setString("Deletions: " + std::to_string(m_deleteCount));
    m_statsCompsText.setString("Search Comps: " + std::to_string(m_searchComparisons));

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Elapsed Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);

    // Sync input frames buttons texts
    if (m_isValActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_valInputFrame->setLabel(displayStr);
    } else {
        m_valInputFrame->setLabel("Value: " + std::to_string(m_inputValue));
    }

    if (m_isPosActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_posInputFrame->setLabel(displayStr);
    } else {
        m_posInputFrame->setLabel("Pos: " + std::to_string(m_inputPos));
    }
}

void LinkedListVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    if (m_showAlert) {
        m_alertResetBtn->handleEvent(event, window);
        return;
    }

    // Capture manual input numeric entries
    if ((m_isValActive || m_isPosActive) && event.type == sf::Event::TextEntered) {
        sf::Uint32 code = event.text.unicode;
        if (code == 8) {
            if (!m_inputTextBuffer.empty()) m_inputTextBuffer.pop_back();
        } else if (code == 13) {
            commitInputText();
        } else if (code >= '0' && code <= '9') {
            if (m_inputTextBuffer.size() < 2) {
                m_inputTextBuffer += static_cast<char>(code);
            }
        }
        return;
    }

    // Dismiss focus click away
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        auto clickInside = [&](const std::unique_ptr<ui::Button>& btn) -> bool {
            sf::Vector2f sz = btn->getSize();
            sf::Vector2f pos = btn->getPosition();
            return sf::FloatRect(pos.x, pos.y, sz.x, sz.y).contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        };

        if (!clickInside(m_valInputFrame) && !clickInside(m_posInputFrame)) {
            if (m_isValActive || m_isPosActive) {
                commitInputText();
            }
        }
    }

    for (auto& btn : m_typeButtons) btn->handleEvent(event, window);
    for (auto& btn : m_opButtons) btn->handleEvent(event, window);
    m_decValBtn->handleEvent(event, window);
    m_incValBtn->handleEvent(event, window);
    m_randValBtn->handleEvent(event, window);
    m_valInputFrame->handleEvent(event, window);
    m_decPosBtn->handleEvent(event, window);
    m_incPosBtn->handleEvent(event, window);
    m_posInputFrame->handleEvent(event, window);
    m_eduModeBtn->handleEvent(event, window);
}

void LinkedListVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // 1. Draw controls
    for (auto& btn : m_typeButtons) window.draw(*btn);
    for (auto& btn : m_opButtons) window.draw(*btn);
    window.draw(*m_decValBtn);
    window.draw(*m_incValBtn);
    window.draw(*m_randValBtn);
    window.draw(*m_valInputFrame);
    window.draw(*m_decPosBtn);
    window.draw(*m_incPosBtn);
    window.draw(*m_posInputFrame);
    window.draw(*m_eduModeBtn);
    window.draw(m_explanationText);

    // 2. Draw List Nodes
    float cellW = 65.f;
    float cellH = 45.f;
    float startX = bounds.left + 40.f;
    float centerY = bounds.top + 180.f;

    for (size_t i = 0; i < m_nodes.size(); ++i) {
        float cx = startX + i * 110.f;
        float cy = centerY;

        // Draw node outline card based on pointer states
        sf::Color outlineCol = colors.border;
        if (static_cast<int>(i) == m_activeIdx) outlineCol = colors.barActive;
        else if (static_cast<int>(i) == m_prevIdx) outlineCol = colors.warning;
        else if (static_cast<int>(i) == m_nextIdx) outlineCol = colors.danger;
        else if (static_cast<int>(i) == m_headIdx) outlineCol = sf::Color(245, 158, 11); // Gold outline
        else if (static_cast<int>(i) == m_tailIdx) outlineCol = sf::Color(139, 92, 246); // Purple outline

        ui::RoundedRectangleShape box(sf::Vector2f(cellW, cellH), 4.f, 4);
        box.setPosition(cx, cy);
        box.setFillColor(colors.cardBg);
        box.setOutlineThickness(2.f);
        box.setOutlineColor(outlineCol);
        window.draw(box);

        // Draw split dividers
        if (m_listType == LinkedListType::Doubly) {
            // Split into Prev (15px), Data (35px), Next (15px)
            sf::RectangleShape prevDivider(sf::Vector2f(1.5f, cellH));
            prevDivider.setPosition(cx + 15.f, cy);
            prevDivider.setFillColor(colors.border);
            window.draw(prevDivider);

            sf::RectangleShape nextDivider(sf::Vector2f(1.5f, cellH));
            nextDivider.setPosition(cx + 50.f, cy);
            nextDivider.setFillColor(colors.border);
            window.draw(nextDivider);
        } else {
            // Split into Data (40px) and Next (25px)
            sf::RectangleShape nextDivider(sf::Vector2f(1.5f, cellH));
            nextDivider.setPosition(cx + 40.f, cy);
            nextDivider.setFillColor(colors.border);
            window.draw(nextDivider);
        }

        // Draw node value text
        sf::Text valText;
        valText.setFont(boldFont);
        valText.setString(std::to_string(m_nodes[i]));
        valText.setCharacterSize(13);
        valText.setFillColor(colors.textPrimary);
        
        sf::FloatRect valBounds = valText.getLocalBounds();
        valText.setOrigin(valBounds.left + valBounds.width / 2.f, valBounds.top + valBounds.height / 2.f);
        
        if (m_listType == LinkedListType::Doubly) {
            valText.setPosition(cx + 32.5f, cy + cellH / 2.f);
        } else {
            valText.setPosition(cx + 20.f, cy + cellH / 2.f);
        }
        window.draw(valText);

        // Draw node memory hex address below the node box
        sf::Text addrText;
        addrText.setFont(regularFont);
        addrText.setString(m_nodeAddresses[i]);
        addrText.setCharacterSize(9);
        addrText.setFillColor(colors.textSecondary);
        addrText.setPosition(cx + 5.f, cy + cellH + 4.f);
        window.draw(addrText);

        // Draw Pointer indicator names
        if (static_cast<int>(i) == m_headIdx) {
            sf::Text ptrText;
            ptrText.setFont(boldFont);
            ptrText.setString("H");
            ptrText.setCharacterSize(10);
            ptrText.setFillColor(sf::Color(245, 158, 11));
            ptrText.setPosition(cx + 2.f, cy - 14.f);
            window.draw(ptrText);
        }
        if (static_cast<int>(i) == m_tailIdx) {
            sf::Text ptrText;
            ptrText.setFont(boldFont);
            ptrText.setString("T");
            ptrText.setCharacterSize(10);
            ptrText.setFillColor(sf::Color(139, 92, 246));
            ptrText.setPosition(cx + cellW - 12.f, cy - 14.f);
            window.draw(ptrText);
        }

        // Draw Pointers connection lines (using smooth coordinate lerps targets)
        if (m_listType == LinkedListType::Circular && i == m_nodes.size() - 1) {
            // Circular loop-back arrow drawn under list
            sf::Vector2f startArrow(cx + cellW, cy + cellH / 2.f);
            sf::Vector2f targetArrow = m_nextArrowCurrent[i];

            sf::Vertex returnLine[] = {
                sf::Vertex(startArrow, colors.accent),
                sf::Vertex(sf::Vector2f(startArrow.x, cy + cellH + 25.f), colors.accent),
                sf::Vertex(sf::Vector2f(targetArrow.x - 15.f, cy + cellH + 25.f), colors.accent),
                sf::Vertex(sf::Vector2f(targetArrow.x - 15.f, cy + cellH / 2.f), colors.accent),
                sf::Vertex(targetArrow, colors.accent)
            };
            window.draw(returnLine, 5, sf::LineStrip);

            sf::CircleShape arrowTip(3.f, 3);
            arrowTip.setOrigin(3.f, 3.f);
            arrowTip.setRotation(90.f);
            arrowTip.setPosition(targetArrow);
            arrowTip.setFillColor(colors.accent);
            window.draw(arrowTip);

        } else {
            // Normal sequential next pointer arrow
            sf::Vector2f startArrow(cx + cellW, cy + cellH / 2.f);
            sf::Vector2f targetArrow = m_nextArrowCurrent[i];

            if (i < m_nodes.size() - 1) {
                sf::RectangleShape line(sf::Vector2f(targetArrow.x - startArrow.x, 2.f));
                line.setPosition(startArrow);
                line.setFillColor(colors.accent);
                window.draw(line);

                sf::CircleShape arrowTip(4.f, 3);
                arrowTip.setOrigin(4.f, 4.f);
                arrowTip.setRotation(90.f);
                arrowTip.setPosition(targetArrow);
                arrowTip.setFillColor(colors.accent);
                window.draw(arrowTip);
            } else {
                // Null next pointer drawn pointing downward
                sf::Vertex nullLine[] = {
                    sf::Vertex(startArrow, colors.border),
                    sf::Vertex(targetArrow, colors.border)
                };
                window.draw(nullLine, 2, sf::Lines);
            }
        }

        // Draw Doubly Prev arrows
        if (m_listType == LinkedListType::Doubly) {
            sf::Vector2f startArrow(cx, cy + cellH / 2.f + 5.f);
            sf::Vector2f targetArrow = m_prevArrowCurrent[i];

            if (i > 0) {
                sf::RectangleShape line(sf::Vector2f(startArrow.x - targetArrow.x, 2.f));
                line.setPosition(targetArrow.x, targetArrow.y + 5.f);
                line.setFillColor(colors.accent);
                window.draw(line);

                sf::CircleShape arrowTip(4.f, 3);
                arrowTip.setOrigin(4.f, 4.f);
                arrowTip.setRotation(270.f);
                arrowTip.setPosition(targetArrow.x, targetArrow.y + 5.f);
                arrowTip.setFillColor(colors.accent);
                window.draw(arrowTip);
            } else {
                // Null prev pointer pointing downward
                sf::Vertex nullLine[] = {
                    sf::Vertex(startArrow, colors.border),
                    sf::Vertex(targetArrow, colors.border)
                };
                window.draw(nullLine, 2, sf::Lines);
            }
        }
    }

    // 3. Draw statistics in Info panel
    window.draw(m_statsTitleText);
    window.draw(m_statsSizeText);
    window.draw(m_statsOpsText);
    window.draw(m_statsTraversalsText);
    window.draw(m_statsInsText);
    window.draw(m_statsDelText);
    window.draw(m_statsCompsText);
    window.draw(m_statsTimeText);

    // 4. Draw History logs stack in Info panel
    window.draw(m_historyTitleText);
    for (auto& txt : m_historyTexts) {
        window.draw(txt);
    }

    // 5. Draw pseudocode textos
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

    // 6. Draw alerts overlay
    if (m_showAlert) {
        window.draw(*m_alertOverlay);
        window.draw(m_alertTitleText);
        window.draw(m_alertMsgText);
        window.draw(*m_alertResetBtn);
    }
}

// -------------------------------------------------------------
// STEP RECORDERS ACCUMULATORS
// -------------------------------------------------------------
void LinkedListVisualizerScreen::addCompareStep(int val, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::Compare;
    step.value = val;
    step.index1 = -1;
    step.index2 = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addTraversalStep(int idx, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::TraversalHighlight;
    step.value = -1;
    step.index1 = idx;
    step.index2 = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addSetPointersStep(int prev, int curr, int next, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::SetPointers;
    step.value = next;
    step.index1 = prev;
    step.index2 = curr;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addPointerRedirectStep(int fromIdx, int toIdx, const std::string& ptrName, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::PointerRedirect;
    step.value = -1;
    step.index1 = fromIdx;
    step.index2 = toIdx;
    step.line = line;
    step.explanation = ptrName; // "next" or "prev"
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addSpawnNodeStep(int val, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::SpawnNode;
    step.value = val;
    step.index1 = -1;
    step.index2 = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addPopNodeStep(int val, int idx, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::PopNode;
    step.value = val;
    step.index1 = idx;
    step.index2 = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addExplainStep(const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::Explain;
    step.value = -1;
    step.index1 = -1;
    step.index2 = -1;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::SuccessAlert;
    step.value = -1;
    step.index1 = -1;
    step.index2 = -1;
    step.line = line;
    step.explanation = msg;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::FailAlert;
    step.value = -1;
    step.index1 = -1;
    step.index2 = -1;
    step.line = line;
    step.explanation = msg;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addClearStep(int line, const std::string& explanation) {
    ListStep step;
    step.type = ListStep::Type::Clear;
    step.value = -1;
    step.index1 = -1;
    step.index2 = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addHighlightStep(int line) {
    ListStep step;
    step.type = ListStep::Type::HighlightLine;
    step.value = -1;
    step.index1 = -1;
    step.index2 = -1;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    m_steps.push_back(step);
}

void LinkedListVisualizerScreen::addCommitNodeStep(int val, int line, const std::string& explanation) {
    ListStep step;
    // We can reuse Type::SetPointers with special value to commit
    step.type = ListStep::Type::CommitNode;
    step.value = val; // val != -1 for insert, val == -1 for delete
    step.index1 = -1;
    step.index2 = -1;
    step.line = line;
    step.explanation = explanation;
    m_steps.push_back(step);
}

} // namespace core
