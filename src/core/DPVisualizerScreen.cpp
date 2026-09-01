#include "core/DPVisualizerScreen.hpp"
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

namespace core {

DPVisualizerScreen::DPVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("Dynamic Programming"),
      m_algorithm(DPAlgorithm::FibMemo),
      m_inputValue(5),
      m_isEducationalMode(true),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_isValActive(false),
      m_cursorBlinkTime(0.f),
      m_cellsComputed(0),
      m_cacheHits(0),
      m_cacheMisses(0),
      m_statesVisited(0),
      m_elapsedTime(0.f),
      m_showAlert(false),
      m_activePseudocodeLine(-1) {
    
    initCommonUI(window);
    initUIComponents();
    loadAlgorithm(DPAlgorithm::FibMemo);
}

void DPVisualizerScreen::initUIComponents() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(12);
    m_explanationText.setFillColor(colors.textSecondary);

    // 1. Algo tabs (7 tabs)
    std::vector<std::string> tabNames = {"Fib Memo", "Fib Tab", "Knapsack", "LCS", "Coin Change", "Matrix", "Edit Dist"};
    for (size_t i = 0; i < tabNames.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(tabNames[i], sf::Vector2f(72.f, 26.f), 9);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        DPAlgorithm algo = static_cast<DPAlgorithm>(i);
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

    m_valInputFrame = std::make_unique<ui::Button>("Value: 5", sf::Vector2f(90.f, 24.f), 10);
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

    // 3. Action operation
    auto runBtn = std::make_unique<ui::Button>("Run DP", sf::Vector2f(90.f, 24.f), 10);
    runBtn->setColors(colors.accent, colors.accentHover, colors.textPrimary);
    runBtn->setCallback([this]() { triggerRun(); });
    m_opButtons.push_back(std::move(runBtn));

    // 4. Statistics
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("DP METRICS");
    m_statsTitleText.setCharacterSize(14);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsComputedText.setFont(regularFont);
    m_statsComputedText.setCharacterSize(11);
    m_statsComputedText.setFillColor(colors.textPrimary);

    m_statsHitsText.setFont(regularFont);
    m_statsHitsText.setCharacterSize(11);
    m_statsHitsText.setFillColor(colors.textPrimary);

    m_statsMissesText.setFont(regularFont);
    m_statsMissesText.setCharacterSize(11);
    m_statsMissesText.setFillColor(colors.textPrimary);

    m_statsStatesText.setFont(regularFont);
    m_statsStatesText.setCharacterSize(11);
    m_statsStatesText.setFillColor(colors.textPrimary);

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

void DPVisualizerScreen::updateUILayouts() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    // Row 1 (tabs)
    for (size_t i = 0; i < m_algoTabs.size(); ++i) {
        m_algoTabs[i]->setPosition(sf::Vector2f(startX + i * (72.f + spacing), startY));
    }

    // Row 2 (adjusters)
    float row2Y = startY + 36.f;
    m_decValBtn->setPosition(sf::Vector2f(startX, row2Y));
    m_valInputFrame->setPosition(sf::Vector2f(startX + 24.f + spacing, row2Y));
    m_incValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing, row2Y));
    m_eduModeBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing + 24.f + spacing, row2Y));
    m_opButtons[0]->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing + 24.f + spacing + 105.f + spacing + 15.f, row2Y));

    // Explanation Y
    m_explanationText.setPosition(bounds.left + 15.f, row2Y + 35.f);

    // Grid Panel Y bounds
    float gridStartY = row2Y + 68.f;
    float gridHeight = bounds.height - 85.f;
    m_gridPanelBounds = sf::FloatRect(bounds.left + 15.f, gridStartY, bounds.width - 30.f, gridHeight);

    // Live Stats right column layout
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 70.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsComputedText.setPosition(infoBounds.left, statsStartY + 20.f);
    m_statsHitsText.setPosition(infoBounds.left, statsStartY + 36.f);
    m_statsMissesText.setPosition(infoBounds.left, statsStartY + 52.f);
    m_statsStatesText.setPosition(infoBounds.left, statsStartY + 68.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 84.f);

    // Theory complexity labels
    m_theoryTitle.setPosition(infoBounds.left, statsStartY + 115.f);
    m_theoryBody.setPosition(infoBounds.left, statsStartY + 140.f);

    // Stacking pseudocode rows
    float pseudocodeStartY = statsStartY + 270.f;
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        m_pseudocodeTexts[i].setPosition(infoBounds.left + 10.f, pseudocodeStartY + i * 14.f);
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

