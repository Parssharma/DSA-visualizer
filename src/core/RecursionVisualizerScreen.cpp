#include "core/RecursionVisualizerScreen.hpp"
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
#include <cmath>
#include <queue>
#include <stack>

namespace core {

RecursionVisualizerScreen::RecursionVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("Recursion & Backtracking"),
      m_algorithm(RecursionAlgorithm::Fibonacci),
      m_inputValue(4),
      m_isEducationalMode(true),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_activeFrameHighlight(-1),
      m_activeRow(-1),
      m_activeCol(-1),
      m_activeState(0),
      m_isValActive(false),
      m_cursorBlinkTime(0.f),
      m_totalCalls(0),
      m_maxDepth(0),
      m_currentDepth(0),
      m_totalBacktracks(0),
      m_solutionsFound(0),
      m_elapsedTime(0.f),
      m_showAlert(false),
      m_activePseudocodeLine(-1) {
    
    initCommonUI(window);
    initUIComponents();
    loadAlgorithm(RecursionAlgorithm::Fibonacci);
}

void RecursionVisualizerScreen::initUIComponents() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(12);
    m_explanationText.setFillColor(colors.textSecondary);

    // 1. Algo tabs
    std::vector<std::string> tabNames = {"Fibonacci", "Factorial", "Binary Search", "N-Queens", "Sudoku"};
    for (size_t i = 0; i < tabNames.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(tabNames[i], sf::Vector2f(95.f, 26.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        RecursionAlgorithm algo = static_cast<RecursionAlgorithm>(i);
        btn->setCallback([this, algo]() { loadAlgorithm(algo); });
        m_algoTabs.push_back(std::move(btn));
    }

    // 2. Adjusters & Educational Mode
    m_decValBtn = std::make_unique<ui::Button>("-", sf::Vector2f(24.f, 24.f), 12);
    m_decValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_decValBtn->setCallback([this]() { adjustVal(-1); });

    m_incValBtn = std::make_unique<ui::Button>("+", sf::Vector2f(24.f, 24.f), 12);
    m_incValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_incValBtn->setCallback([this]() { adjustVal(1); });

    m_valInputFrame = std::make_unique<ui::Button>("Value: 4", sf::Vector2f(90.f, 24.f), 10);
    m_valInputFrame->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_valInputFrame->setCallback([this]() {
        m_isValActive = true;
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

    // 3. Operations
    auto runBtn = std::make_unique<ui::Button>("Run Algo", sf::Vector2f(100.f, 24.f), 10);
    runBtn->setColors(colors.accent, colors.accentHover, colors.textPrimary);
    runBtn->setCallback([this]() { triggerRun(); });
    m_opButtons.push_back(std::move(runBtn));

    // 4. Statistics
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("RECURSION METRICS");
    m_statsTitleText.setCharacterSize(14);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsCallsText.setFont(regularFont);
    m_statsCallsText.setCharacterSize(11);
    m_statsCallsText.setFillColor(colors.textPrimary);

    m_statsMaxDepthText.setFont(regularFont);
    m_statsMaxDepthText.setCharacterSize(11);
    m_statsMaxDepthText.setFillColor(colors.textPrimary);

    m_statsCurrentDepthText.setFont(regularFont);
    m_statsCurrentDepthText.setCharacterSize(11);
    m_statsCurrentDepthText.setFillColor(colors.textPrimary);

    m_statsBacktracksText.setFont(regularFont);
    m_statsBacktracksText.setCharacterSize(11);
    m_statsBacktracksText.setFillColor(colors.textPrimary);

    m_statsSolutionsText.setFont(regularFont);
    m_statsSolutionsText.setCharacterSize(11);
    m_statsSolutionsText.setFillColor(colors.textPrimary);

    m_statsTimeText.setFont(regularFont);
    m_statsTimeText.setCharacterSize(11);
    m_statsTimeText.setFillColor(colors.textPrimary);

    // 5. Alert Overlay Panel
    m_alertOverlay = std::make_unique<ui::Panel>(sf::Vector2f(320.f, 150.f), "Alert!");
    m_alertOverlay->setBackgroundColor(sf::Color(26, 26, 36, 242));
    m_alertOverlay->setBorderThickness(2.f);

    m_alertTitleText.setFont(boldFont);
    m_alertTitleText.setCharacterSize(16);
    m_alertTitleText.setFillColor(colors.success);

    m_alertMsgText.setFont(regularFont);
    m_alertMsgText.setCharacterSize(12);
    m_alertMsgText.setFillColor(colors.textPrimary);

    m_alertResetBtn = std::make_unique<ui::Button>("Close", sf::Vector2f(90.f, 28.f), 11);
    m_alertResetBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_alertResetBtn->setCallback([this]() {
        m_showAlert = false;
    });
}

void RecursionVisualizerScreen::updateUILayouts() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    // Row 1 (tabs)
    for (size_t i = 0; i < m_algoTabs.size(); ++i) {
        m_algoTabs[i]->setPosition(sf::Vector2f(startX + i * (95.f + spacing), startY));
    }

    // Row 2 (adjusters)
    float row2Y = startY + 36.f;
    m_decValBtn->setPosition(sf::Vector2f(startX, row2Y));
    m_valInputFrame->setPosition(sf::Vector2f(startX + 24.f + spacing, row2Y));
    m_incValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing, row2Y));
    m_eduModeBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing + 24.f + spacing, row2Y));
    
    // Row 2 operation button
    m_opButtons[0]->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing + 24.f + spacing + 105.f + spacing + 15.f, row2Y));

    // Explanation
    m_explanationText.setPosition(bounds.left + 15.f, row2Y + 35.f);

    // Layout panels inside drawing area
    float drawingStartY = row2Y + 68.f;
    float drawingHeight = bounds.height - 110.f;

    m_stackPanelBounds = sf::FloatRect(bounds.left + 10.f, drawingStartY, 175.f, drawingHeight - 110.f);
    m_memoryPanelBounds = sf::FloatRect(bounds.left + 10.f, drawingStartY + drawingHeight - 100.f, 175.f, 90.f);
    m_timelinePanelBounds = sf::FloatRect(bounds.left + bounds.width - 200.f, drawingStartY, 190.f, drawingHeight);
    
    m_treePanelBounds = sf::FloatRect(bounds.left + 195.f, drawingStartY, bounds.width - 405.f, drawingHeight);
    m_gridPanelBounds = sf::FloatRect(bounds.left + 195.f, drawingStartY, bounds.width - 405.f, drawingHeight);

    // Stats layout
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 70.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsCallsText.setPosition(infoBounds.left, statsStartY + 20.f);
    m_statsMaxDepthText.setPosition(infoBounds.left, statsStartY + 36.f);
    m_statsCurrentDepthText.setPosition(infoBounds.left, statsStartY + 52.f);
    m_statsBacktracksText.setPosition(infoBounds.left, statsStartY + 68.f);
    m_statsSolutionsText.setPosition(infoBounds.left, statsStartY + 84.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 100.f);

    // Theory complexity parameters
    m_theoryTitle.setPosition(infoBounds.left, statsStartY + 130.f);
    m_theoryBody.setPosition(infoBounds.left, statsStartY + 155.f);

    // Pseudocode texts Y positions
    float pseudocodeStartY = statsStartY + 275.f;
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        m_pseudocodeTexts[i].setPosition(infoBounds.left + 10.f, pseudocodeStartY + i * 14.f);
    }

    // Centering alert panel
    float ax = bounds.left + (bounds.width - 320.f) / 2.f;
    float ay = bounds.top + (bounds.height - 150.f) / 2.f;
    m_alertOverlay->setPosition(sf::Vector2f(ax, ay));

    sf::FloatRect abounds = m_alertOverlay->getContentBounds();
    m_alertTitleText.setPosition(abounds.left, abounds.top + 5.f);
    m_alertMsgText.setPosition(abounds.left, abounds.top + 32.f);
    m_alertResetBtn->setPosition(sf::Vector2f(abounds.left + (abounds.width - 90.f) / 2.f, abounds.top + 80.f));
}

