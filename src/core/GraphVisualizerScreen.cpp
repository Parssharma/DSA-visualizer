#include "core/GraphVisualizerScreen.hpp"
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
#include <queue>
#include <stack>
#include <cmath>

namespace core {

GraphVisualizerScreen::GraphVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("Graph Visualizer"),
      m_directed(false),
      m_weighted(true),
      m_srcNodeIdx(0),
      m_destNodeIdx(5),
      m_dragNodeIdx(-1),
      m_isEducationalMode(true),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_activeNodeIdx(-1),
      m_activeEdgeU(-1),
      m_activeEdgeV(-1),
      m_isValActive(false),
      m_cursorBlinkTime(0.f),
      m_nodeCount(0),
      m_edgeCount(0),
      m_visitedCount(0),
      m_queueSize(0),
      m_stackSize(0),
      m_pqSize(0),
      m_distUpdates(0),
      m_relaxCount(0),
      m_comparisonsCount(0),
      m_elapsedTime(0.f),
      m_showAlert(false),
      m_activePseudocodeLine(-1) {
    
    srand(static_cast<unsigned int>(time(nullptr)));
    initCommonUI(window);
    initUIComponents();
    
    // Build default 6 nodes circular layout
    triggerGenConnected();
}

void GraphVisualizerScreen::initUIComponents() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(12);
    m_explanationText.setFillColor(colors.textSecondary);

    // 1. Toggles row
    auto dirBtn = std::make_unique<ui::Button>("Directed: OFF", sf::Vector2f(140.f, 26.f), 11);
    dirBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    dirBtn->setCallback([this]() { toggleDirected(); });
    m_typeToggles.push_back(std::move(dirBtn));

    auto wgtBtn = std::make_unique<ui::Button>("Weighted: ON", sf::Vector2f(140.f, 26.f), 11);
    wgtBtn->setColors(colors.accent, colors.accentHover, colors.textPrimary);
    wgtBtn->setCallback([this]() { toggleWeighted(); });
    m_typeToggles.push_back(std::move(wgtBtn));

    // 2. Adjusters & Educational Mode
    m_decValBtn = std::make_unique<ui::Button>("-", sf::Vector2f(24.f, 24.f), 12);
    m_decValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_decValBtn->setCallback([this]() { adjustVal(-1); });

    m_incValBtn = std::make_unique<ui::Button>("+", sf::Vector2f(24.f, 24.f), 12);
    m_incValBtn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    m_incValBtn->setCallback([this]() { adjustVal(1); });

    m_valInputFrame = std::make_unique<ui::Button>("Src Node: A", sf::Vector2f(110.f, 24.f), 10);
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