void DPVisualizerScreen::loadAlgorithm(DPAlgorithm algo) {
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

    // Update Theory details
    switch (algo) {
        case DPAlgorithm::FibMemo:
            m_theoryTitle.setString("Fibonacci (Memoization)");
            m_theoryBody.setString("Top-down DP. Solves recursive subproblems, storing results in a memo lookup table. Avoids duplicate branches. Time: O(N), Space: O(N) stack & cache.");
            m_inputValue = 6;
            break;
        case DPAlgorithm::FibTab:
            m_theoryTitle.setString("Fibonacci (Tabulation)");
            m_theoryBody.setString("Bottom-up DP. Solves base cases first, sequentially filling a 1D table up to N. No recursion stack overhead. Time: O(N), Space: O(N) array.");
            m_inputValue = 6;
            break;
        case DPAlgorithm::Knapsack:
            m_theoryTitle.setString("0/1 Knapsack");
            m_theoryBody.setString("Finds optimal item subset maximizing value within capacity W. State: dp[i][w] = max(exclude, include). Time: O(N*W), Space: O(N*W).");
            m_inputValue = 6; // Max Capacity W
            break;
        case DPAlgorithm::LCS:
            m_theoryTitle.setString("Longest Common Subsequence");
            m_theoryBody.setString("Finds length of longest subsequence shared by strings A and B. Cell values depend on matching characters and diagonals. Time: O(M*N), Space: O(M*N).");
            m_inputValue = 5; // length factor
            break;
        case DPAlgorithm::CoinChange:
            m_theoryTitle.setString("Coin Change");
            m_theoryBody.setString("Finds minimum coins needed to sum to amount A. Transitions: dp[a] = min(dp[a], dp[a - coin] + 1). Time: O(N*A), Space: O(A).");
            m_inputValue = 6; // amount to sum to
            break;
        case DPAlgorithm::MatrixChain:
            m_theoryTitle.setString("Matrix Chain Multiplication");
            m_theoryBody.setString("Finds optimal order to multiply matrices minimizing scalar multiplications. Diagonal parenthesization split updates. Time: O(N^3), Space: O(N^2).");
            m_inputValue = 4; // number of matrices
            break;
        case DPAlgorithm::EditDistance:
            m_theoryTitle.setString("Edit Distance");
            m_theoryBody.setString("Finds minimal edit operations (insert, delete, replace) to transform A to B. Cell transitions query top, left, and diagonal. Time: O(M*N).");
            m_inputValue = 4;
            break;
    }

    loadPseudocode(algo);
    onReset();
}