void RecursionVisualizerScreen::loadAlgorithm(RecursionAlgorithm algo) {
    m_algorithm = algo;
    m_isPlaying = false;
    m_playPauseButton->setLabel("Play");

    const auto& colors = ThemeManager::getInstance().getColors();
    m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);

    // Update Tab Colors
    for (size_t i = 0; i < m_algoTabs.size(); ++i) {
        if (i == static_cast<size_t>(algo)) {
            m_algoTabs[i]->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        } else {
            m_algoTabs[i]->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        }
    }

    // Update Theory Complexities
    switch (algo) {
        case RecursionAlgorithm::Fibonacci:
            m_theoryTitle.setString("Fibonacci Tree Recursion");
            m_theoryBody.setString("Computes F(N) = F(N-1) + F(N-2). Without memoization, it branches exponentially leading to a tree complexity of O(2^N) time and O(N) space stack depth.");
            m_inputValue = 4;
            break;
        case RecursionAlgorithm::Factorial:
            m_theoryTitle.setString("Factorial Linear Recursion");
            m_theoryBody.setString("Computes N! = N * (N-1)!. It executes linearly with O(N) time complexity and O(N) stack frame memory space depth.");
            m_inputValue = 5;
            break;
        case RecursionAlgorithm::BinarySearch:
            m_theoryTitle.setString("Recursive Binary Search");
            m_theoryBody.setString("Recursively halves search bounds: mid = (L + R)/2. Divide-and-conquer strategy resulting in extremely fast O(log N) time and O(log N) space complexity.");
            m_inputValue = 8; // target value to search
            break;
        case RecursionAlgorithm::NQueens:
            m_theoryTitle.setString("N-Queens Backtracking");
            m_theoryBody.setString("Places N queens on an NxN chessboard such that no two queens attack each other. Backtracks when safe placements are impossible. Time: O(N!).");
            m_inputValue = 4; // 4x4 board
            break;
        case RecursionAlgorithm::Sudoku:
            m_theoryTitle.setString("Sudoku Solver Backtracking");
            m_theoryBody.setString("Places digits 1-9 in empty cells, backtracking on conflict violations. DFS path search space of size O(9^k) where k is empty cells.");
            m_inputValue = 1; // default level 1 grid
            break;
    }

    loadPseudocode(algo);
    onReset();
}