    // 3. Action Algorithms
    std::vector<std::string> opNames1 = {"BFS", "DFS", "Dijkstra", "Bellman-Ford", "Prim", "Kruskal"};
    for (size_t i = 0; i < opNames1.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(opNames1[i], sf::Vector2f(85.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        if (i == 0) btn->setCallback([this]() { triggerBFS(); });
        else if (i == 1) btn->setCallback([this]() { triggerDFS(); });
        else if (i == 2) btn->setCallback([this]() { triggerDijkstra(); });
        else if (i == 3) btn->setCallback([this]() { triggerBellmanFord(); });
        else if (i == 4) btn->setCallback([this]() { triggerPrim(); });
        else btn->setCallback([this]() { triggerKruskal(); });

        m_opButtons.push_back(std::move(btn));
    }

    // 4. Action Utilities & Editing
    std::vector<std::string> opNames2 = {"Clear Graph", "Random Graph", "Connected", "Dense", "Sparse", "Add Node", "Add Edge", "Del Edge"};
    for (size_t i = 0; i < opNames2.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(opNames2[i], sf::Vector2f(110.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);

        if (i == 0) btn->setCallback([this]() { triggerClear(); });
        else if (i == 1) btn->setCallback([this]() { triggerGenRandom(); });
        else if (i == 2) btn->setCallback([this]() { triggerGenConnected(); });
        else if (i == 3) btn->setCallback([this]() { triggerGenDense(); });
        else if (i == 4) btn->setCallback([this]() { triggerGenSparse(); });
        else if (i == 5) btn->setCallback([this]() { triggerAddNode(); });
        else if (i == 6) btn->setCallback([this]() { triggerAddEdge(); });
        else btn->setCallback([this]() { triggerDelEdge(); });

        m_opButtons.push_back(std::move(btn));
    }

    // 5. Statistics
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("GRAPH METRICS");
    m_statsTitleText.setCharacterSize(14);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsNodesText.setFont(regularFont);
    m_statsNodesText.setCharacterSize(11);
    m_statsNodesText.setFillColor(colors.textPrimary);

    m_statsEdgesText.setFont(regularFont);
    m_statsEdgesText.setCharacterSize(11);
    m_statsEdgesText.setFillColor(colors.textPrimary);

    m_statsVisitedText.setFont(regularFont);
    m_statsVisitedText.setCharacterSize(11);
    m_statsVisitedText.setFillColor(colors.textPrimary);

    m_statsQueueText.setFont(regularFont);
    m_statsQueueText.setCharacterSize(11);
    m_statsQueueText.setFillColor(colors.textPrimary);

    m_statsStackText.setFont(regularFont);
    m_statsStackText.setCharacterSize(11);
    m_statsStackText.setFillColor(colors.textPrimary);

    m_statsPqText.setFont(regularFont);
    m_statsPqText.setCharacterSize(11);
    m_statsPqText.setFillColor(colors.textPrimary);

    m_statsUpdatesText.setFont(regularFont);
    m_statsUpdatesText.setCharacterSize(11);
    m_statsUpdatesText.setFillColor(colors.textPrimary);

    m_statsRelaxText.setFont(regularFont);
    m_statsRelaxText.setCharacterSize(11);
    m_statsRelaxText.setFillColor(colors.textPrimary);

    m_statsTimeText.setFont(regularFont);
    m_statsTimeText.setCharacterSize(11);
    m_statsTimeText.setFillColor(colors.textPrimary);

    // 6. Alert panel
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

void GraphVisualizerScreen::updateUILayouts() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    // Row 1
    for (size_t i = 0; i < m_typeToggles.size(); ++i) {
        m_typeToggles[i]->setPosition(sf::Vector2f(startX + i * (140.f + spacing), startY));
    }

    // Row 2
    float row2Y = startY + 36.f;
    m_decValBtn->setPosition(sf::Vector2f(startX, row2Y));
    m_valInputFrame->setPosition(sf::Vector2f(startX + 24.f + spacing, row2Y));
    m_incValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 110.f + spacing, row2Y));
    m_eduModeBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 110.f + spacing + 24.f + spacing + 15.f, row2Y));

    // Row 3
    float row3Y = row2Y + 34.f;
    for (size_t i = 0; i < m_opButtons.size() - 8; ++i) {
        m_opButtons[i]->setPosition(sf::Vector2f(startX + i * (85.f + spacing), row3Y));
    }

    // Row 4
    float row4Y = row3Y + 32.f;
    for (size_t i = m_opButtons.size() - 8; i < m_opButtons.size(); ++i) {
        size_t idx = i - (m_opButtons.size() - 8);
        m_opButtons[i]->setPosition(sf::Vector2f(startX + idx * (110.f + spacing), row4Y));
    }

    // Explanation text Y
    m_explanationText.setPosition(bounds.left + 15.f, row4Y + 35.f);

    // Live Stats right column layout
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 70.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsNodesText.setPosition(infoBounds.left, statsStartY + 20.f);
    m_statsEdgesText.setPosition(infoBounds.left, statsStartY + 36.f);
    m_statsVisitedText.setPosition(infoBounds.left, statsStartY + 52.f);
    m_statsQueueText.setPosition(infoBounds.left, statsStartY + 68.f);
    m_statsStackText.setPosition(infoBounds.left, statsStartY + 84.f);
    m_statsPqText.setPosition(infoBounds.left, statsStartY + 100.f);
    m_statsUpdatesText.setPosition(infoBounds.left, statsStartY + 116.f);
    m_statsRelaxText.setPosition(infoBounds.left, statsStartY + 132.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 148.f);

    // Theory complex details
    m_theoryTitle.setPosition(infoBounds.left, statsStartY + 180.f);
    m_theoryBody.setPosition(infoBounds.left, statsStartY + 205.f);

    // Stacking pseudocode rows
    float pseudocodeStartY = statsStartY + 310.f;
    for (size_t i = 0; i < m_pseudocodeTexts.size(); ++i) {
        m_pseudocodeTexts[i].setPosition(infoBounds.left + 10.f, pseudocodeStartY + i * 15.f);
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

void GraphVisualizerScreen::toggleDirected() {
    m_directed = !m_directed;
    const auto& colors = ThemeManager::getInstance().getColors();
    if (m_directed) {
        m_typeToggles[0]->setLabel("Directed: ON");
        m_typeToggles[0]->setColors(colors.accent, colors.accentHover, colors.textPrimary);
    } else {
        m_typeToggles[0]->setLabel("Directed: OFF");
        m_typeToggles[0]->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    }
    onReset();
}

void GraphVisualizerScreen::toggleWeighted() {
    m_weighted = !m_weighted;
    const auto& colors = ThemeManager::getInstance().getColors();
    if (m_weighted) {
        m_typeToggles[1]->setLabel("Weighted: ON");
        m_typeToggles[1]->setColors(colors.accent, colors.accentHover, colors.textPrimary);
    } else {
        m_typeToggles[1]->setLabel("Weighted: OFF");
        m_typeToggles[1]->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
    }
    onReset();
}

void GraphVisualizerScreen::adjustVal(int delta) {
    m_srcNodeIdx += delta;
    if (m_srcNodeIdx < 0) m_srcNodeIdx = 0;
    if (m_srcNodeIdx >= static_cast<int>(m_nodes.size())) m_srcNodeIdx = m_nodes.size() - 1;
}

void GraphVisualizerScreen::commitValueInput() {
    m_isValActive = false;
    if (!m_inputTextBuffer.empty()) {
        char label = std::toupper(m_inputTextBuffer[0]);
        if (label >= 'A' && label <= 'A' + m_nodes.size() - 1) {
            m_srcNodeIdx = label - 'A';
        }
    }
}

// -------------------------------------------------------------
// UNION FIND HELPERS (Kruskal MST)
// -------------------------------------------------------------
int GraphVisualizerScreen::findSubset(std::vector<Subset>& subsets, int i) {
    if (subsets[i].parent != i) {
        subsets[i].parent = findSubset(subsets, subsets[i].parent);
    }
    return subsets[i].parent;
}

void GraphVisualizerScreen::unionSubsets(std::vector<Subset>& subsets, int x, int y) {
    int xroot = findSubset(subsets, x);
    int yroot = findSubset(subsets, y);

    if (subsets[xroot].rank < subsets[yroot].rank) {
        subsets[xroot].parent = yroot;
    } else if (subsets[xroot].rank > subsets[yroot].rank) {
        subsets[yroot].parent = xroot;
    } else {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}

// -------------------------------------------------------------
// SNAPSHOT BUILDERS
// -------------------------------------------------------------
std::vector<GraphStepNode> GraphVisualizerScreen::captureNodesSnapshot() {
    std::vector<GraphStepNode> list;
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        GraphStepNode sn;
        sn.x = m_nodes[i].x;
        sn.y = m_nodes[i].y;
        sn.label = m_nodes[i].label;
        sn.visitOrder = m_nodes[i].visitOrder;
        sn.distance = m_nodes[i].distance;
        sn.isVisited = m_nodes[i].visitOrder != -1;
        sn.isHighlighted = (static_cast<int>(i) == m_activeNodeIdx);
        sn.parentIdx = m_nodes[i].parentIdx;
        list.push_back(sn);
    }
    return list;
}

std::vector<GraphStepEdge> GraphVisualizerScreen::captureEdgesSnapshot() {
    std::vector<GraphStepEdge> list;
    for (const auto& e : m_edges) {
        GraphStepEdge se;
        se.u = e.u;
        se.v = e.v;
        se.weight = e.weight;
        se.isHighlighted = (e.u == m_activeEdgeU && e.v == m_activeEdgeV);
        se.isRelaxing = false; // default
        list.push_back(se);
    }
    return list;
}

// -------------------------------------------------------------
// PRE-BUILT GRAPH GENERATOR PRESETS
// -------------------------------------------------------------
void GraphVisualizerScreen::triggerGenConnected() {
    m_nodes.clear();
    m_edges.clear();
    m_steps.clear();

    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    float centerX = bounds.left + bounds.width / 2.f;
    float centerY = bounds.top + 180.f + (bounds.height - 195.f) / 2.f;
    float radius = 100.f;

    // Create 6 nodes circular preset
    for (int i = 0; i < 6; ++i) {
        NodeModel n;
        n.label = 'A' + i;
        float angle = i * 60.f * 3.14159f / 180.f;
        n.x = centerX + cos(angle) * radius;
        n.y = centerY + sin(angle) * radius;
        n.isDragging = false;
        n.visitOrder = -1;
        n.distance = 99999;
        n.parentIdx = -1;
        m_nodes.push_back(n);
    }

    // Connect them in a cycle with cross-links
    m_edges.push_back({0, 1, 4});
    m_edges.push_back({1, 2, 6});
    m_edges.push_back({2, 3, 3});
    m_edges.push_back({3, 4, 8});
    m_edges.push_back({4, 5, 2});
    m_edges.push_back({5, 0, 5});
    m_edges.push_back({1, 4, 7});
    m_edges.push_back({0, 3, 9});

    onReset();
}

void GraphVisualizerScreen::triggerGenDense() {
    triggerGenConnected();
    // Add additional cross connections to make it dense
    m_edges.push_back({0, 2, 3});
    m_edges.push_back({1, 3, 5});
    m_edges.push_back({2, 4, 4});
    m_edges.push_back({3, 5, 7});
    m_edges.push_back({0, 4, 6});
    onReset();
}

void GraphVisualizerScreen::triggerGenSparse() {
    triggerGenConnected();
    // Reduce edges to minimal tree line
    m_edges.clear();
    m_edges.push_back({0, 1, 3});
    m_edges.push_back({1, 2, 4});
    m_edges.push_back({2, 3, 5});
    m_edges.push_back({3, 4, 2});
    m_edges.push_back({4, 5, 6});
    onReset();
}

void GraphVisualizerScreen::triggerGenRandom() {
    triggerGenConnected();
    m_edges.clear();
    // Connect randomly with random weights
    for (int i = 0; i < 6; ++i) {
        for (int j = i + 1; j < 6; ++j) {
            if (rand() % 2 == 0) {
                int w = 2 + rand() % 8;
                m_edges.push_back({i, j, w});
            }
        }
    }
    // Make sure it is connected
    if (m_edges.empty()) {
        m_edges.push_back({0, 1, 4});
    }
    onReset();
}

void GraphVisualizerScreen::triggerClear() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;
    addClearStep(-1, "Resetting Graph structures.");
}

