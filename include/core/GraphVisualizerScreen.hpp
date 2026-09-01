#ifndef GRAPH_VISUALIZER_SCREEN_HPP
#define GRAPH_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

struct GraphStepNode {
    float x;
    float y;
    char label;
    int visitOrder;
    int distance;
    bool isVisited;
    bool isHighlighted;
    int parentIdx;
};

struct GraphStepEdge {
    int u;
    int v;
    int weight;
    bool isHighlighted;
    bool isRelaxing;
};

struct GraphStep {
    enum class Type {
        Compare,
        Relax,
        Traversal,
        PriorityQueueUpdate,
        Explain,
        SuccessAlert,
        FailAlert,
        Clear,
        SnapshotState,
        HighlightLine
    };
    Type type;
    int activeNodeIdx;
    int activeEdgeU;
    int activeEdgeV;
    int line;
    std::string explanation;
    std::vector<GraphStepNode> nodesSnapshot;
    std::vector<GraphStepEdge> edgesSnapshot;
    
    // Auxiliary states
    int queueSize;
    int stackSize;
    int pqSize;
};

struct NodeModel {
    char label;
    float x;
    float y;
    bool isDragging;
    int visitOrder;
    int distance;
    int parentIdx;
};

struct EdgeModel {
    int u;
    int v;
    int weight;
};

class GraphVisualizerScreen : public VisualizerBase {
public:
    explicit GraphVisualizerScreen(sf::RenderWindow& window);
    virtual ~GraphVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    bool m_directed;
    bool m_weighted;
    int m_srcNodeIdx;
    int m_destNodeIdx;
    int m_dragNodeIdx;

    std::vector<NodeModel> m_nodes;
    std::vector<EdgeModel> m_edges;

    bool m_isEducationalMode;

    std::vector<GraphStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Visual highlights snapshot trackers
    int m_activeNodeIdx;
    int m_activeEdgeU;
    int m_activeEdgeV;
    std::vector<GraphStepNode> m_currentNodesSnapshot;
    std::vector<GraphStepEdge> m_currentEdgesSnapshot;

    // UI Buttons
    std::vector<std::unique_ptr<ui::Button>> m_typeToggles; // Directed/Undirected, Weighted/Unweighted
    std::vector<std::unique_ptr<ui::Button>> m_opButtons;   // BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal, Clear, Random, Connected, Dense, Sparse, Add Node, Add Edge, Del Edge

    std::unique_ptr<ui::Button> m_decValBtn;
    std::unique_ptr<ui::Button> m_incValBtn;
    std::unique_ptr<ui::Button> m_valInputFrame;
    std::unique_ptr<ui::Button> m_eduModeBtn;

    // Manual typing variables
    bool m_isValActive;
    std::string m_inputTextBuffer;
    float m_cursorBlinkTime;

    // Live Statistics
    int m_nodeCount;
    int m_edgeCount;
    int m_visitedCount;
    int m_queueSize;
    int m_stackSize;
    int m_pqSize;
    int m_distUpdates;
    int m_relaxCount;
    int m_comparisonsCount;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsNodesText;
    sf::Text m_statsEdgesText;
    sf::Text m_statsVisitedText;
    sf::Text m_statsQueueText;
    sf::Text m_statsStackText;
    sf::Text m_statsPqText;
    sf::Text m_statsUpdatesText;
    sf::Text m_statsRelaxText;
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

    void initUIComponents();
    void updateUILayouts();
    void toggleDirected();
    void toggleWeighted();

    // Variable adjustments helpers
    void adjustVal(int delta);
    void commitValueInput();

    // Graph operations triggers
    void triggerBFS();
    void triggerDFS();
    void triggerDijkstra();
    void triggerBellmanFord();
    void triggerPrim();
    void triggerKruskal();
    void triggerClear();
    void triggerGenRandom();
    void triggerGenConnected();
    void triggerGenDense();
    void triggerGenSparse();
    void triggerAddNode();
    void triggerAddEdge();
    void triggerDelEdge();

    // Step recorders helpers
    void addCompareStep(int activeNode, int parentNode, int line, const std::string& explanation);
    void addRelaxStep(int u, int v, int line, const std::string& explanation);
    void addTraversalStep(int activeNode, int line, const std::string& explanation);
    void addPQUpdateStep(int line, const std::string& explanation);
    void addExplainStep(const std::string& explanation);
    void addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addSnapshotStep(int line, const std::string& explanation);
    void addClearStep(int line, const std::string& explanation);
    void addHighlightStep(int line);

    std::vector<GraphStepNode> captureNodesSnapshot();
    std::vector<GraphStepEdge> captureEdgesSnapshot();

    // Union Find structural helpers for Kruskal MST
    struct Subset {
        int parent;
        int rank;
    };
    int findSubset(std::vector<Subset>& subsets, int i);
    void unionSubsets(std::vector<Subset>& subsets, int x, int y);

    // Dynamic pseudocode loading
    void loadPseudocode(const std::string& algo);
};

} // namespace core

#endif // GRAPH_VISUALIZER_SCREEN_HPP