void RecursionVisualizerScreen::loadPseudocode(RecursionAlgorithm algo) {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    switch (algo) {
        case RecursionAlgorithm::Fibonacci:
            m_pseudocodeLines = {
                "fib(n):",
                "  if n <= 1: return n",
                "  return fib(n-1) + fib(n-2)"
            };
            break;
        case RecursionAlgorithm::Factorial:
            m_pseudocodeLines = {
                "fact(n):",
                "  if n <= 1: return 1",
                "  return n * fact(n-1)"
            };
            break;
        case RecursionAlgorithm::BinarySearch:
            m_pseudocodeLines = {
                "binarySearch(arr, target, L, R):",
                "  if L > R: return -1",
                "  mid = (L + R) / 2",
                "  if arr[mid] == target: return mid",
                "  if target < arr[mid]:",
                "    return BS(arr, target, L, mid-1)",
                "  return BS(arr, target, mid+1, R)"
            };
            break;
        case RecursionAlgorithm::NQueens:
            m_pseudocodeLines = {
                "solveNQueens(row):",
                "  if row == N: return true",
                "  for col = 0 to N-1:",
                "    if isSafe(row, col):",
                "      placeQueen(row, col)",
                "      if solve(row+1): return true",
                "      removeQueen(row, col) // backtrack"
            };
            break;
        case RecursionAlgorithm::Sudoku:
            m_pseudocodeLines = {
                "solveSudoku():",
                "  if grid is full: return true",
                "  find empty (row, col)",
                "  for val = 1 to 9:",
                "    if isSafe(row, col, val):",
                "      grid[row][col] = val",
                "      if solve(): return true",
                "      grid[row][col] = 0 // backtrack"
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
    updateUILayouts();
}

void RecursionVisualizerScreen::adjustVal(int delta) {
    m_inputValue += delta;
    if (m_inputValue < 1) m_inputValue = 1;
    if (m_algorithm == RecursionAlgorithm::Fibonacci && m_inputValue > 5) m_inputValue = 5;
    if (m_algorithm == RecursionAlgorithm::Factorial && m_inputValue > 6) m_inputValue = 6;
    if (m_algorithm == RecursionAlgorithm::NQueens && m_inputValue > 8) m_inputValue = 8;
    if (m_algorithm == RecursionAlgorithm::Sudoku && m_inputValue > 2) m_inputValue = 2;
}

void RecursionVisualizerScreen::commitValueInput() {
    m_isValActive = false;
    if (!m_inputTextBuffer.empty()) {
        int val = std::atoi(m_inputTextBuffer.c_str());
        if (val >= 1) {
            m_inputValue = val;
            adjustVal(0); // clamp boundaries
        }
    }
}

// -------------------------------------------------------------
// CORE RECURSIVE RECORDER TRIGGERS
// -------------------------------------------------------------
void RecursionVisualizerScreen::triggerRun() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    // Reset stats
    m_totalCalls = 0;
    m_maxDepth = 0;
    m_currentDepth = 0;
    m_totalBacktracks = 0;
    m_solutionsFound = 0;
    m_elapsedTime = 0.f;

    m_currentFrames.clear();
    m_currentTreeNodes.clear();
    m_currentTimeline.clear();

    addExplainStep("Initializing recursion: " + m_theoryTitle.getString());

    int nodeCounter = 0;

    if (m_algorithm == RecursionAlgorithm::Fibonacci) {
        recordFib(m_inputValue, -1, nodeCounter, 0);
        addSuccessAlertStep("Recursion Completed", "Fibonacci tree construction finished successfully.", -1, "Completed.");
    } else if (m_algorithm == RecursionAlgorithm::Factorial) {
        recordFact(m_inputValue, -1, nodeCounter, 0);
        addSuccessAlertStep("Recursion Completed", "Factorial linear recursion stack trace finished.", -1, "Completed.");
    } else if (m_algorithm == RecursionAlgorithm::BinarySearch) {
        std::vector<int> arr = {11, 23, 35, 47, 59, 71, 83, 95};
        addExplainStep("Searching target " + std::to_string(m_inputValue) + " in sorted array: [11, 23, 35, 47, 59, 71, 83, 95]");
        recordBS(arr, m_inputValue, 0, 7, -1, nodeCounter, 0);
    } else if (m_algorithm == RecursionAlgorithm::NQueens) {
        // Set board size
        m_currentNQueensBoard.assign(m_inputValue, -1);
        recordNQueens(0, m_inputValue, -1, nodeCounter, 0);
    } else {
        // Sudoku solver default preset level 1 grid
        m_currentSudokuGrid = {
            {5, 3, 0, 0, 7, 0, 0, 0, 0},
            {6, 0, 0, 1, 9, 5, 0, 0, 0},
            {0, 9, 8, 0, 0, 0, 0, 6, 0},
            {8, 0, 0, 0, 6, 0, 0, 0, 3},
            {4, 0, 0, 8, 0, 3, 0, 0, 1},
            {7, 0, 0, 0, 2, 0, 0, 0, 6},
            {0, 6, 0, 0, 0, 0, 2, 8, 0},
            {0, 0, 0, 4, 1, 9, 0, 0, 5},
            {0, 0, 0, 0, 8, 0, 0, 7, 9}
        };
        if (m_inputValue == 2) {
            // Level 2 harder grid
            m_currentSudokuGrid[0] = {0, 0, 0, 2, 6, 0, 7, 0, 1};
        }
        recordSudoku(0, 0, -1, nodeCounter, 0);
    }
}

// 1. Fibonacci
int RecursionVisualizerScreen::recordFib(int n, int parentId, int& nodeCounter, int depth, bool isLeft) {
    m_totalCalls++;
    m_maxDepth = std::max(m_maxDepth, depth);

    int id = nodeCounter++;
    std::string label = "fib(" + std::to_string(n) + ")";
    
    // Add call stack frame push
    std::string vars = "n=" + std::to_string(n);
    std::string simulatedAddr = "0x" + std::to_string(0x203A + depth * 16);
    
    // Log timeline
    TimelineEntry te = {label, depth, "Call", "Pushed active stack frame. Parameters: " + vars};
    m_currentTimeline.push_back(te);

    // Call tree node
    CallTreeNode tn;
    tn.id = id;
    tn.parentId = parentId;
    tn.depth = depth;
    tn.label = label;
    tn.scale = 0.05f; // grows
    tn.state = 0; // Active
    tn.returnVal = "?";
    
    // Dynamic coordinate layout calculations
    float spacingX = (m_treePanelBounds.width * 0.16f) / std::pow(1.35f, depth);
    float parentX = m_treePanelBounds.left + m_treePanelBounds.width / 2.f;
    float parentY = m_treePanelBounds.top + 30.f;
    if (parentId != -1) {
        auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), [&](const CallTreeNode& n) { return n.id == parentId; });
        if (it != m_currentTreeNodes.end()) {
            parentX = it->targetX;
            parentY = it->targetY;
        }
    }
    
    tn.x = parentX;
    tn.y = parentY;
    tn.targetX = parentId == -1 ? parentX : (isLeft ? parentX - spacingX : parentX + spacingX);
    tn.targetY = parentY + 70.f;
    
    m_currentTreeNodes.push_back(tn);

    // Push Frame
    addPushStep("fib", vars, 0, "Calling fib(" + std::to_string(n) + ")");
    if (m_isEducationalMode) {
        addExplainStep("[Teacher] Branch expand: Creating new recursive branch fib(" + std::to_string(n) + "). Space is allocated on the stack.");
    }

    if (n <= 1) {
        // Base Case Hit
        m_currentTreeNodes.back().state = 1; // Returned
        m_currentTreeNodes.back().returnVal = std::to_string(n);
        
        m_currentTimeline.back().actionType = "BaseCase";
        m_currentTimeline.back().description = "Base case hit. Returning value " + std::to_string(n);

        addBaseCaseStep(1, "Base Case reached: n <= 1. Returning " + std::to_string(n));
        addPopStep(std::to_string(n), 1, "Popping stack frame. Return value: " + std::to_string(n));
        return n;
    }

    addCompareStep(2, "Reducing problem: Computing left branch fib(" + std::to_string(n-1) + ")");
    int leftVal = recordFib(n - 1, id, nodeCounter, depth + 1, true);

    addCompareStep(2, "Reducing problem: Computing right branch fib(" + std::to_string(n-2) + ")");
    int rightVal = recordFib(n - 2, id, nodeCounter, depth + 1, false);

    int result = leftVal + rightVal;

    // Returned
    // Find node in tree
    auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), [&](const CallTreeNode& tn) { return tn.id == id; });
    if (it != m_currentTreeNodes.end()) {
        it->state = 1;
        it->returnVal = std::to_string(result);
    }

    TimelineEntry teRet = {label, depth, "Return", "Function return. Value: " + std::to_string(result)};
    m_currentTimeline.push_back(teRet);

    addPopStep(std::to_string(result), 2, "Returning fib(" + std::to_string(n) + ") = " + std::to_string(result));
    return result;
}

// 2. Factorial
int RecursionVisualizerScreen::recordFact(int n, int parentId, int& nodeCounter, int depth) {
    m_totalCalls++;
    m_maxDepth = std::max(m_maxDepth, depth);

    int id = nodeCounter++;
    std::string label = "fact(" + std::to_string(n) + ")";
    std::string vars = "n=" + std::to_string(n);

    // Call tree node
    CallTreeNode tn;
    tn.id = id;
    tn.parentId = parentId;
    tn.depth = depth;
    tn.label = label;
    tn.scale = 0.05f;
    tn.state = 0;
    tn.returnVal = "?";
    
    float startX = m_treePanelBounds.left + m_treePanelBounds.width / 2.f;
    tn.x = startX;
    tn.y = m_treePanelBounds.top + 30.f + depth * 40.f;
    tn.targetX = startX;
    tn.targetY = tn.y + 40.f;
    m_currentTreeNodes.push_back(tn);

    TimelineEntry te = {label, depth, "Call", "Stack frame push fact(" + std::to_string(n) + ")"};
    m_currentTimeline.push_back(te);

    addPushStep("fact", vars, 0, "Calling fact(" + std::to_string(n) + ")");

    if (n <= 1) {
        m_currentTreeNodes.back().state = 1;
        m_currentTreeNodes.back().returnVal = "1";
        
        m_currentTimeline.back().actionType = "BaseCase";
        m_currentTimeline.back().description = "Base case. Returning 1";

        addBaseCaseStep(1, "Base Case reached: n <= 1. Returning 1");
        addPopStep("1", 1, "Popping stack frame. Return value: 1");
        return 1;
    }

    addCompareStep(2, "Executing recursive step: n * fact(n-1)");
    int subVal = recordFact(n - 1, id, nodeCounter, depth + 1);

    int result = n * subVal;
    auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), [&](const CallTreeNode& tn) { return tn.id == id; });
    if (it != m_currentTreeNodes.end()) {
        it->state = 1;
        it->returnVal = std::to_string(result);
    }

    TimelineEntry teRet = {label, depth, "Return", "Return value " + std::to_string(result)};
    m_currentTimeline.push_back(teRet);

    addPopStep(std::to_string(result), 2, "Returning fact(" + std::to_string(n) + ") = " + std::to_string(result));
    return result;
}