void GraphVisualizerScreen::triggerAddNode() {
    if (m_nodes.size() >= 6) {
        m_showAlert = true;
        m_alertTitle = "Capacity Reached";
        m_alertMessage = "Graph node capacity has reached max limits (6).";
        m_alertTitleText.setString(m_alertTitle);
        m_alertMsgText.setString(m_alertMessage);
        return;
    }
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    NodeModel n;
    n.label = 'A' + m_nodes.size();
    n.x = bounds.left + 50.f + rand() % 300;
    n.y = bounds.top + 200.f + rand() % 150;
    n.isDragging = false;
    n.visitOrder = -1;
    n.distance = 99999;
    n.parentIdx = -1;
    m_nodes.push_back(n);
    onReset();
}

void GraphVisualizerScreen::triggerAddEdge() {
    if (m_nodes.size() < 2) return;
    // Add random edge
    int u = rand() % m_nodes.size();
    int v = rand() % m_nodes.size();
    if (u != v) {
        // Check if exists
        auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](const EdgeModel& e) {
            return (e.u == u && e.v == v) || (!m_directed && e.u == v && e.v == u);
        });
        if (it == m_edges.end()) {
            m_edges.push_back({u, v, 3 + rand() % 7});
        }
    }
    onReset();
}

void GraphVisualizerScreen::triggerDelEdge() {
    if (!m_edges.empty()) {
        m_edges.pop_back();
    }
    onReset();
}

// -------------------------------------------------------------
// ALGORITHM RECORDER TRIGGERS
// -------------------------------------------------------------
void GraphVisualizerScreen::triggerBFS() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Breadth-First Search (BFS) at source node " + std::string(1, m_nodes[m_srcNodeIdx].label));
    
    int n = m_nodes.size();
    std::vector<bool> visited(n, false);
    std::queue<int> q;

    q.push(m_srcNodeIdx);
    visited[m_srcNodeIdx] = true;
    int order = 1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        m_nodes[u].visitOrder = order++;
        addTraversalStep(u, 2, "Visiting node " + std::string(1, m_nodes[u].label) + ". Pulling from queue.");
        
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] BFS Traversal: Level-by-level expand. Dequeue front element and visit all its unvisited neighbors next.");
        }

        // Neighbors
        for (const auto& e : m_edges) {
            int v = -1;
            if (e.u == u) v = e.v;
            else if (!m_directed && e.v == u) v = e.u;

            if (v != -1 && !visited[v]) {
                visited[v] = true;
                q.push(v);
                
                // Set temporary highlights
                m_activeNodeIdx = v;
                m_activeEdgeU = u;
                m_activeEdgeV = v;
                
                addCompareStep(v, u, 3, "Discovered neighbor " + std::string(1, m_nodes[v].label) + ". Pushing to Queue.");
            }
        }
    }

    addSuccessAlertStep("BFS Completed", "Breadth-First Search traversal concluded successfully.", -1, "BFS Completed.");
}