void DPVisualizerScreen::loadPseudocode(DPAlgorithm algo) {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    switch (algo) {
        case DPAlgorithm::FibMemo:
            m_pseudocodeLines = {
                "fibMemo(n):",
                "  if memo[n] != -1: return memo[n]",
                "  if n <= 1: memo[n] = n",
                "  else: memo[n] = fib(n-1) + fib(n-2)",
                "  return memo[n]"
            };
            break;
        case DPAlgorithm::FibTab:
            m_pseudocodeLines = {
                "fibTab(n):",
                "  dp[0] = 0; dp[1] = 1",
                "  for i = 2 to n:",
                "    dp[i] = dp[i-1] + dp[i-2]",
                "  return dp[n]"
            };
            break;
        case DPAlgorithm::Knapsack:
            m_pseudocodeLines = {
                "knapsack(W, wt, val, n):",
                "  for i = 1 to n:",
                "    for w = 1 to W:",
                "      if wt[i-1] <= w:",
                "        dp[i][w] = max(val[i-1] + dp[i-1][w-wt[i-1]], dp[i-1][w])",
                "      else: dp[i][w] = dp[i-1][w]"
            };
            break;
        case DPAlgorithm::LCS:
            m_pseudocodeLines = {
                "LCS(X, Y, m, n):",
                "  for i = 1 to m:",
                "    for j = 1 to n:",
                "      if X[i-1] == Y[j-1]: dp[i][j] = dp[i-1][j-1] + 1",
                "      else: dp[i][j] = max(dp[i-1][j], dp[i][j-1])"
            };
            break;
        case DPAlgorithm::CoinChange:
            m_pseudocodeLines = {
                "coinChange(coins, amt):",
                "  dp[0] = 0 // base case",
                "  for i = 1 to amt: dp[i] = INF",
                "  for c in coins:",
                "    for a = c to amt:",
                "      dp[a] = min(dp[a], dp[a - c] + 1)"
            };
            break;
        case DPAlgorithm::MatrixChain:
            m_pseudocodeLines = {
                "matrixChain(p, n):",
                "  for len = 2 to n:",
                "    for i = 1 to n - len + 1:",
                "      j = i + len - 1; dp[i][j] = INF",
                "      for k = i to j - 1:",
                "        cost = dp[i][k] + dp[k+1][j] + p[i-1]*p[k]*p[j]",
                "        dp[i][j] = min(dp[i][j], cost)"
            };
            break;
        case DPAlgorithm::EditDistance:
            m_pseudocodeLines = {
                "editDistance(A, B, m, n):",
                "  if A[i-1] == B[j-1]: dp[i][j] = dp[i-1][j-1]",
                "  else:",
                "    dp[i][j] = 1 + min(dp[i-1][j],   // Remove",
                "                       dp[i][j-1],   // Insert",
                "                       dp[i-1][j-1]) // Replace"
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

void DPVisualizerScreen::adjustVal(int delta) {
    m_inputValue += delta;
    if (m_inputValue < 1) m_inputValue = 1;
    if (m_algorithm == DPAlgorithm::FibMemo && m_inputValue > 8) m_inputValue = 8;
    if (m_algorithm == DPAlgorithm::FibTab && m_inputValue > 8) m_inputValue = 8;
    if (m_algorithm == DPAlgorithm::Knapsack && m_inputValue > 7) m_inputValue = 7;
    if (m_algorithm == DPAlgorithm::LCS && m_inputValue > 7) m_inputValue = 7;
    if (m_algorithm == DPAlgorithm::CoinChange && m_inputValue > 8) m_inputValue = 8;
    if (m_algorithm == DPAlgorithm::MatrixChain && m_inputValue > 5) m_inputValue = 5;
    if (m_algorithm == DPAlgorithm::EditDistance && m_inputValue > 6) m_inputValue = 6;
}

void DPVisualizerScreen::commitValueInput() {
    m_isValActive = false;
    if (!m_inputTextBuffer.empty()) {
        int val = std::atoi(m_inputTextBuffer.c_str());
        if (val >= 1) {
            m_inputValue = val;
            adjustVal(0);
        }
    }
}

// -------------------------------------------------------------
// CORE DP RECORDERS
// -------------------------------------------------------------
std::vector<DPGridCell> DPVisualizerScreen::captureGridSnapshot() {
    return m_currentGridCells;
}

void DPVisualizerScreen::triggerRun() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    m_cellsComputed = 0;
    m_cacheHits = 0;
    m_cacheMisses = 0;
    m_statesVisited = 0;
    m_elapsedTime = 0.f;

    m_currentGridCells.clear();
    m_currentDependencies.clear();

    addExplainStep("Initializing Dynamic Programming: " + m_theoryTitle.getString());

    switch (m_algorithm) {
        case DPAlgorithm::FibMemo:
            recordFibMemo();
            break;
        case DPAlgorithm::FibTab:
            recordFibTab();
            break;
        case DPAlgorithm::Knapsack:
            recordKnapsack();
            break;
        case DPAlgorithm::LCS:
            recordLCS();
            break;
        case DPAlgorithm::CoinChange:
            recordCoinChange();
            break;
        case DPAlgorithm::MatrixChain:
            recordMatrixChain();
            break;
        case DPAlgorithm::EditDistance:
            recordEditDistance();
            break;
    }
}

// 1. Fibonacci Memoization
void DPVisualizerScreen::recordFibMemo() {
    int n = m_inputValue;
    // Set 1D cache array in grid (1 row, n+1 cols)
    for (int i = 0; i <= n; ++i) {
        m_currentGridCells.push_back({0, i, -1, "memo", "F(" + std::to_string(i) + ")", 0});
    }

    std::vector<int> memo(n + 1, -1);
    fibMemoRec(n, memo);

    // Final backtrack path highlight
    for (int i = 0; i <= n; ++i) {
        m_currentGridCells[i].state = 4; // SolutionPath (Green)
    }
    addSnapshotStep(-1, "Memoization complete. Cache filled.");
    addSuccessAlertStep("DP Completed", "Fibonacci F(" + std::to_string(n) + ") = " + std::to_string(memo[n]), -1, "DP Completed.");
}

int DPVisualizerScreen::fibMemoRec(int n, std::vector<int>& memo) {
    m_statesVisited++;
    if (memo[n] != -1) {
        m_cacheHits++;
        addCacheHitStep(0, n, memo[n], 1, "Cache hit! memo[" + std::to_string(n) + "] = " + std::to_string(memo[n]));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Memoization hit: Subproblem F(" + std::to_string(n) + ") has been computed previously. Recomputation avoided.");
        }
        return memo[n];
    }

    m_cacheMisses++;
    addCellUpdateStep(0, n, -2, 1, "Cache miss. Computing F(" + std::to_string(n) + ")");

    if (n <= 1) {
        memo[n] = n;
        m_cellsComputed++;
        m_currentGridCells[n].value = n;
        m_currentGridCells[n].state = 2; // Filled
        addCellUpdateStep(0, n, n, 2, "Base case: memo[" + std::to_string(n) + "] = " + std::to_string(n));
        return n;
    }

    addDependencyStep(0, n, {{0, n-1}, {0, n-2}}, 3, "Transition: F(" + std::to_string(n) + ") = F(" + std::to_string(n-1) + ") + F(" + std::to_string(n-2) + ")");

    int left = fibMemoRec(n - 1, memo);
    int right = fibMemoRec(n - 2, memo);
    memo[n] = left + right;

    m_cellsComputed++;
    m_currentGridCells[n].value = memo[n];
    m_currentGridCells[n].state = 2; // Filled

    addCellUpdateStep(0, n, memo[n], 3, "Cached result: memo[" + std::to_string(n) + "] = " + std::to_string(memo[n]));
    return memo[n];
}

// 2. Fibonacci Tabulation
void DPVisualizerScreen::recordFibTab() {
    int n = m_inputValue;
    for (int i = 0; i <= n; ++i) {
        m_currentGridCells.push_back({0, i, -1, "dp", "F(" + std::to_string(i) + ")", 0});
    }

    m_currentGridCells[0].value = 0;
    m_currentGridCells[0].state = 2;
    m_cellsComputed++;
    addCellUpdateStep(0, 0, 0, 1, "Base case: dp[0] = 0");

    if (n > 0) {
        m_currentGridCells[1].value = 1;
        m_currentGridCells[1].state = 2;
        m_cellsComputed++;
        addCellUpdateStep(0, 1, 1, 1, "Base case: dp[1] = 1");
    }

    for (int i = 2; i <= n; ++i) {
        m_statesVisited++;
        addDependencyStep(0, i, {{0, i-1}, {0, i-2}}, 3, "Applying recurrence relation for index " + std::to_string(i));
        
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Tabulation transition: dp[" + std::to_string(i) + "] summate previous two contiguous cell states. dp[i] = dp[i-1] + dp[i-2].");
        }

        int val = m_currentGridCells[i-1].value + m_currentGridCells[i-2].value;
        m_currentGridCells[i].value = val;
        m_currentGridCells[i].state = 2;
        m_cellsComputed++;
        
        addCellUpdateStep(0, i, val, 3, "Updated cell dp[" + std::to_string(i) + "] = " + std::to_string(val));
    }

    // Backtrack path
    for (int i = 0; i <= n; ++i) {
        m_currentGridCells[i].state = 4;
    }
    addSnapshotStep(-1, "Tabulation complete. F(" + std::to_string(n) + ") calculated.");
    addSuccessAlertStep("DP Completed", "Tabulation F(" + std::to_string(n) + ") = " + std::to_string(m_currentGridCells[n].value), -1, "Completed.");
}