// 3. Binary Search
int RecursionVisualizerScreen::recordBS(const std::vector<int>& arr, int target, int low, int high, int parentId, int& nodeCounter, int depth, bool isLeft) {
    m_totalCalls++;
    m_maxDepth = std::max(m_maxDepth, depth);

    int id = nodeCounter++;
    std::string label = "BS(" + std::to_string(low) + "," + std::to_string(high) + ")";
    std::string vars = "low=" + std::to_string(low) + ",high=" + std::to_string(high) + ",target=" + std::to_string(target);

    // Call tree node
    CallTreeNode tn;
    tn.id = id;
    tn.parentId = parentId;
    tn.depth = depth;
    tn.label = label;
    tn.scale = 0.05f;
    tn.state = 0;
    tn.returnVal = "?";
    
    float spacing = (m_treePanelBounds.width * 0.16f) / std::pow(1.35f, depth);
    float parentX = m_treePanelBounds.left + m_treePanelBounds.width / 2.f;
    float parentY = m_treePanelBounds.top + 30.f;
    if (parentId != -1) {
        auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), [&](const CallTreeNode& n) { return n.id == parentId; });
        if (it != m_currentTreeNodes.end()) {
            parentX = it->targetX;
            parentY = it->targetY;
        }
    }
    tn.x = parentX;
    tn.y = parentY;
    tn.targetX = parentId == -1 ? parentX : (isLeft ? parentX - spacing : parentX + spacing);
    tn.targetY = parentY + 70.f;
    m_currentTreeNodes.push_back(tn);

    TimelineEntry te = {label, depth, "Call", "Binary search range limit check"};
    m_currentTimeline.push_back(te);

    addPushStep("binarySearch", vars, 0, "Calling BS range [" + std::to_string(low) + ", " + std::to_string(high) + "]");

    if (low > high) {
        m_currentTreeNodes.back().state = 2; // failed
        m_currentTreeNodes.back().returnVal = "-1";
        
        addBaseCaseStep(1, "Base Case reached: low > high. Target not found.");
        addPopStep("-1", 1, "Popping stack frame. Return: -1");
        addFailAlertStep("Target Not Found", "Target element " + std::to_string(target) + " is not present in sorted array.", -1, "Search concluded.");
        return -1;
    }

    int mid = (low + high) / 2;
    addCompareStep(2, "Checking mid element index: " + std::to_string(mid) + " value: " + std::to_string(arr[mid]));

    if (arr[mid] == target) {
        m_currentTreeNodes.back().state = 1;
        m_currentTreeNodes.back().returnVal = std::to_string(mid);
        
        m_currentTimeline.back().actionType = "BaseCase";
        m_currentTimeline.back().description = "Target found at index: " + std::to_string(mid);

        addBaseCaseStep(3, "Target found! arr[" + std::to_string(mid) + "] == " + std::to_string(target));
        addPopStep(std::to_string(mid), 3, "Popping stack frame. Return index: " + std::to_string(mid));
        addSuccessAlertStep("Target Found", "Target element located successfully at index: " + std::to_string(mid), -1, "Search concluded.");
        return mid;
    }

    if (target < arr[mid]) {
        addCompareStep(4, "Target < arr[mid]. Searching left half: [" + std::to_string(low) + ", " + std::to_string(mid - 1) + "]");
        int res = recordBS(arr, target, low, mid - 1, id, nodeCounter, depth + 1, true);
        
        auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), [&](const CallTreeNode& tn) { return tn.id == id; });
        if (it != m_currentTreeNodes.end()) {
            it->state = 1;
            it->returnVal = std::to_string(res);
        }
        addPopStep(std::to_string(res), 5, "Popping search stack frame. Return index: " + std::to_string(res));
        return res;
    } else {
        addCompareStep(4, "Target > arr[mid]. Searching right half: [" + std::to_string(mid + 1) + ", " + std::to_string(high) + "]");
        int res = recordBS(arr, target, mid + 1, high, id, nodeCounter, depth + 1, false);
        
        auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), [&](const CallTreeNode& tn) { return tn.id == id; });
        if (it != m_currentTreeNodes.end()) {
            it->state = 1;
            it->returnVal = std::to_string(res);
        }
        addPopStep(std::to_string(res), 6, "Popping search stack frame. Return: " + std::to_string(res));
        return res;
    }
}

// 4. N-Queens Backtracking
bool RecursionVisualizerScreen::isNQueensSafe(const std::vector<int>& board, int row, int col) {
    for (int i = 0; i < row; ++i) {
        if (board[i] == col || abs(board[i] - col) == abs(i - row)) {
            return false;
        }
    }
    return true;
}

bool RecursionVisualizerScreen::recordNQueens(int row, int n, int parentId, int& nodeCounter, int depth) {
    m_totalCalls++;
    m_maxDepth = std::max(m_maxDepth, depth);

    int id = nodeCounter++;
    std::string label = "solve(" + std::to_string(row) + ")";
    std::string vars = "row=" + std::to_string(row) + ",N=" + std::to_string(n);

    // Call stack frame push
    TimelineEntry te = {label, depth, "Call", "Placing queens for row " + std::to_string(row)};
    m_currentTimeline.push_back(te);

    addPushStep("solveQueens", vars, 0, "Trying to place Queen on row: " + std::to_string(row));

    if (row == n) {
        m_solutionsFound++;
        addBaseCaseStep(1, "Base Case reached: All queens placed safely. Solution found!");
        addPopStep("true", 1, "Solution complete. Returning true.");
        addSuccessAlertStep("Solution Found", "N-Queens board configured successfully.", -1, "N-Queens Solver finished.");
        return true;
    }

    for (int col = 0; col < n; ++col) {
        // Place temporarily
        m_currentNQueensBoard[row] = col;
        m_activeRow = row;
        m_activeCol = col;
        m_activeState = 1; // Orange exploration path
        
        addCompareStep(3, "Checking safety at position: Row " + std::to_string(row) + ", Col " + std::to_string(col));

        if (isNQueensSafe(m_currentNQueensBoard, row, col)) {
            m_activeState = 2; // Green safe placement
            addCompareStep(4, "Position safe. Placing Queen on Row " + std::to_string(row) + ", Col " + std::to_string(col));

            if (m_isEducationalMode) {
                addExplainStep("[Teacher] N-Queens safe rule: No collisions detected vertically or diagonally. Proceeding to row " + std::to_string(row+1));
            }

            if (recordNQueens(row + 1, n, id, nodeCounter, depth + 1)) {
                return true; // propagate success
            }

            // Backtrack rollback step
            m_totalBacktracks++;
            m_activeRow = row;
            m_activeCol = col;
            m_activeState = 3; // Red backtrack warning path
            
            addBacktrackStep(row, col, 6, "Queen placement at Row " + std::to_string(row) + ", Col " + std::to_string(col) + " failed down-stream. Backtracking.");
            
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Backtrack: Placing queen at (" + std::to_string(row) + ", " + std::to_string(col) + ") yields no valid setups for remaining rows. Removing Queen.");
            }
            m_currentNQueensBoard[row] = -1;
        } else {
            m_activeState = 3; // Red unsafe placement
            addCompareStep(3, "Conflict at Row " + std::to_string(row) + ", Col " + std::to_string(col) + ". Invalid position.");
            m_currentNQueensBoard[row] = -1;
        }
    }

    TimelineEntry teRet = {label, depth, "Backtrack", "No valid options at row " + std::to_string(row) + ". Returning false."};
    m_currentTimeline.push_back(teRet);

    addPopStep("false", 6, "No safe placement found for Row " + std::to_string(row) + ". Popping stack frame.");
    return false;
}

