#ifndef TREE_VISUALIZER_SCREEN_HPP
#define TREE_VISUALIZER_SCREEN_HPP

#include "core/VisualizerBase.hpp"
#include <string>
#include <vector>
#include <memory>

namespace core {

enum class TreeType {
    BST,
    AVL
};

struct TreeStepNode {
    int value;
    int leftVal;
    int rightVal;
    int height;
    int balanceFactor;
    float targetX;
    float targetY;
    float scale; // for growth animations
};

struct TreeStep {
    enum class Type {
        Compare,
        TraversalHighlight,
        RotationAlert,
        HeightUpdate,
        Explain,
        SuccessAlert,
        FailAlert,
        Clear,
        SnapshotState,
        HighlightLine
    };
    Type type;
    int activeNodeVal;
    int parentNodeVal;
    int grandparentVal;
    int siblingVal;
    int line;
    std::string explanation;
    std::vector<TreeStepNode> nodesSnapshot; // full node structure in this step
};

class TreeVisualizerScreen : public VisualizerBase {
public:
    explicit TreeVisualizerScreen(sf::RenderWindow& window);
    virtual ~TreeVisualizerScreen() = default;

protected:
    virtual void drawVisuals(sf::RenderWindow& window) override;
    virtual void handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) override;
    virtual void updateVisuals(float deltaTime) override;
    virtual void onReset() override;
    virtual void onStep() override;
    virtual void onSizeChanged(int newSize) override;

private:
    TreeType m_treeType;
    int m_inputValue;
    bool m_isEducationalMode;

    std::vector<TreeStep> m_steps;
    int m_currentStepIndex;
    float m_stepAccumulator;

    // Highlight values
    int m_activeVal;
    int m_parentVal;
    int m_grandParentVal;
    int m_siblingVal;

    // Node state interpolation targets
    std::vector<TreeStepNode> m_currentTreeNodes;

    // UI Buttons
    std::vector<std::unique_ptr<ui::Button>> m_typeButtons; // BST, AVL
    std::vector<std::unique_ptr<ui::Button>> m_opButtons;   // Insert, Delete, Search, Inorder, Preorder, Postorder, Level, Clear, Random, Balanced, Skewed

    std::unique_ptr<ui::Button> m_decValBtn;
    std::unique_ptr<ui::Button> m_incValBtn;
    std::unique_ptr<ui::Button> m_randValBtn;
    std::unique_ptr<ui::Button> m_valInputFrame;
    std::unique_ptr<ui::Button> m_eduModeBtn;

    // Manual typing variables
    bool m_isValActive;
    std::string m_inputTextBuffer;
    float m_cursorBlinkTime;

    // Live Statistics
    int m_capacity; // cap = 12 nodes
    int m_nodeCount;
    int m_treeHeight;
    int m_leafCount;
    int m_internalNodes;
    int m_comparisonsCount;
    int m_rotationsCount;
    int m_traversalCount;
    float m_elapsedTime;

    sf::Text m_statsTitleText;
    sf::Text m_statsNodeCountText;
    sf::Text m_statsHeightText;
    sf::Text m_statsLeavesText;
    sf::Text m_statsInternalText;
    sf::Text m_statsCompsText;
    sf::Text m_statsRotationsText;
    sf::Text m_statsTraversalsText;
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

    // Structural helper classes inside TreeVisualizerScreen
    struct Node {
        int value;
        int height;
        Node* left;
        Node* right;
        Node(int val) : value(val), height(1), left(nullptr), right(nullptr) {}
    };

    Node* m_root;

    void initUIComponents();
    void updateUILayouts();
    void loadTypeMode(TreeType type);

    // Value adjustment helpers
    void adjustVal(int delta);
    void pickRandomValue();
    void commitValueInput();

    // BST / AVL Operations
    void triggerInsert();
    void triggerDelete();
    void triggerSearch();
    void triggerInorder();
    void triggerPreorder();
    void triggerPostorder();
    void triggerLevelOrder();
    void triggerClear();
    void triggerGenRandom();
    void triggerGenBalanced();
    void triggerGenSkewed();

    // Visual Node position builders (calculating non-overlapping tree coordinates)
    void calculateNodePositions(Node* root, float x, float y, float spacing, int depth, std::vector<TreeStepNode>& list);
    std::vector<TreeStepNode> captureSnapshot();

    // Steps queue helper nodes
    void addCompareStep(int activeVal, int parentVal, int line, const std::string& explanation);
    void addTraversalStep(int activeVal, int line, const std::string& explanation);
    void addRotationStep(const std::string& rotName, int line, const std::string& explanation);
    void addHeightUpdateStep(int line, const std::string& explanation);
    void addExplainStep(const std::string& explanation);
    void addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation);
    void addSnapshotStep(int line, const std::string& explanation);
    void addClearStep(int line, const std::string& explanation);
    void addHighlightStep(int line);
    void addPopNodeStep(int val, int idx, int line, const std::string& explanation);

    // Tree modifications & Recorders
    Node* insertBST(Node* node, int val, int parentVal);
    Node* deleteBST(Node* node, int val, int parentVal);
    Node* insertAVL(Node* node, int val, int parentVal);
    Node* deleteAVL(Node* node, int val, int parentVal);

    // Rotations (AVL balancing algorithms)
    Node* rotateRight(Node* y);
    Node* rotateLeft(Node* x);
    int getHeight(Node* n);
    int getBalanceFactor(Node* n);
    void freeTree(Node* node);

    // Snapshot helper routines
    Node* findNode(Node* root, int val);
    std::vector<TreeStepNode> captureInsertSnapshot(Node* newNode, int parentVal);

    // Dynamic pseudocode loading
    void loadPseudocode(TreeType type);
};

} // namespace core

#endif // TREE_VISUALIZER_SCREEN_HPP