// 3. 0/1 Knapsack
void DPVisualizerScreen::recordKnapsack() {
    int W = m_inputValue; // capacity limit
    int n = 4; // 4 default items
    std::vector<int> wt = {1, 2, 3, 4};
    std::vector<int> val = {10, 15, 40, 50};

    // 2D grid: rows represent items (0-4), cols represent capacity (0-W)
    for (int i = 0; i <= n; ++i) {
        for (int w = 0; w <= W; ++w) {
            std::string labelRow = i == 0 ? "Empty" : "Item" + std::to_string(i);
            m_currentGridCells.push_back({i, w, 0, labelRow, "c" + std::to_string(w), 0});
        }
    }

    auto getCellIdx = [&](int r, int c) { return r * (W + 1) + c; };

    // Initialize DP 0 values
    for (int w = 0; w <= W; ++w) {
        m_currentGridCells[getCellIdx(0, w)].state = 2;
        m_cellsComputed++;
    }
    for (int i = 0; i <= n; ++i) {
        m_currentGridCells[getCellIdx(i, 0)].state = 2;
        m_cellsComputed++;
    }

    addSnapshotStep(0, "Base cases loaded: capacity 0 and item subset 0 initialized to value 0.");

    for (int i = 1; i <= n; ++i) {
        for (int w = 1; w <= W; ++w) {
            m_statesVisited++;
            int currIdx = getCellIdx(i, w);
            m_currentGridCells[currIdx].state = 1; // Filling (Orange)

            if (wt[i-1] <= w) {
                // Dependency: exclude or include
                int exclIdx = getCellIdx(i-1, w);
                int inclIdx = getCellIdx(i-1, w - wt[i-1]);
                addDependencyStep(i, w, {{i-1, w}, {i-1, w - wt[i-1]}}, 4, "Comparing options: Exclude Item " + std::to_string(i) + " vs Include Item " + std::to_string(i));

                int exclVal = m_currentGridCells[exclIdx].value;
                int inclVal = val[i-1] + m_currentGridCells[inclIdx].value;
                int maxV = std::max(exclVal, inclVal);

                m_currentGridCells[currIdx].value = maxV;
                m_currentGridCells[currIdx].state = 2;
                m_cellsComputed++;

                if (m_isEducationalMode) {
                    addExplainStep("[Teacher] Knapsack decision: Maximize values. Exclude Item " + std::to_string(i) + " (val = " + std::to_string(exclVal) + ") vs Include (value = Item " + std::to_string(i) + " val " + std::to_string(val[i-1]) + " + remaining capacity dp[" + std::to_string(i-1) + "][" + std::to_string(w - wt[i-1]) + "] = " + std::to_string(maxV) + ").");
                }
            } else {
                // Must exclude
                int exclIdx = getCellIdx(i-1, w);
                addDependencyStep(i, w, {{i-1, w}}, 5, "Item " + std::to_string(i) + " weight " + std::to_string(wt[i-1]) + " exceeds capacity " + std::to_string(w) + ". Exclude.");

                int maxV = m_currentGridCells[exclIdx].value;
                m_currentGridCells[currIdx].value = maxV;
                m_currentGridCells[currIdx].state = 2;
                m_cellsComputed++;
            }
        }
    }

    // Backtrack path reconstruction
    int currRow = n;
    int currCol = W;
    while (currRow > 0 && currCol > 0) {
        int idx = getCellIdx(currRow, currCol);
        m_currentGridCells[idx].state = 4; // SolutionPath (Green)
        
        int exclIdx = getCellIdx(currRow - 1, currCol);
        if (m_currentGridCells[idx].value != m_currentGridCells[exclIdx].value) {
            // Included item!
            addBacktrackStep(currRow, currCol, -1, "Solution backtracking: Item " + std::to_string(currRow) + " was INCLUDED.");
            currCol -= wt[currRow-1];
        } else {
            addBacktrackStep(currRow, currCol, -1, "Solution backtracking: Item " + std::to_string(currRow) + " was EXCLUDED.");
        }
        currRow--;
    }
    if (currRow >= 0) {
        m_currentGridCells[getCellIdx(currRow, currCol)].state = 4;
    }

    addSnapshotStep(-1, "Solution path backtrack tracing complete.");
    addSuccessAlertStep("DP Completed", "Knapsack maximum value is: " + std::to_string(m_currentGridCells[getCellIdx(n, W)].value), -1, "Completed.");
}