// 5. Sudoku Backtracking
bool RecursionVisualizerScreen::isSudokuSafe(const std::vector<std::vector<int>>& grid, int row, int col, int val) {
    for (int x = 0; x < 9; x++) {
        if (grid[row][x] == val) return false;
        if (grid[x][col] == val) return false;
    }
    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i + startRow][j + startCol] == val) return false;
        }
    }
    return true;
}

bool RecursionVisualizerScreen::recordSudoku(int row, int col, int parentId, int& nodeCounter, int depth) {
    m_totalCalls++;
    m_maxDepth = std::max(m_maxDepth, depth);

    if (row == 9) {
        m_solutionsFound++;
        addBaseCaseStep(1, "Base Case reached: Sudoku grid solved successfully!");
        addSuccessAlertStep("Sudoku Solved", "Sudoku grid solved successfully.", -1, "Sudoku Completed.");
        return true;
    }

    if (col == 9) {
        return recordSudoku(row + 1, 0, parentId, nodeCounter, depth);
    }

    if (m_currentSudokuGrid[row][col] != 0) {
        return recordSudoku(row, col + 1, parentId, nodeCounter, depth);
    }

    std::string label = "solve(" + std::to_string(row) + "," + std::to_string(col) + ")";
    std::string vars = "row=" + std::to_string(row) + ",col=" + std::to_string(col);

    addPushStep("solveSudoku", vars, 0, "Entering solve block for empty cell (" + std::to_string(row) + ", " + std::to_string(col) + ")");

    for (int val = 1; val <= 9; val++) {
        m_activeRow = row;
        m_activeCol = col;
        m_activeState = 1; // Orange exploration path
        
        addCompareStep(3, "Testing value " + std::to_string(val) + " in cell (" + std::to_string(row) + "," + std::to_string(col) + ")");

        if (isSudokuSafe(m_currentSudokuGrid, row, col, val)) {
            m_currentSudokuGrid[row][col] = val;
            m_activeState = 2; // Green accepted cell path
            
            addCompareStep(4, "Cell safe. Setting cell (" + std::to_string(row) + "," + std::to_string(col) + ") to value " + std::to_string(val));

            if (recordSudoku(row, col + 1, -1, nodeCounter, depth + 1)) {
                return true;
            }

            // Backtrack
            m_totalBacktracks++;
            m_activeState = 3; // Red backtrack warning path
            addBacktrackStep(row, col, 7, "Value " + std::to_string(val) + " yields grid conflicts down-stream. Backtracking cell (" + std::to_string(row) + "," + std::to_string(col) + ")");
            m_currentSudokuGrid[row][col] = 0;
        }
    }

    addPopStep("false", 7, "No valid digits fit cell (" + std::to_string(row) + "," + std::to_string(col) + "). Popping frame.");
    return false;
}

// -------------------------------------------------------------
// PLAYBACK STEP TICK MAPPINGS
// -------------------------------------------------------------
void RecursionVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        
        m_activeFrameHighlight = -1;
        m_activePseudocodeLine = -1;
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);

    switch (step.type) {
        case RecursionStep::Type::PushFrame:
            m_currentDepth++;
            break;
        case RecursionStep::Type::PopFrame:
            if (m_currentDepth > 0) m_currentDepth--;
            break;
        case RecursionStep::Type::Compare:
            break;
        case RecursionStep::Type::BacktrackAction:
            break;
        case RecursionStep::Type::BaseCaseHit:
            break;
        case RecursionStep::Type::Explain:
            break;
        case RecursionStep::Type::SuccessAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().success);
            break;
        case RecursionStep::Type::FailAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().danger);
            break;
        case RecursionStep::Type::SnapshotState:
            break;
        case RecursionStep::Type::HighlightLine:
            break;
    }

    // Load active state snapshot elements
    m_currentFrames = step.framesSnapshot;
    m_activeFrameHighlight = step.activeFrameIdx;
    m_currentTimeline = step.timelineSnapshot;

    // Clamps scaling logic during active tree expansions
    for (const auto& snNode : step.treeNodesSnapshot) {
        auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), 
                               [&](const CallTreeNode& n) { return n.id == snNode.id; });
        if (it == m_currentTreeNodes.end()) {
            CallTreeNode newNode = snNode;
            newNode.scale = 0.05f; // starts small
            m_currentTreeNodes.push_back(newNode);
        } else {
            it->targetX = snNode.targetX;
            it->targetY = snNode.targetY;
            it->state = snNode.state;
            it->returnVal = snNode.returnVal;
        }
    }

    m_currentTreeNodes.erase(
        std::remove_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), 
            [&](const CallTreeNode& n) {
                return std::find_if(step.treeNodesSnapshot.begin(), step.treeNodesSnapshot.end(),
                    [&](const CallTreeNode& sn) { return sn.id == n.id; }) == step.treeNodesSnapshot.end();
            }),
        m_currentTreeNodes.end()
    );

    // Sync grids
    m_currentNQueensBoard = step.nQueensBoard;
    m_currentSudokuGrid = step.sudokuGrid;
    m_activeRow = step.backtrackRow;
    m_activeCol = step.backtrackCol;
    m_activeState = step.backtrackState;

    m_currentStepIndex++;
}

void RecursionVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_activeFrameHighlight = -1;
    m_activeRow = -1;
    m_activeCol = -1;
    m_activeState = 0;

    m_isValActive = false;
    m_showAlert = false;

    m_currentFrames.clear();
    m_currentTreeNodes.clear();
    m_currentTimeline.clear();
    m_currentNQueensBoard.clear();
    m_currentSudokuGrid.clear();

    m_totalCalls = 0;
    m_maxDepth = 0;
    m_currentDepth = 0;
    m_totalBacktracks = 0;
    m_solutionsFound = 0;
    m_elapsedTime = 0.f;

    m_explanationText.setString("Recursion visualizer ready. Trigger Run to simulate.");
    m_activePseudocodeLine = -1;

    if (m_window) {
        updateUILayouts();
    }
}