void GraphVisualizerScreen::triggerDFS() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Depth-First Search (DFS) at source node " + std::string(1, m_nodes[m_srcNodeIdx].label));

    int n = m_nodes.size();
    std::vector<bool> visited(n, false);
    std::stack<int> s;

    s.push(m_srcNodeIdx);
    int order = 1;

    while (!s.empty()) {
        int u = s.top();
        s.pop();

        if (!visited[u]) {
            visited[u] = true;
            m_nodes[u].visitOrder = order++;
            addTraversalStep(u, 2, "Visiting node " + std::string(1, m_nodes[u].label) + ". Pulling from Stack.");

            if (m_isEducationalMode) {
                addExplainStep("[Teacher] DFS Traversal: Go deep before wide. Pop top node, visit, and push its unvisited neighbors onto the stack.");
            }

            for (const auto& e : m_edges) {
                int v = -1;
                if (e.u == u) v = e.v;
                else if (!m_directed && e.v == u) v = e.u;

                if (v != -1 && !visited[v]) {
                    s.push(v);
                    
                    m_activeNodeIdx = v;
                    m_activeEdgeU = u;
                    m_activeEdgeV = v;
                    addCompareStep(v, u, 3, "Discovered neighbor " + std::string(1, m_nodes[v].label) + ". Pushing onto Stack.");
                }
            }
        }
    }

    addSuccessAlertStep("DFS Completed", "Depth-First Search traversal concluded successfully.", -1, "DFS Completed.");
}

void GraphVisualizerScreen::triggerDijkstra() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Dijkstra Shortest Path at source node " + std::string(1, m_nodes[m_srcNodeIdx].label));

    int n = m_nodes.size();
    std::vector<int> dist(n, 99999);
    std::vector<bool> visited(n, false);
    
    dist[m_srcNodeIdx] = 0;
    m_nodes[m_srcNodeIdx].distance = 0;

    for (int i = 0; i < n; ++i) {
        // Find min distance vertex
        int minD = 99999;
        int u = -1;
        for (int j = 0; j < n; ++j) {
            if (!visited[j] && dist[j] < minD) {
                minD = dist[j];
                u = j;
            }
        }

        if (u == -1) break;

        visited[u] = true;
        m_nodes[u].visitOrder = i + 1;
        addTraversalStep(u, -1, "Visiting node " + std::string(1, m_nodes[u].label) + " with minimal tentative distance " + std::to_string(dist[u]));

        // Relax neighbors
        for (const auto& e : m_edges) {
            int v = -1;
            if (e.u == u) v = e.v;
            else if (!m_directed && e.v == u) v = e.u;

            if (v != -1 && !visited[v]) {
                int w = m_weighted ? e.weight : 1;
                addCompareStep(v, u, -1, "Checking edge (" + std::string(1, m_nodes[u].label) + ", " + std::string(1, m_nodes[v].label) + "). Distance through " + std::string(1, m_nodes[u].label) + " is " + std::to_string(dist[u] + w));
                
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    m_nodes[v].distance = dist[v];
                    m_nodes[v].parentIdx = u;

                    addRelaxStep(u, v, -1, "Edge relaxed! Distance to node " + std::string(1, m_nodes[v].label) + " updated to: " + std::to_string(dist[v]));
                    
                    if (m_isEducationalMode) {
                        addExplainStep("[Teacher] Dijkstra edge relaxation: Found a shorter path to node " + std::string(1, m_nodes[v].label) + " (dist = " + std::to_string(dist[v]) + ") through " + std::string(1, m_nodes[u].label) + ". Updating parent pointer.");
                    }
                }
            }
        }
    }

    addSuccessAlertStep("Shortest Path Found", "Dijkstra shortest path computed successfully.", -1, "Dijkstra Completed.");
}

void GraphVisualizerScreen::triggerBellmanFord() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Bellman-Ford Shortest Path.");
    
    int n = m_nodes.size();
    std::vector<int> dist(n, 99999);
    dist[m_srcNodeIdx] = 0;
    m_nodes[m_srcNodeIdx].distance = 0;

    // Loop V-1 times
    for (int i = 0; i < n - 1; ++i) {
        addExplainStep("Bellman-Ford iteration: " + std::to_string(i + 1));
        for (const auto& e : m_edges) {
            int u = e.u;
            int v = e.v;
            int w = m_weighted ? e.weight : 1;

            // Direct relax
            if (dist[u] != 99999 && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                m_nodes[v].distance = dist[v];
                m_nodes[v].parentIdx = u;
                addRelaxStep(u, v, -1, "Relaxed edge (" + std::string(1, m_nodes[u].label) + ", " + std::string(1, m_nodes[v].label) + "). Distance to " + std::string(1, m_nodes[v].label) + " updated to " + std::to_string(dist[v]));
            }

            // Undirected reverse relax
            if (!m_directed && dist[v] != 99999 && dist[v] + w < dist[u]) {
                dist[u] = dist[v] + w;
                m_nodes[u].distance = dist[u];
                m_nodes[u].parentIdx = v;
                addRelaxStep(v, u, -1, "Relaxed edge (" + std::string(1, m_nodes[v].label) + ", " + std::string(1, m_nodes[u].label) + "). Distance to " + std::string(1, m_nodes[u].label) + " updated to " + std::to_string(dist[u]));
            }
        }
    }

    addSuccessAlertStep("Shortest Path Found", "Bellman-Ford paths computed successfully.", -1, "Bellman-Ford Completed.");
}