// 4. LCS
void DPVisualizerScreen::recordLCS() {
    std::string A = "STONE";
    std::string B = "LONGEST";
    int m = A.size();
    int n = B.size();

    // 2D grid: (m+1) rows, (n+1) cols
    for (int i = 0; i <= m; ++i) {
        for (int j = 0; j <= n; ++j) {
            std::string labelRow = i == 0 ? "0" : std::string(1, A[i-1]);
            std::string labelCol = j == 0 ? "0" : std::string(1, B[j-1]);
            m_currentGridCells.push_back({i, j, 0, labelRow, labelCol, 0});
        }
    }

    auto getCellIdx = [&](int r, int c) { return r * (n + 1) + c; };

    // Zero columns base cases
    for (int i = 0; i <= m; ++i) {
        m_currentGridCells[getCellIdx(i, 0)].state = 2;
        m_cellsComputed++;
    }
    for (int j = 0; j <= n; ++j) {
        m_currentGridCells[getCellIdx(0, j)].state = 2;
        m_cellsComputed++;
    }

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            m_statesVisited++;
            int currIdx = getCellIdx(i, j);
            m_currentGridCells[currIdx].state = 1; // Filling

            if (A[i-1] == B[j-1]) {
                // Match case (dependency diagonal top-left)
                int diagIdx = getCellIdx(i-1, j-1);
                addDependencyStep(i, j, {{i-1, j-1}}, 3, "Characters Match: '" + std::string(1, A[i-1]) + "' == '" + std::string(1, B[j-1]) + "'");

                int val = m_currentGridCells[diagIdx].value + 1;
                m_currentGridCells[currIdx].value = val;
                m_currentGridCells[currIdx].state = 2;
                m_cellsComputed++;

                if (m_isEducationalMode) {
                    addExplainStep("[Teacher] LCS character match: Increment diagonal value. dp[i][j] = dp[i-1][j-1] + 1.");
                }
            } else {
                // Mismatch (dependency top and left)
                int topIdx = getCellIdx(i-1, j);
                int leftIdx = getCellIdx(i, j-1);
                addDependencyStep(i, j, {{i-1, j}, {i, j-1}}, 4, "Mismatch: taking max(top, left)");

                int val = std::max(m_currentGridCells[topIdx].value, m_currentGridCells[leftIdx].value);
                m_currentGridCells[currIdx].value = val;
                m_currentGridCells[currIdx].state = 2;
                m_cellsComputed++;
            }
        }
    }

    // Backtrack path
    int currRow = m;
    int currCol = n;
    while (currRow > 0 && currCol > 0) {
        int idx = getCellIdx(currRow, currCol);
        m_currentGridCells[idx].state = 4;

        if (A[currRow-1] == B[currCol-1]) {
            addBacktrackStep(currRow, currCol, -1, "Match char: '" + std::string(1, A[currRow-1]) + "' included in subsequence.");
            currRow--;
            currCol--;
        } else {
            int topIdx = getCellIdx(currRow-1, currCol);
            int leftIdx = getCellIdx(currRow, currCol-1);
            if (m_currentGridCells[topIdx].value >= m_currentGridCells[leftIdx].value) {
                currRow--;
            } else {
                currCol--;
            }
        }
    }
    if (currRow >= 0 && currCol >= 0) {
        m_currentGridCells[getCellIdx(currRow, currCol)].state = 4;
    }

    addSnapshotStep(-1, "LCS path backtracking complete.");
    addSuccessAlertStep("DP Completed", "LCS length is: " + std::to_string(m_currentGridCells[getCellIdx(m, n)].value), -1, "Completed.");
}

// 5. Coin Change
void DPVisualizerScreen::recordCoinChange() {
    int amount = m_inputValue; // target sum amount
    std::vector<int> coins = {1, 3, 4};
    int n = coins.size();

    // 2D grid representing coin alternatives vs capacity amounts (0 to amount)
    for (int i = 0; i < n; ++i) {
        for (int a = 0; a <= amount; ++a) {
            m_currentGridCells.push_back({i, a, 99999, "Coin" + std::to_string(coins[i]), "a" + std::to_string(a), 0});
        }
    }

    auto getCellIdx = [&](int r, int c) { return r * (amount + 1) + c; };

    // Amount 0 base cases
    for (int i = 0; i < n; ++i) {
        m_currentGridCells[getCellIdx(i, 0)].value = 0;
        m_currentGridCells[getCellIdx(i, 0)].state = 2;
        m_cellsComputed++;
    }

    addSnapshotStep(0, "Base case: 0 coins needed to sum to amount 0.");

    for (int i = 0; i < n; ++i) {
        for (int a = 1; a <= amount; ++a) {
            m_statesVisited++;
            int currIdx = getCellIdx(i, a);
            m_currentGridCells[currIdx].state = 1;

            int exclVal = i > 0 ? m_currentGridCells[getCellIdx(i-1, a)].value : 99999;
            int inclVal = 99999;

            if (a >= coins[i]) {
                int remIdx = getCellIdx(i, a - coins[i]);
                if (m_currentGridCells[remIdx].value != 99999) {
                    inclVal = m_currentGridCells[remIdx].value + 1;
                }
                
                std::vector<std::pair<int, int>> deps;
                if (i > 0) deps.push_back({i-1, a});
                deps.push_back({i, a - coins[i]});
                addDependencyStep(i, a, deps, 4, "Comparing coin usage combinations: exclude coin vs include coin");
            } else {
                std::vector<std::pair<int, int>> deps;
                if (i > 0) deps.push_back({i-1, a});
                addDependencyStep(i, a, deps, 4, "Coin " + std::to_string(coins[i]) + " exceeds capacity. Exclude.");
            }

            int val = std::min(exclVal, inclVal);
            m_currentGridCells[currIdx].value = val;
            m_currentGridCells[currIdx].state = 2;
            m_cellsComputed++;
        }
    }

    // Backtrack path
    int currRow = n - 1;
    int currCol = amount;
    while (currRow >= 0 && currCol > 0) {
        int idx = getCellIdx(currRow, currCol);
        m_currentGridCells[idx].state = 4;

        int exclVal = currRow > 0 ? m_currentGridCells[getCellIdx(currRow-1, currCol)].value : 99999;
        if (m_currentGridCells[idx].value != exclVal) {
            addBacktrackStep(currRow, currCol, -1, "Coin Change backtrack: Coin value " + std::to_string(coins[currRow]) + " was USED.");
            currCol -= coins[currRow];
        } else {
            currRow--;
        }
    }
    if (currRow >= 0) {
        m_currentGridCells[getCellIdx(currRow, currCol)].state = 4;
    }

    addSnapshotStep(-1, "Coin Change path backtracking complete.");
    addSuccessAlertStep("DP Completed", "Minimum coins count: " + std::to_string(m_currentGridCells[getCellIdx(n-1, amount)].value), -1, "Completed.");
}