void RecursionVisualizerScreen::onSizeChanged(int newSize) {
    onReset();
}

void RecursionVisualizerScreen::updateVisuals(float deltaTime) {
    if (m_window) {
        if (m_showAlert) {
            m_alertResetBtn->update(deltaTime, *m_window);
        } else {
            for (auto& btn : m_algoTabs) btn->update(deltaTime, *m_window);
            for (auto& btn : m_opButtons) btn->update(deltaTime, *m_window);
            m_decValBtn->update(deltaTime, *m_window);
            m_incValBtn->update(deltaTime, *m_window);
            m_valInputFrame->update(deltaTime, *m_window);
            m_eduModeBtn->update(deltaTime, *m_window);
        }

        if (m_isValActive) {
            m_cursorBlinkTime += deltaTime;
        }
    }

    // Call tree lerping growth animations
    for (auto& tn : m_currentTreeNodes) {
        float targetScale = 1.f / std::pow(1.15f, tn.depth);
        tn.scale += (targetScale - tn.scale) * deltaTime * m_animationSpeed * 4.f;
        tn.x += (tn.targetX - tn.x) * deltaTime * m_animationSpeed * 4.f;
        tn.y += (tn.targetY - tn.y) * deltaTime * m_animationSpeed * 4.f;
    }

    if (m_isPlaying && !m_showAlert) {
        m_elapsedTime += deltaTime;
        m_stepAccumulator += deltaTime * m_animationSpeed;

        float stepDelay = 0.7f;
        if (m_stepAccumulator >= stepDelay) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }

    // Statistics texts updates
    m_statsCallsText.setString("Total Calls: " + std::to_string(m_totalCalls));
    m_statsMaxDepthText.setString("Max Stack Depth: " + std::to_string(m_maxDepth));
    m_statsCurrentDepthText.setString("Current Depth: " + std::to_string(m_currentDepth));
    m_statsBacktracksText.setString("Total Backtracks: " + std::to_string(m_totalBacktracks));
    m_statsSolutionsText.setString("Solutions Found: " + std::to_string(m_solutionsFound));

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Execution Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);

    if (m_isValActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_valInputFrame->setLabel(displayStr);
    } else {
        m_valInputFrame->setLabel("Input: " + std::to_string(m_inputValue));
    }
}

void RecursionVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    if (m_showAlert) {
        m_alertResetBtn->handleEvent(event, window);
        return;
    }

    if (m_isValActive && event.type == sf::Event::TextEntered) {
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

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f bounds = m_valInputFrame->getSize();
        sf::Vector2f pos = m_valInputFrame->getPosition();
        sf::FloatRect abounds(pos.x, pos.y, bounds.x, bounds.y);
        
        if (!abounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if (m_isValActive) {
                commitValueInput();
            }
        }
    }

    for (auto& btn : m_algoTabs) btn->handleEvent(event, window);
    for (auto& btn : m_opButtons) btn->handleEvent(event, window);
    m_decValBtn->handleEvent(event, window);
    m_incValBtn->handleEvent(event, window);
    m_valInputFrame->handleEvent(event, window);
    m_eduModeBtn->handleEvent(event, window);
}

void RecursionVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // 1. Draw controls
    for (auto& btn : m_algoTabs) window.draw(*btn);
    for (auto& btn : m_opButtons) window.draw(*btn);
    window.draw(*m_decValBtn);
    window.draw(*m_incValBtn);
    window.draw(*m_valInputFrame);
    window.draw(*m_eduModeBtn);
    window.draw(m_explanationText);

    // 2. Draw Call Stack Panel Frame (Debugger Stack UI)
    sf::RectangleShape stackPanelBg(sf::Vector2f(m_stackPanelBounds.width, m_stackPanelBounds.height));
    stackPanelBg.setPosition(m_stackPanelBounds.left, m_stackPanelBounds.top);
    stackPanelBg.setFillColor(sf::Color(20, 20, 28, 200));
    stackPanelBg.setOutlineThickness(1.f);
    stackPanelBg.setOutlineColor(colors.border);
    window.draw(stackPanelBg);

    sf::Text stackTitleText;
    stackTitleText.setFont(boldFont);
    stackTitleText.setString("CALL STACK");
    stackTitleText.setCharacterSize(13);
    stackTitleText.setFillColor(colors.accent);
    stackTitleText.setPosition(m_stackPanelBounds.left + 8.f, m_stackPanelBounds.top + 8.f);
    window.draw(stackTitleText);

    // Draw active frames in stack list representation
    float frameHeight = 28.f;
    float frameSpacing = 5.f;
    for (size_t i = 0; i < m_currentFrames.size(); ++i) {
        if (i >= 8) break; // stack height cap limits
        float fy = m_stackPanelBounds.top + 28.f + i * (frameHeight + frameSpacing);
        
        sf::RectangleShape frameBox(sf::Vector2f(m_stackPanelBounds.width - 10.f, frameHeight));
        frameBox.setPosition(m_stackPanelBounds.left + 5.f, fy);
        frameBox.setFillColor(colors.cardBg);
        
        sf::Color frameBorder = colors.border;
        if (static_cast<int>(i) == m_activeFrameHighlight) {
            frameBorder = colors.barActive;
        }
        frameBox.setOutlineColor(frameBorder);
        frameBox.setOutlineThickness(1.5f);
        window.draw(frameBox);

        sf::Text fnText;
        fnText.setFont(regularFont);
        fnText.setString(m_currentFrames[i].name + "(" + m_currentFrames[i].parameters + ")");
        fnText.setCharacterSize(11);
        fnText.setFillColor(colors.textPrimary);
        fnText.setPosition(m_stackPanelBounds.left + 12.f, fy + 7.f);
        window.draw(fnText);
    }

    // 3. Draw Stack Memory Simulation Panel
    sf::RectangleShape memPanelBg(sf::Vector2f(m_memoryPanelBounds.width, m_memoryPanelBounds.height));
    memPanelBg.setPosition(m_memoryPanelBounds.left, m_memoryPanelBounds.top);
    memPanelBg.setFillColor(sf::Color(20, 20, 28, 200));
    memPanelBg.setOutlineThickness(1.f);
    memPanelBg.setOutlineColor(colors.border);
    window.draw(memPanelBg);

    sf::Text memTitleText;
    memTitleText.setFont(boldFont);
    memTitleText.setString("STACK MEMORY SIM");
    memTitleText.setCharacterSize(13);
    memTitleText.setFillColor(colors.accent);
    memTitleText.setPosition(m_memoryPanelBounds.left + 8.f, m_memoryPanelBounds.top + 8.f);
    window.draw(memTitleText);

    // Draw active variables simulator details
    if (!m_currentFrames.empty()) {
        const auto& activeF = m_currentFrames.back();
        sf::Text detailsText;
        detailsText.setFont(regularFont);
        detailsText.setString("Params: " + activeF.parameters + "\nLocals: " + activeF.localVars + "\nRetAddr: " + activeF.returnAddr + "\nLine: " + std::to_string(activeF.line));
        detailsText.setCharacterSize(11);
        detailsText.setFillColor(colors.textSecondary);
        detailsText.setPosition(m_memoryPanelBounds.left + 12.f, m_memoryPanelBounds.top + 28.f);
        window.draw(detailsText);
    }

    // 4. Draw Timeline Panel Log
    sf::RectangleShape timePanelBg(sf::Vector2f(m_timelinePanelBounds.width, m_timelinePanelBounds.height));
    timePanelBg.setPosition(m_timelinePanelBounds.left, m_timelinePanelBounds.top);
    timePanelBg.setFillColor(sf::Color(20, 20, 28, 200));
    timePanelBg.setOutlineThickness(1.f);
    timePanelBg.setOutlineColor(colors.border);
    window.draw(timePanelBg);

    sf::Text timeTitleText;
    timeTitleText.setFont(boldFont);
    timeTitleText.setString("EXECUTION TIMELINE");
    timeTitleText.setCharacterSize(13);
    timeTitleText.setFillColor(colors.accent);
    timeTitleText.setPosition(m_timelinePanelBounds.left + 8.f, m_timelinePanelBounds.top + 8.f);
    window.draw(timeTitleText);

    float tlItemHeight = 28.f;
    float tlSpacing = 4.f;
    for (size_t i = 0; i < m_currentTimeline.size(); ++i) {
        if (i >= 9) break; // clamp limit display
        float ty = m_timelinePanelBounds.top + 28.f + i * (tlItemHeight + tlSpacing);
        
        sf::RectangleShape itemBox(sf::Vector2f(m_timelinePanelBounds.width - 10.f, tlItemHeight));
        itemBox.setPosition(m_timelinePanelBounds.left + 5.f, ty);
        itemBox.setFillColor(colors.cardBg);
        itemBox.setOutlineThickness(1.f);
        
        sf::Color itemBorder = colors.border;
        if (m_currentTimeline[i].actionType == "Call") itemBorder = colors.warning;
        else if (m_currentTimeline[i].actionType == "Return") itemBorder = colors.success;
        else if (m_currentTimeline[i].actionType == "Backtrack") itemBorder = colors.danger;
        else itemBorder = colors.accent;

        itemBox.setOutlineColor(itemBorder);
        window.draw(itemBox);

        sf::Text logText;
        logText.setFont(regularFont);
        logText.setString(m_currentTimeline[i].funcName + " - " + m_currentTimeline[i].actionType);
        logText.setCharacterSize(11);
        logText.setFillColor(colors.textPrimary);
        logText.setPosition(m_timelinePanelBounds.left + 12.f, ty + 7.f);
        window.draw(logText);
    }

    // 5. Draw Recursive Call Tree OR Backtracking board grids
    if (m_algorithm == RecursionAlgorithm::Fibonacci || m_algorithm == RecursionAlgorithm::Factorial || m_algorithm == RecursionAlgorithm::BinarySearch) {
        // Draw Tree Edges
        for (const auto& tn : m_currentTreeNodes) {
            if (tn.parentId != -1) {
                auto pIt = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), [&](const CallTreeNode& p) { return p.id == tn.parentId; });
                if (pIt != m_currentTreeNodes.end()) {
                    sf::Vector2f parentPos(pIt->x, pIt->y);
                    sf::Vector2f childPos(tn.x, tn.y);
                    sf::Vector2f targetEdge = parentPos + (childPos - parentPos) * tn.scale;

                    sf::Vertex edge[] = {
                        sf::Vertex(parentPos, colors.border),
                        sf::Vertex(targetEdge, colors.border)
                    };
                    window.draw(edge, 2, sf::Lines);
                }
            }
        }

        // Draw Tree Nodes
        float nodeR = 26.f;
        for (const auto& tn : m_currentTreeNodes) {
            float r = nodeR * tn.scale;
            if (r <= 0.5f) continue;

            sf::CircleShape circle(r);
            circle.setOrigin(r, r);
            circle.setPosition(tn.x, tn.y);
            circle.setFillColor(colors.cardBg);

            sf::Color outlineCol = colors.border;
            if (tn.state == 0) outlineCol = colors.barActive; // Orange/Active
            else if (tn.state == 1) outlineCol = colors.success; // Green/Returned
            else outlineCol = colors.textSecondary; // Dimmed

            circle.setOutlineColor(outlineCol);
            circle.setOutlineThickness(2.0f);
            window.draw(circle);

            sf::Text nodeText;
            nodeText.setFont(boldFont);
            nodeText.setString(tn.label + "\n=" + tn.returnVal);
            nodeText.setCharacterSize(static_cast<unsigned int>(11 * tn.scale));
            nodeText.setFillColor(colors.textPrimary);
            
            sf::FloatRect nodeBounds = nodeText.getLocalBounds();
            nodeText.setOrigin(nodeBounds.left + nodeBounds.width / 2.f, nodeBounds.top + nodeBounds.height / 2.f);
            nodeText.setPosition(tn.x, tn.y);
            window.draw(nodeText);
        }
    } else if (m_algorithm == RecursionAlgorithm::NQueens) {
        // Draw Chess board grid for N-Queens
        int n = m_inputValue;
        float cellSize = std::min(150.f / n, 28.f);
        float boardWidth = n * cellSize;
        float startGridX = m_gridPanelBounds.left + (m_gridPanelBounds.width - boardWidth) / 2.f;
        float startGridY = m_gridPanelBounds.top + (m_gridPanelBounds.height - boardWidth) / 2.f;

        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
                cell.setPosition(startGridX + c * cellSize, startGridY + r * cellSize);
                
                // Checkerboard colors
                sf::Color cellBg = (r + c) % 2 == 0 ? sf::Color(45, 45, 55) : sf::Color(30, 30, 40);
                
                // Color decision states
                if (r == m_activeRow && c == m_activeCol) {
                    if (m_activeState == 1) cellBg = colors.warning; // Orange
                    else if (m_activeState == 2) cellBg = colors.success; // Green
                    else cellBg = colors.danger; // Red
                }

                cell.setFillColor(cellBg);
                cell.setOutlineThickness(0.5f);
                cell.setOutlineColor(colors.border);
                window.draw(cell);

                // Draw Queen character if placed
                if (r < static_cast<int>(m_currentNQueensBoard.size()) && m_currentNQueensBoard[r] == c) {
                    sf::Text qText;
                    qText.setFont(boldFont);
                    qText.setString("Q");
                    qText.setCharacterSize(11);
                    qText.setFillColor(colors.textPrimary);
                    
                    sf::FloatRect qBounds = qText.getLocalBounds();
                    qText.setOrigin(qBounds.left + qBounds.width / 2.f, qBounds.top + qBounds.height / 2.f);
                    qText.setPosition(startGridX + c * cellSize + cellSize / 2.f, startGridY + r * cellSize + cellSize / 2.f);
                    window.draw(qText);
                }
            }
        }
    } else {
        // Draw Sudoku grid solver
        float cellSize = 16.f;
        float gridWidth = 9 * cellSize;
        float startGridX = m_gridPanelBounds.left + (m_gridPanelBounds.width - gridWidth) / 2.f;
        float startGridY = m_gridPanelBounds.top + (m_gridPanelBounds.height - gridWidth) / 2.f;

        for (int r = 0; r < 9; ++r) {
            for (int c = 0; c < 9; ++c) {
                sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
                cell.setPosition(startGridX + c * cellSize, startGridY + r * cellSize);

                sf::Color cellBg = colors.cardBg;
                if (r == m_activeRow && c == m_activeCol) {
                    if (m_activeState == 1) cellBg = colors.warning;
                    else if (m_activeState == 2) cellBg = colors.success;
                    else cellBg = colors.danger;
                }

                cell.setFillColor(cellBg);
                
                // Draw bold grid boundaries for sub-grids
                float lineThick = 0.5f;
                if (r % 3 == 0 && r > 0) lineThick = 1.5f;
                cell.setOutlineThickness(lineThick);
                cell.setOutlineColor(colors.border);
                window.draw(cell);

                // Draw cell digit
                if (!m_currentSudokuGrid.empty() && m_currentSudokuGrid[r][c] != 0) {
                    sf::Text cellText;
                    cellText.setFont(boldFont);
                    cellText.setString(std::to_string(m_currentSudokuGrid[r][c]));
                    cellText.setCharacterSize(9);
                    cellText.setFillColor(colors.textPrimary);

                    sf::FloatRect cellBounds = cellText.getLocalBounds();
                    cellText.setOrigin(cellBounds.left + cellBounds.width / 2.f, cellBounds.top + cellBounds.height / 2.f);
                    cellText.setPosition(startGridX + c * cellSize + cellSize / 2.f, startGridY + r * cellSize + cellSize / 2.f);
                    window.draw(cellText);
                }
            }
        }
    }

    // 6. Draw pseudocode lines
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

    // 7. Draw Alert panel overlay
    if (m_showAlert) {
        window.draw(*m_alertOverlay);
        window.draw(m_alertTitleText);
        window.draw(m_alertMsgText);
        window.draw(*m_alertResetBtn);
    }
}