void GraphVisualizerScreen::triggerPrim() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Prim Minimum Spanning Tree.");
    if (m_directed) {
        addFailAlertStep("Invalid Mode", "Prim MST algorithm requires Undirected Graph.", -1, "Prim failed.");
        return;
    }

    int n = m_nodes.size();
    std::vector<bool> inMST(n, false);
    std::vector<int> key(n, 99999);
    std::vector<int> parent(n, -1);

    key[m_srcNodeIdx] = 0;
    
    for (int i = 0; i < n; ++i) {
        // Find min key vertex
        int minK = 99999;
        int u = -1;
        for (int j = 0; j < n; ++j) {
            if (!inMST[j] && key[j] < minK) {
                minK = key[j];
                u = j;
            }
        }

        if (u == -1) break;

        inMST[u] = true;
        m_nodes[u].visitOrder = i + 1;
        addTraversalStep(u, -1, "Adding node " + std::string(1, m_nodes[u].label) + " to Spanning Tree.");

        if (parent[u] != -1) {
            addRelaxStep(parent[u], u, -1, "MST edge selected: (" + std::string(1, m_nodes[parent[u]].label) + ", " + std::string(1, m_nodes[u].label) + ")");
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Prim MST: Connect nearest outer node u. Edge weight is minimal among all crossing boundaries edges.");
            }
        }

        // Neighbors
        for (const auto& e : m_edges) {
            int v = -1;
            if (e.u == u) v = e.v;
            else if (e.v == u) v = e.u;

            if (v != -1 && !inMST[v]) {
                int w = m_weighted ? e.weight : 1;
                addCompareStep(v, u, -1, "Checking boundary edge (" + std::string(1, m_nodes[u].label) + ", " + std::string(1, m_nodes[v].label) + "). Key = " + std::to_string(w));
                if (w < key[v]) {
                    key[v] = w;
                    parent[v] = u;
                    m_nodes[v].parentIdx = u;
                }
            }
        }
    }

    addSuccessAlertStep("MST Completed", "Prim minimum spanning tree constructed successfully.", -1, "Prim Completed.");
}

void GraphVisualizerScreen::triggerKruskal() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Kruskal Minimum Spanning Tree.");
    if (m_directed) {
        addFailAlertStep("Invalid Mode", "Kruskal MST requires Undirected Graph.", -1, "Kruskal failed.");
        return;
    }

    int n = m_nodes.size();
    std::vector<EdgeModel> sortedEdges = m_edges;
    // Sort edges by weight
    std::sort(sortedEdges.begin(), sortedEdges.end(), [](const EdgeModel& a, const EdgeModel& b) {
        return a.weight < b.weight;
    });

    std::vector<Subset> subsets(n);
    for (int i = 0; i < n; ++i) {
        subsets[i].parent = i;
        subsets[i].rank = 0;
    }

    int edgeIdx = 0;
    int mstEdges = 0;

    while (mstEdges < n - 1 && edgeIdx < static_cast<int>(sortedEdges.size())) {
        const auto& e = sortedEdges[edgeIdx++];
        int u = e.u;
        int v = e.v;

        int x = findSubset(subsets, u);
        int y = findSubset(subsets, v);

        // Highlight active edge being checked
        m_activeEdgeU = u;
        m_activeEdgeV = v;
        addCompareStep(u, v, -1, "Checking minimal edge weight: (" + std::string(1, m_nodes[u].label) + ", " + std::string(1, m_nodes[v].label) + ") = " + std::to_string(e.weight));

        if (x != y) {
            mstEdges++;
            unionSubsets(subsets, x, y);
            
            // Mark node visit
            m_nodes[u].visitOrder = mstEdges;
            m_nodes[v].visitOrder = mstEdges + 1;
            m_nodes[v].parentIdx = u; // set parent mapping for final highlight

            addRelaxStep(u, v, -1, "Safe edge! Adding edge (" + std::string(1, m_nodes[u].label) + ", " + std::string(1, m_nodes[v].label) + ") to MST.");
            
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Kruskal MST: Edge connects components (" + std::string(1, m_nodes[u].label) + ", " + std::string(1, m_nodes[v].label) + ") without creating cycles. Unioning subsets.");
            }
        } else {
            addCompareStep(u, v, -1, "Cycle detected! Skipping edge (" + std::string(1, m_nodes[u].label) + ", " + std::string(1, m_nodes[v].label) + ").");
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Union-Find Cycle alert: Both nodes have root " + std::to_string(x) + ". Connecting them forms a cycle.");
            }
        }
    }

    addSuccessAlertStep("MST Completed", "Kruskal MST built successfully.", -1, "Kruskal Completed.");
}

// -------------------------------------------------------------
// PLAYBACK STEP TICK IMPLEMENTATIONS
// -------------------------------------------------------------
void GraphVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        
        m_activeNodeIdx = -1;
        m_activeEdgeU = -1;
        m_activeEdgeV = -1;
        m_activePseudocodeLine = -1;
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);

    switch (step.type) {
        case GraphStep::Type::Compare:
            m_activeNodeIdx = step.activeNodeIdx;
            m_activeEdgeU = step.activeEdgeU;
            m_activeEdgeV = step.activeEdgeV;
            m_comparisonsCount++;
            break;
        case GraphStep::Type::Relax:
            m_relaxCount++;
            m_distUpdates++;
            m_activeNodeIdx = step.activeNodeIdx;
            m_activeEdgeU = step.activeEdgeU;
            m_activeEdgeV = step.activeEdgeV;
            break;
        case GraphStep::Type::Traversal:
            m_activeNodeIdx = step.activeNodeIdx;
            m_activeEdgeU = -1;
            m_activeEdgeV = -1;
            m_visitedCount++;
            break;
        case GraphStep::Type::PriorityQueueUpdate:
            break;
        case GraphStep::Type::Explain:
            break;
        case GraphStep::Type::SuccessAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().success);
            break;
        case GraphStep::Type::FailAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().danger);
            break;
        case GraphStep::Type::Clear:
            m_nodes.clear();
            m_edges.clear();
            onReset();
            break;
        case GraphStep::Type::SnapshotState:
            break;
        case GraphStep::Type::HighlightLine:
            break;
    }

    // Sync snapshot nodes properties from step
    if (!step.nodesSnapshot.empty()) {
        m_currentNodesSnapshot = step.nodesSnapshot;
    }
    if (!step.edgesSnapshot.empty()) {
        m_currentEdgesSnapshot = step.edgesSnapshot;
    }

    // Update stats sizes
    m_queueSize = step.queueSize;
    m_stackSize = step.stackSize;
    m_pqSize = step.pqSize;

    m_currentStepIndex++;
}

void GraphVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_activeNodeIdx = -1;
    m_activeEdgeU = -1;
    m_activeEdgeV = -1;

    m_showAlert = false;

    // Reset node variables
    for (auto& n : m_nodes) {
        n.visitOrder = -1;
        n.distance = 99999;
        n.parentIdx = -1;
    }

    m_currentNodesSnapshot = captureNodesSnapshot();
    m_currentEdgesSnapshot = captureEdgesSnapshot();

    m_nodeCount = m_nodes.size();
    m_edgeCount = m_edges.size();
    m_visitedCount = 0;
    m_queueSize = 0;
    m_stackSize = 0;
    m_pqSize = 0;
    m_distUpdates = 0;
    m_relaxCount = 0;
    m_comparisonsCount = 0;
    m_elapsedTime = 0.f;

    m_explanationText.setString("Graph ready. Select algorithms or edit node vertices.");
    m_activePseudocodeLine = -1;

    if (m_window) {
        updateUILayouts();
    }
}

void GraphVisualizerScreen::onSizeChanged(int newSize) {
    onReset();
}

void GraphVisualizerScreen::updateVisuals(float deltaTime) {
    if (m_window) {
        if (m_showAlert) {
            m_alertResetBtn->update(deltaTime, *m_window);
        } else {
            for (auto& btn : m_typeToggles) btn->update(deltaTime, *m_window);
            for (auto& btn : m_opButtons) btn->update(deltaTime, *m_window);
            m_decValBtn->update(deltaTime, *m_window);
            m_incValBtn->update(deltaTime, *m_window);
            m_valInputFrame->update(deltaTime, *m_window);
            m_eduModeBtn->update(deltaTime, *m_window);
        }

        if (m_isValActive) {
            m_cursorBlinkTime += deltaTime;
        }

        // Interactive Node drag coordinates updater
        if (m_dragNodeIdx != -1) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(*m_window);
            sf::Vector2f localPos = m_window->mapPixelToCoords(mousePos);
            
            // Clamp drag to visual panel content bounds
            sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
            float nx = std::max(bounds.left + 25.f, std::min(bounds.left + bounds.width - 25.f, localPos.x));
            float ny = std::max(bounds.top + 180.f, std::min(bounds.top + bounds.height - 25.f, localPos.y));
            
            m_nodes[m_dragNodeIdx].x = nx;
            m_nodes[m_dragNodeIdx].y = ny;

            // Update live snapshot
            if (!m_isPlaying) {
                m_currentNodesSnapshot = captureNodesSnapshot();
            }
        }
    }

    if (m_isPlaying && !m_showAlert) {
        m_elapsedTime += deltaTime;
        m_stepAccumulator += deltaTime * m_animationSpeed;

        float stepDelay = 0.65f; // clear playback step speed
        if (m_stepAccumulator >= stepDelay) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }

    // Refresh live stats text labels
    m_statsNodesText.setString("Nodes Count: " + std::to_string(m_nodeCount));
    m_statsEdgesText.setString("Edges Count: " + std::to_string(m_edgeCount));
    m_statsVisitedText.setString("Visited Nodes: " + std::to_string(m_visitedCount));
    m_statsQueueText.setString("Queue Size: " + std::to_string(m_queueSize));
    m_statsStackText.setString("Stack Size: " + std::to_string(m_stackSize));
    m_statsPqText.setString("Priority Queue Size: " + std::to_string(m_pqSize));
    m_statsUpdatesText.setString("Distance Updates: " + std::to_string(m_distUpdates));
    m_statsRelaxText.setString("Relaxations Count: " + std::to_string(m_relaxCount));

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Elapsed Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);

    // Sync input frame button text
    if (m_isValActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_valInputFrame->setLabel(displayStr);
    } else {
        std::string label(1, 'A' + m_srcNodeIdx);
        m_valInputFrame->setLabel("Src Node: " + label);
    }
}

void GraphVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
    if (m_showAlert) {
        m_alertResetBtn->handleEvent(event, window);
        return;
    }

    // Capture manual variable input box typing
    if (m_isValActive && event.type == sf::Event::TextEntered) {
        sf::Uint32 code = event.text.unicode;
        if (code == 8) {
            if (!m_inputTextBuffer.empty()) m_inputTextBuffer.pop_back();
        } else if (code == 13) {
            commitValueInput();
        } else if (code >= 'a' && code <= 'z') {
            m_inputTextBuffer += static_cast<char>(code);
        } else if (code >= 'A' && code <= 'Z') {
            m_inputTextBuffer += static_cast<char>(code);
        }
        return;
    }

    // Dismiss focus click away
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

    // Click-and-drag mouse events
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f localPos = window.mapPixelToCoords(mousePos);

        for (size_t i = 0; i < m_nodes.size(); ++i) {
            float dx = m_nodes[i].x - localPos.x;
            float dy = m_nodes[i].y - localPos.y;
            if (sqrt(dx*dx + dy*dy) < 22.f) {
                m_dragNodeIdx = i;
                m_nodes[i].isDragging = true;
                break;
            }
        }
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (m_dragNodeIdx != -1) {
            m_nodes[m_dragNodeIdx].isDragging = false;
            m_dragNodeIdx = -1;
        }
    }

    for (auto& btn : m_typeToggles) btn->handleEvent(event, window);
    for (auto& btn : m_opButtons) btn->handleEvent(event, window);
    m_decValBtn->handleEvent(event, window);
    m_incValBtn->handleEvent(event, window);
    m_valInputFrame->handleEvent(event, window);
    m_eduModeBtn->handleEvent(event, window);
}

void GraphVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // 1. Draw controls
    for (auto& btn : m_typeToggles) window.draw(*btn);
    for (auto& btn : m_opButtons) window.draw(*btn);
    window.draw(*m_decValBtn);
    window.draw(*m_incValBtn);
    window.draw(*m_valInputFrame);
    window.draw(*m_eduModeBtn);
    window.draw(m_explanationText);

    // 2. Draw Edges
    for (const auto& e : m_currentEdgesSnapshot) {
        if (e.u < 0 || e.u >= static_cast<int>(m_currentNodesSnapshot.size())) continue;
        if (e.v < 0 || e.v >= static_cast<int>(m_currentNodesSnapshot.size())) continue;

        sf::Vector2f p1(m_currentNodesSnapshot[e.u].x, m_currentNodesSnapshot[e.u].y);
        sf::Vector2f p2(m_currentNodesSnapshot[e.v].x, m_currentNodesSnapshot[e.v].y);

        // Highlight color
        sf::Color edgeCol = colors.border;
        if (e.isHighlighted) {
            edgeCol = colors.accent; // active edge
        } else if (e.isRelaxing) {
            edgeCol = colors.warning; // glowing relaxation edge
        } else if (m_currentNodesSnapshot[e.v].parentIdx == e.u || m_currentNodesSnapshot[e.u].parentIdx == e.v) {
            edgeCol = colors.success; // MST / shortest path parent edge
        }

        // Draw Line
        sf::Vertex line[] = {
            sf::Vertex(p1, edgeCol),
            sf::Vertex(p2, edgeCol)
        };
        window.draw(line, 2, sf::Lines);

        // Draw arrowhead if Directed Graph mode
        if (m_directed) {
            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float length = sqrt(dx*dx + dy*dy);
            if (length > 0.f) {
                sf::Vector2f dir = sf::Vector2f(dx / length, dy / length);
                // Arrowhead offset to node boundary
                sf::Vector2f arrowPos = p2 - dir * 22.f;

                sf::CircleShape arrowTip(4.f, 3);
                arrowTip.setOrigin(4.f, 4.f);
                float angle = atan2(dir.y, dir.x) * 180.f / 3.14159f;
                arrowTip.setRotation(angle + 90.f);
                arrowTip.setPosition(arrowPos);
                arrowTip.setFillColor(edgeCol);
                window.draw(arrowTip);
            }
        }

        // Draw Edge Weight
        if (m_weighted) {
            sf::Vector2f midPoint = (p1 + p2) * 0.5f;
            sf::Text weightText;
            weightText.setFont(regularFont);
            weightText.setString(std::to_string(e.weight));
            weightText.setCharacterSize(10);
            weightText.setFillColor(colors.textSecondary);
            
            sf::FloatRect wtBounds = weightText.getLocalBounds();
            weightText.setOrigin(wtBounds.left + wtBounds.width / 2.f, wtBounds.top + wtBounds.height / 2.f);
            // Draw background rectangle for legibility
            sf::RectangleShape rect(sf::Vector2f(wtBounds.width + 4.f, wtBounds.height + 4.f));
            rect.setOrigin(rect.getSize().x / 2.f, rect.getSize().y / 2.f);
            rect.setPosition(midPoint);
            rect.setFillColor(colors.panelBg);
            window.draw(rect);

            weightText.setPosition(midPoint);
            window.draw(weightText);
        }
    }

    // 3. Draw Nodes (circles with labels and visit order / distances)
    float radius = 18.f;
    for (size_t i = 0; i < m_currentNodesSnapshot.size(); ++i) {
        const auto& n = m_currentNodesSnapshot[i];

        sf::CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(n.x, n.y);
        circle.setFillColor(colors.cardBg);

        // Highlight color border
        sf::Color outlineCol = colors.border;
        if (n.isHighlighted) outlineCol = colors.barActive;
        else if (n.isVisited) outlineCol = colors.accent;
        else if (static_cast<int>(i) == m_srcNodeIdx) outlineCol = sf::Color(245, 158, 11); // Gold source

        circle.setOutlineColor(outlineCol);
        circle.setOutlineThickness(2.f);
        window.draw(circle);

        // Draw Node label text
        sf::Text labelText;
        labelText.setFont(boldFont);
        labelText.setString(std::string(1, n.label));
        labelText.setCharacterSize(12);
        labelText.setFillColor(colors.textPrimary);
        
        sf::FloatRect lblBounds = labelText.getLocalBounds();
        labelText.setOrigin(lblBounds.left + lblBounds.width / 2.f, lblBounds.top + lblBounds.height / 2.f);
        labelText.setPosition(n.x, n.y);
        window.draw(labelText);

        // Draw Node visit order or Dijkstra distance above node
        std::string infoStr = "";
        if (n.visitOrder != -1) {
            infoStr = "#" + std::to_string(n.visitOrder);
        }
        if (n.distance != 99999) {
            if (!infoStr.empty()) infoStr += " ";
            infoStr += "d:" + std::to_string(n.distance);
        }

        if (!infoStr.empty()) {
            sf::Text infoText;
            infoText.setFont(regularFont);
            infoText.setString(infoStr);
            infoText.setCharacterSize(9);
            infoText.setFillColor(colors.textSecondary);

            sf::FloatRect infoBounds = infoText.getLocalBounds();
            infoText.setOrigin(infoBounds.left + infoBounds.width / 2.f, infoBounds.top + infoBounds.height / 2.f);
            infoText.setPosition(n.x, n.y - radius - 10.f);
            window.draw(infoText);
        }
    }

    // 4. Draw statistics in Info panel
    window.draw(m_statsTitleText);
    window.draw(m_statsNodesText);
    window.draw(m_statsEdgesText);
    window.draw(m_statsVisitedText);
    window.draw(m_statsQueueText);
    window.draw(m_statsStackText);
    window.draw(m_statsPqText);
    window.draw(m_statsUpdatesText);
    window.draw(m_statsRelaxText);
    window.draw(m_statsTimeText);

    // 5. Draw pseudocode lines
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
// STEP RECORDING ACCUMULATORS
// -------------------------------------------------------------
void GraphVisualizerScreen::addCompareStep(int activeNode, int parentNode, int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::Compare;
    step.activeNodeIdx = activeNode;
    step.activeEdgeU = parentNode;
    step.activeEdgeV = activeNode;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();
    
    step.queueSize = m_queueSize;
    step.stackSize = m_stackSize;
    step.pqSize = m_pqSize;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addRelaxStep(int u, int v, int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::Relax;
    step.activeNodeIdx = v;
    step.activeEdgeU = u;
    step.activeEdgeV = v;
    step.line = line;
    step.explanation = explanation;
    
    // Nodes / Edges Snapshot
    step.nodesSnapshot = captureNodesSnapshot();
    auto snapshotEdges = captureEdgesSnapshot();
    for (auto& se : snapshotEdges) {
        if ((se.u == u && se.v == v) || (!m_directed && se.u == v && se.v == u)) {
            se.isRelaxing = true; // glow
        }
    }
    step.edgesSnapshot = snapshotEdges;
    
    step.queueSize = m_queueSize;
    step.stackSize = m_stackSize;
    step.pqSize = m_pqSize;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addTraversalStep(int activeNode, int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::Traversal;
    step.activeNodeIdx = activeNode;
    step.activeEdgeU = -1;
    step.activeEdgeV = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();

    step.queueSize = m_queueSize;
    step.stackSize = m_stackSize;
    step.pqSize = m_pqSize;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addPQUpdateStep(int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::PriorityQueueUpdate;
    step.activeNodeIdx = -1;
    step.activeEdgeU = -1;
    step.activeEdgeV = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();

    step.queueSize = m_queueSize;
    step.stackSize = m_stackSize;
    step.pqSize = m_pqSize;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addExplainStep(const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::Explain;
    step.activeNodeIdx = m_steps.empty() ? -1 : m_steps.back().activeNodeIdx;
    step.activeEdgeU = m_steps.empty() ? -1 : m_steps.back().activeEdgeU;
    step.activeEdgeV = m_steps.empty() ? -1 : m_steps.back().activeEdgeV;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();

    step.queueSize = m_queueSize;
    step.stackSize = m_stackSize;
    step.pqSize = m_pqSize;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::SuccessAlert;
    step.activeNodeIdx = -1;
    step.activeEdgeU = -1;
    step.activeEdgeV = -1;
    step.line = line;
    step.explanation = msg;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();

    step.queueSize = 0;
    step.stackSize = 0;
    step.pqSize = 0;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::FailAlert;
    step.activeNodeIdx = -1;
    step.activeEdgeU = -1;
    step.activeEdgeV = -1;
    step.line = line;
    step.explanation = msg;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();

    step.queueSize = 0;
    step.stackSize = 0;
    step.pqSize = 0;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addSnapshotStep(int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::SnapshotState;
    step.activeNodeIdx = -1;
    step.activeEdgeU = -1;
    step.activeEdgeV = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();

    step.queueSize = m_queueSize;
    step.stackSize = m_stackSize;
    step.pqSize = m_pqSize;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addClearStep(int line, const std::string& explanation) {
    GraphStep step;
    step.type = GraphStep::Type::Clear;
    step.activeNodeIdx = -1;
    step.activeEdgeU = -1;
    step.activeEdgeV = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = std::vector<GraphStepNode>();
    step.edgesSnapshot = std::vector<GraphStepEdge>();

    step.queueSize = 0;
    step.stackSize = 0;
    step.pqSize = 0;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::addHighlightStep(int line) {
    GraphStep step;
    step.type = GraphStep::Type::HighlightLine;
    step.activeNodeIdx = m_steps.empty() ? -1 : m_steps.back().activeNodeIdx;
    step.activeEdgeU = m_steps.empty() ? -1 : m_steps.back().activeEdgeU;
    step.activeEdgeV = m_steps.empty() ? -1 : m_steps.back().activeEdgeV;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    step.nodesSnapshot = captureNodesSnapshot();
    step.edgesSnapshot = captureEdgesSnapshot();

    step.queueSize = m_queueSize;
    step.stackSize = m_stackSize;
    step.pqSize = m_pqSize;
    m_steps.push_back(step);
}

void GraphVisualizerScreen::loadPseudocode(const std::string& algo) {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    if (algo == "BFS") {
        m_pseudocodeLines = {
            "procedure BFS(G, start):",
            "  Q = new Queue(); visited[start] = true",
            "  Q.push(start)",
            "  while Q is not empty:",
            "    u = Q.pop(); visit(u)",
            "    for each neighbor v of u:",
            "      if not visited[v]:",
            "        visited[v] = true; Q.push(v)"
        };
    } else if (algo == "DFS") {
        m_pseudocodeLines = {
            "procedure DFS(G, start):",
            "  S = new Stack(); S.push(start)",
            "  while S is not empty:",
            "    u = S.pop()",
            "    if not visited[u]:",
            "      visit(u); visited[u] = true",
            "      for each neighbor v of u: S.push(v)"
        };
    } else {
        // Dijkstra / Prim
        m_pseudocodeLines = {
            "procedure Dijkstra(G, start):",
            "  dist[start] = 0; Q = new PriorityQueue()",
            "  while Q is not empty:",
            "    u = Q.popMin()",
            "    for each neighbor v of u:",
            "      if dist[u] + w(u,v) < dist[v]:",
            "        dist[v] = dist[u] + w(u,v)"
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
    updateUILayouts();
}

} // namespace core