// 6. Matrix Chain Multiplication
void DPVisualizerScreen::recordMatrixChain() {
    int n = m_inputValue; // matrix count (e.g. 4)
    std::vector<int> p = {10, 20, 30, 40, 30}; // dimensions

    // Triangular table: size (n+1) x (n+1)
    for (int i = 0; i <= n; ++i) {
        for (int j = 0; j <= n; ++j) {
            m_currentGridCells.push_back({i, j, 0, "M" + std::to_string(i), "M" + std::to_string(j), 0});
        }
    }

    auto getCellIdx = [&](int r, int c) { return r * (n + 1) + c; };

    // Diagonal base cases (len 1 costs = 0)
    for (int i = 1; i <= n; ++i) {
        m_currentGridCells[getCellIdx(i, i)].value = 0;
        m_currentGridCells[getCellIdx(i, i)].state = 2;
        m_cellsComputed++;
    }

    addSnapshotStep(0, "Diagonal base cases cost 0 scalar multiplications.");

    for (int len = 2; len <= n; ++len) {
        for (int i = 1; i <= n - len + 1; ++i) {
            int j = i + len - 1;
            m_statesVisited++;
            int currIdx = getCellIdx(i, j);
            m_currentGridCells[currIdx].state = 1;

            int minCost = 999999;
            for (int k = i; k < j; ++k) {
                int cost = m_currentGridCells[getCellIdx(i, k)].value +
                           m_currentGridCells[getCellIdx(k+1, j)].value +
                           p[i-1] * p[k] * p[j];
                
                addDependencyStep(i, j, {{i, k}, {k+1, j}}, 5, "Splitting chain parenthesization at split index: " + std::to_string(k));
                if (cost < minCost) {
                    minCost = cost;
                }
            }

            m_currentGridCells[currIdx].value = minCost;
            m_currentGridCells[currIdx].state = 2;
            m_cellsComputed++;
        }
    }

    m_currentGridCells[getCellIdx(1, n)].state = 4;
    addSnapshotStep(-1, "Matrix chain computation finished.");
    addSuccessAlertStep("DP Completed", "Minimum multiplications cost: " + std::to_string(m_currentGridCells[getCellIdx(1, n)].value), -1, "Completed.");
}

// 7. Edit Distance
void DPVisualizerScreen::recordEditDistance() {
    std::string A = "CAT";
    std::string B = "CARTS";
    int m = A.size();
    int n = B.size();

    // 2D grid: (m+1) rows, (n+1) cols
    for (int i = 0; i <= m; ++i) {
        for (int j = 0; j <= n; ++j) {
            std::string labelRow = i == 0 ? "0" : std::string(1, A[i-1]);
            std::string labelCol = j == 0 ? "0" : std::string(1, B[j-1]);
            m_currentGridCells.push_back({i, j, 0, labelRow, labelCol, 0});
        }
    }

    auto getCellIdx = [&](int r, int c) { return r * (n + 1) + c; };

    // Column / Row base cases
    for (int i = 0; i <= m; ++i) {
        m_currentGridCells[getCellIdx(i, 0)].value = i;
        m_currentGridCells[getCellIdx(i, 0)].state = 2;
        m_cellsComputed++;
    }
    for (int j = 0; j <= n; ++j) {
        m_currentGridCells[getCellIdx(0, j)].value = j;
        m_currentGridCells[getCellIdx(0, j)].state = 2;
        m_cellsComputed++;
    }

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            m_statesVisited++;
            int currIdx = getCellIdx(i, j);
            m_currentGridCells[currIdx].state = 1;

            if (A[i-1] == B[j-1]) {
                int diagIdx = getCellIdx(i-1, j-1);
                addDependencyStep(i, j, {{i-1, j-1}}, 1, "Characters match: '" + std::string(1, A[i-1]) + "'. Cost remains " + std::to_string(m_currentGridCells[diagIdx].value));
                m_currentGridCells[currIdx].value = m_currentGridCells[diagIdx].value;
                m_currentGridCells[currIdx].state = 2;
                m_cellsComputed++;
            } else {
                int removeIdx = getCellIdx(i-1, j);
                int insertIdx = getCellIdx(i, j-1);
                int replaceIdx = getCellIdx(i-1, j-1);
                
                addDependencyStep(i, j, {{i-1, j}, {i, j-1}, {i-1, j-1}}, 3, "Mismatch. Checking Remove, Insert, Replace operations");

                int minCost = std::min({m_currentGridCells[removeIdx].value,
                                        m_currentGridCells[insertIdx].value,
                                        m_currentGridCells[replaceIdx].value}) + 1;
                m_currentGridCells[currIdx].value = minCost;
                m_currentGridCells[currIdx].state = 2;
                m_cellsComputed++;
            }
        }
    }

    // Backtrack path
    int currRow = m;
    int currCol = n;
    while (currRow > 0 && currCol > 0) {
        int idx = getCellIdx(currRow, currCol);
        m_currentGridCells[idx].state = 4;

        if (A[currRow-1] == B[currCol-1]) {
            currRow--;
            currCol--;
        } else {
            int removeIdx = getCellIdx(currRow-1, currCol);
            int insertIdx = getCellIdx(currRow, currCol-1);
            int replaceIdx = getCellIdx(currRow-1, currCol-1);

            int val = m_currentGridCells[idx].value;
            if (val == m_currentGridCells[replaceIdx].value + 1) {
                addBacktrackStep(currRow, currCol, -1, "Backtrack: REPLACE character '" + std::string(1, A[currRow-1]) + "' with '" + std::string(1, B[currCol-1]) + "'");
                currRow--;
                currCol--;
            } else if (val == m_currentGridCells[removeIdx].value + 1) {
                addBacktrackStep(currRow, currCol, -1, "Backtrack: REMOVE character '" + std::string(1, A[currRow-1]) + "'");
                currRow--;
            } else {
                addBacktrackStep(currRow, currCol, -1, "Backtrack: INSERT character '" + std::string(1, B[currCol-1]) + "'");
                currCol--;
            }
        }
    }
    if (currRow >= 0 && currCol >= 0) {
        m_currentGridCells[getCellIdx(currRow, currCol)].state = 4;
    }

    addSnapshotStep(-1, "Edit Distance backtrack complete.");
    addSuccessAlertStep("DP Completed", "Minimum Edit Distance is: " + std::to_string(m_currentGridCells[getCellIdx(m, n)].value), -1, "Completed.");
}