// -------------------------------------------------------------
// STEP RECORDING ACCUMULATORS
// -------------------------------------------------------------
void RecursionVisualizerScreen::addPushStep(const std::string& name, const std::string& params, int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::PushFrame;
    step.line = line;
    step.explanation = explanation;
    
    // Build active stack frame snapshot
    CallStackFrame f;
    f.name = name;
    f.parameters = params;
    f.depth = m_currentFrames.size();
    f.returnVal = "?";
    f.localVars = params;
    f.returnAddr = "0x" + std::to_string(0x10A2 + f.depth * 8);
    f.line = line;
    
    m_currentFrames.push_back(f);
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.size() - 1;
    
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    // Backtrack coordinates
    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = m_activeRow;
    step.backtrackCol = m_activeCol;
    step.backtrackState = m_activeState;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addPopStep(const std::string& returnVal, int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::PopFrame;
    step.line = line;
    step.explanation = explanation;

    if (!m_currentFrames.empty()) {
        m_currentFrames.back().returnVal = returnVal;
    }
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.empty() ? -1 : static_cast<int>(m_currentFrames.size() - 1);

    // Pop the frame locally to capture state updates
    if (!m_currentFrames.empty()) {
        m_currentFrames.pop_back();
    }

    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = m_activeRow;
    step.backtrackCol = m_activeCol;
    step.backtrackState = m_activeState;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addCompareStep(int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::Compare;
    step.line = line;
    step.explanation = explanation;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.empty() ? -1 : static_cast<int>(m_currentFrames.size() - 1);
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = m_activeRow;
    step.backtrackCol = m_activeCol;
    step.backtrackState = m_activeState;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addBacktrackStep(int row, int col, int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::BacktrackAction;
    step.line = line;
    step.explanation = explanation;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.empty() ? -1 : static_cast<int>(m_currentFrames.size() - 1);
    step.treeNodesSnapshot = m_currentTreeNodes;
    
    // Add backtrack to timeline
    TimelineEntry te = {"backtrack", row, "Backtrack", "Backtracked cell (" + std::to_string(row) + "," + std::to_string(col) + ")"};
    m_currentTimeline.push_back(te);
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = row;
    step.backtrackCol = col;
    step.backtrackState = 3; // Red backtrack warning path
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addBaseCaseStep(int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::BaseCaseHit;
    step.line = line;
    step.explanation = explanation;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.empty() ? -1 : static_cast<int>(m_currentFrames.size() - 1);
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = m_activeRow;
    step.backtrackCol = m_activeCol;
    step.backtrackState = m_activeState;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addExplainStep(const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::Explain;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.empty() ? -1 : static_cast<int>(m_currentFrames.size() - 1);
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = m_activeRow;
    step.backtrackCol = m_activeCol;
    step.backtrackState = m_activeState;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::SuccessAlert;
    step.line = line;
    step.explanation = msg;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = -1;
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = -1;
    step.backtrackCol = -1;
    step.backtrackState = 0;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::FailAlert;
    step.line = line;
    step.explanation = msg;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = -1;
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = -1;
    step.backtrackCol = -1;
    step.backtrackState = 0;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addSnapshotStep(int line, const std::string& explanation) {
    RecursionStep step;
    step.type = RecursionStep::Type::SnapshotState;
    step.line = line;
    step.explanation = explanation;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.empty() ? -1 : static_cast<int>(m_currentFrames.size() - 1);
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = m_activeRow;
    step.backtrackCol = m_activeCol;
    step.backtrackState = m_activeState;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::addHighlightStep(int line) {
    RecursionStep step;
    step.type = RecursionStep::Type::HighlightLine;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    step.framesSnapshot = m_currentFrames;
    step.activeFrameIdx = m_currentFrames.empty() ? -1 : static_cast<int>(m_currentFrames.size() - 1);
    step.treeNodesSnapshot = m_currentTreeNodes;
    step.timelineSnapshot = m_currentTimeline;

    step.nQueensBoard = m_currentNQueensBoard;
    step.sudokuGrid = m_currentSudokuGrid;
    step.backtrackRow = m_activeRow;
    step.backtrackCol = m_activeCol;
    step.backtrackState = m_activeState;
    m_steps.push_back(step);
}

void RecursionVisualizerScreen::adjustTreePositions() {
    // Dynamic offsets adjustments are done during recursive recording calculations
}

void RecursionVisualizerScreen::buildTimelineHeights() {
    // Spacing offsets calculations are done in drawVisuals timeline loops
}

} // namespace core