// -------------------------------------------------------------
// PLAYBACK STEP TICK MAPPINGS
// -------------------------------------------------------------
void DPVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        m_activePseudocodeLine = -1;
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);

    // Sync grid cell structures
    m_currentGridCells = step.gridSnapshot;
    m_currentDependencies = step.dependencySnapshot;

    // Sync stats
    m_cellsComputed = step.cellsComputed;
    m_cacheHits = step.cacheHits;
    m_cacheMisses = step.cacheMisses;
    m_statesVisited = step.statesVisited;

    m_currentStepIndex++;
}

void DPVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_showAlert = false;

    m_currentGridCells.clear();
    m_currentDependencies.clear();

    m_cellsComputed = 0;
    m_cacheHits = 0;
    m_cacheMisses = 0;
    m_statesVisited = 0;
    m_elapsedTime = 0.f;

    m_explanationText.setString("DP grid ready. Select Run DP to solve.");
    m_activePseudocodeLine = -1;

    if (m_window) {
        updateUILayouts();
    }
}

void DPVisualizerScreen::onSizeChanged(int newSize) {
    onReset();
}

void DPVisualizerScreen::updateVisuals(float deltaTime) {
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

    if (m_isPlaying && !m_showAlert) {
        m_elapsedTime += deltaTime;
        m_stepAccumulator += deltaTime * m_animationSpeed;

        float stepDelay = 0.7f;
        if (m_stepAccumulator >= stepDelay) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }

    // Refresh live stats text labels
    m_statsComputedText.setString("Cells Computed: " + std::to_string(m_cellsComputed));
    m_statsHitsText.setString("Cache Hits: " + std::to_string(m_cacheHits));
    m_statsMissesText.setString("Cache Misses: " + std::to_string(m_cacheMisses));
    m_statsStatesText.setString("States Visited: " + std::to_string(m_statesVisited));

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Execution Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);

    // Sync input frame button text
    if (m_isValActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_valInputFrame->setLabel(displayStr);
    } else {
        m_valInputFrame->setLabel("Input: " + std::to_string(m_inputValue));
    }
}

void DPVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
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

void DPVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
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

    // 2. Draw DP Grid Table cells
    if (!m_currentGridCells.empty()) {
        // Find grid boundaries
        int maxRow = 0;
        int maxCol = 0;
        for (const auto& cell : m_currentGridCells) {
            maxRow = std::max(maxRow, cell.row);
            maxCol = std::max(maxCol, cell.col);
        }

        int rows = maxRow + 1;
        int cols = maxCol + 1;

        float cellWidth = std::min(m_gridPanelBounds.width / (cols + 1), 32.f);
        float cellHeight = std::min(m_gridPanelBounds.height / (rows + 1), 30.f);

        float totalGridW = cols * cellWidth;
        float totalGridH = rows * cellHeight;

        float startX = m_gridPanelBounds.left + (m_gridPanelBounds.width - totalGridW) / 2.f;
        float startY = m_gridPanelBounds.top + (m_gridPanelBounds.height - totalGridH) / 2.f;

        // Draw headers and label cells
        for (const auto& cell : m_currentGridCells) {
            float cx = startX + cell.col * cellWidth;
            float cy = startY + cell.row * cellHeight;

            sf::RectangleShape box(sf::Vector2f(cellWidth - 2.f, cellHeight - 2.f));
            box.setPosition(cx, cy);

            sf::Color cellBg = colors.cardBg;
            sf::Color borderCol = colors.border;
            float borderThick = 1.f;

            if (cell.state == 1) {
                cellBg = colors.warning; // Orange (Filling)
                borderCol = colors.warning;
            } else if (cell.state == 2) {
                cellBg = colors.accent; // Turquoise (Filled)
            } else if (cell.state == 3) {
                cellBg = sf::Color(194, 65, 12); // Amber (Dependency)
                borderCol = sf::Color(251, 146, 60);
                borderThick = 1.5f;
            } else if (cell.state == 4) {
                cellBg = colors.success; // Green (Solution backtrack)
                borderCol = colors.success;
                borderThick = 2.f;
            }

            box.setFillColor(cellBg);
            box.setOutlineColor(borderCol);
            box.setOutlineThickness(borderThick);
            window.draw(box);

            // Draw Value
            std::string valStr = std::to_string(cell.value);
            if (cell.value == -1) valStr = "-";
            else if (cell.value == -2) valStr = "?";
            else if (cell.value == 99999) valStr = "INF";

            sf::Text valText;
            valText.setFont(boldFont);
            valText.setString(valStr);
            valText.setCharacterSize(10);
            valText.setFillColor(colors.textPrimary);
            
            sf::FloatRect valBounds = valText.getLocalBounds();
            valText.setOrigin(valBounds.left + valBounds.width / 2.f, valBounds.top + valBounds.height / 2.f);
            valText.setPosition(cx + cellWidth / 2.f, cy + cellHeight / 2.f);
            window.draw(valText);

            // Draw Row/Col headers label
            if (cell.row == 0) {
                sf::Text colText;
                colText.setFont(regularFont);
                colText.setString(cell.labelCol);
                colText.setCharacterSize(8);
                colText.setFillColor(colors.textSecondary);
                
                sf::FloatRect colBounds = colText.getLocalBounds();
                colText.setOrigin(colBounds.left + colBounds.width / 2.f, colBounds.top + colBounds.height / 2.f);
                colText.setPosition(cx + cellWidth / 2.f, cy - 10.f);
                window.draw(colText);
            }
            if (cell.col == 0) {
                sf::Text rowText;
                rowText.setFont(regularFont);
                rowText.setString(cell.labelRow);
                rowText.setCharacterSize(8);
                rowText.setFillColor(colors.textSecondary);
                
                sf::FloatRect rowBounds = rowText.getLocalBounds();
                rowText.setOrigin(rowBounds.left + rowBounds.width / 2.f, rowBounds.top + rowBounds.height / 2.f);
                rowText.setPosition(cx - 15.f, cy + cellHeight / 2.f);
                window.draw(rowText);
            }
        }

        // Draw Dependency lines connecting cells
        for (const auto& link : m_currentDependencies) {
            sf::Vector2f fromPos(startX + link.fromCol * cellWidth + cellWidth / 2.f,
                                 startY + link.fromRow * cellHeight + cellHeight / 2.f);
            sf::Vector2f toPos(startX + link.toCol * cellWidth + cellWidth / 2.f,
                               startY + link.toRow * cellHeight + cellHeight / 2.f);

            sf::Vertex line[] = {
                sf::Vertex(fromPos, colors.warning),
                sf::Vertex(toPos, colors.warning)
            };
            window.draw(line, 2, sf::Lines);
        }
    }

    // 3. Draw statistics
    window.draw(m_statsTitleText);
    window.draw(m_statsComputedText);
    window.draw(m_statsHitsText);
    window.draw(m_statsMissesText);
    window.draw(m_statsStatesText);
    window.draw(m_statsTimeText);

    // 4. Draw pseudocode lines
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

    // 5. Draw overlay
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
void DPVisualizerScreen::addCellUpdateStep(int r, int c, int val, int line, const std::string& explanation) {
    DPStep step;
    step.type = DPStep::Type::CellUpdate;
    step.line = line;
    step.explanation = explanation;
    step.gridSnapshot = captureGridSnapshot();
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addCacheHitStep(int r, int c, int val, int line, const std::string& explanation) {
    DPStep step;
    step.type = DPStep::Type::ReadCache;
    step.line = line;
    step.explanation = explanation;
    
    auto snap = captureGridSnapshot();
    for (auto& cell : snap) {
        if (cell.row == r && cell.col == c) {
            cell.state = 4; // Solution (Green) cache glow
        }
    }
    step.gridSnapshot = snap;
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addDependencyStep(int r, int c, const std::vector<std::pair<int, int>>& deps, int line, const std::string& explanation) {
    m_currentDependencies.clear();
    auto snap = captureGridSnapshot();

    for (const auto& dep : deps) {
        m_currentDependencies.push_back({dep.first, dep.second, r, c});
        // Highlight source dependencies cells
        for (auto& cell : snap) {
            if (cell.row == dep.first && cell.col == dep.second) {
                cell.state = 3; // Dependency (Yellow)
            }
        }
    }

    DPStep step;
    step.type = DPStep::Type::DependencyHighlight;
    step.line = line;
    step.explanation = explanation;
    step.gridSnapshot = snap;
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addBacktrackStep(int r, int c, int line, const std::string& explanation) {
    DPStep step;
    step.type = DPStep::Type::BacktrackHighlight;
    step.line = line;
    step.explanation = explanation;
    step.gridSnapshot = captureGridSnapshot();
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addExplainStep(const std::string& explanation) {
    DPStep step;
    step.type = DPStep::Type::Explain;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    step.gridSnapshot = captureGridSnapshot();
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    DPStep step;
    step.type = DPStep::Type::SuccessAlert;
    step.line = line;
    step.explanation = msg;
    step.gridSnapshot = captureGridSnapshot();
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    DPStep step;
    step.type = DPStep::Type::FailAlert;
    step.line = line;
    step.explanation = msg;
    step.gridSnapshot = captureGridSnapshot();
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addSnapshotStep(int line, const std::string& explanation) {
    DPStep step;
    step.type = DPStep::Type::CellUpdate;
    step.line = line;
    step.explanation = explanation;
    step.gridSnapshot = captureGridSnapshot();
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

void DPVisualizerScreen::addHighlightStep(int line) {
    DPStep step;
    step.type = DPStep::Type::CellUpdate;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    step.gridSnapshot = captureGridSnapshot();
    step.dependencySnapshot = m_currentDependencies;

    step.cellsComputed = m_cellsComputed;
    step.cacheHits = m_cacheHits;
    step.cacheMisses = m_cacheMisses;
    step.statesVisited = m_statesVisited;
    m_steps.push_back(step);
}

} // namespace core
