#include "core/TreeVisualizerScreen.hpp"
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
#include <cmath>

namespace core {

TreeVisualizerScreen::TreeVisualizerScreen(sf::RenderWindow& window)
    : VisualizerBase("Tree Visualizer"),
      m_treeType(TreeType::BST),
      m_inputValue(25),
      m_isEducationalMode(true),
      m_currentStepIndex(0),
      m_stepAccumulator(0.f),
      m_activeVal(-1),
      m_parentVal(-1),
      m_grandParentVal(-1),
      m_siblingVal(-1),
      m_isValActive(false),
      m_cursorBlinkTime(0.f),
      m_capacity(12),
      m_nodeCount(0),
      m_treeHeight(0),
      m_leafCount(0),
      m_internalNodes(0),
      m_comparisonsCount(0),
      m_rotationsCount(0),
      m_traversalCount(0),
      m_elapsedTime(0.f),
      m_showAlert(false),
      m_activePseudocodeLine(-1),
      m_root(nullptr) {
    
    srand(static_cast<unsigned int>(time(nullptr)));
    initCommonUI(window);
    initUIComponents();
    loadTypeMode(TreeType::BST);
    triggerGenRandom();
}

void TreeVisualizerScreen::initUIComponents() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    m_explanationText.setFont(regularFont);
    m_explanationText.setCharacterSize(13);
    m_explanationText.setFillColor(colors.textSecondary);

    // 1. Selector type tabs
    std::vector<std::string> typeNames = {"Binary Search Tree", "AVL Tree"};
    for (size_t i = 0; i < typeNames.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(typeNames[i], sf::Vector2f(145.f, 26.f), 11);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        TreeType type = static_cast<TreeType>(i);
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

    // 3. Actions Row 1 (BST/AVL Operations)
    std::vector<std::string> opNames1 = {"Insert", "Delete", "Search", "Inorder", "Preorder", "Postorder", "Level-Order"};
    for (size_t i = 0; i < opNames1.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(opNames1[i], sf::Vector2f(85.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        if (i == 0) btn->setCallback([this]() { triggerInsert(); });
        else if (i == 1) btn->setCallback([this]() { triggerDelete(); });
        else if (i == 2) btn->setCallback([this]() { triggerSearch(); });
        else if (i == 3) btn->setCallback([this]() { triggerInorder(); });
        else if (i == 4) btn->setCallback([this]() { triggerPreorder(); });
        else if (i == 5) btn->setCallback([this]() { triggerPostorder(); });
        else btn->setCallback([this]() { triggerLevelOrder(); });
        
        m_opButtons.push_back(std::move(btn));
    }

    // 4. Actions Row 2 (Tree Utilities)
    std::vector<std::string> opNames2 = {"Clear Tree", "Random Tree", "Balanced Tree", "Skewed Tree"};
    for (size_t i = 0; i < opNames2.size(); ++i) {
        auto btn = std::make_unique<ui::Button>(opNames2[i], sf::Vector2f(110.f, 24.f), 10);
        btn->setColors(colors.panelBg, colors.cardBg, colors.textPrimary);
        
        if (i == 0) btn->setCallback([this]() { triggerClear(); });
        else if (i == 1) btn->setCallback([this]() { triggerGenRandom(); });
        else if (i == 2) btn->setCallback([this]() { triggerGenBalanced(); });
        else btn->setCallback([this]() { triggerGenSkewed(); });

        m_opButtons.push_back(std::move(btn));
    }

    // 5. Statistics Texts
    m_statsTitleText.setFont(boldFont);
    m_statsTitleText.setString("TREE METRICS");
    m_statsTitleText.setCharacterSize(15);
    m_statsTitleText.setFillColor(colors.accent);

    m_statsNodeCountText.setFont(regularFont);
    m_statsNodeCountText.setCharacterSize(13);
    m_statsNodeCountText.setFillColor(colors.textPrimary);

    m_statsHeightText.setFont(regularFont);
    m_statsHeightText.setCharacterSize(13);
    m_statsHeightText.setFillColor(colors.textPrimary);

    m_statsLeavesText.setFont(regularFont);
    m_statsLeavesText.setCharacterSize(13);
    m_statsLeavesText.setFillColor(colors.textPrimary);

    m_statsInternalText.setFont(regularFont);
    m_statsInternalText.setCharacterSize(13);
    m_statsInternalText.setFillColor(colors.textPrimary);

    m_statsCompsText.setFont(regularFont);
    m_statsCompsText.setCharacterSize(13);
    m_statsCompsText.setFillColor(colors.textPrimary);

    m_statsRotationsText.setFont(regularFont);
    m_statsRotationsText.setCharacterSize(13);
    m_statsRotationsText.setFillColor(colors.textPrimary);

    m_statsTraversalsText.setFont(regularFont);
    m_statsTraversalsText.setCharacterSize(13);
    m_statsTraversalsText.setFillColor(colors.textPrimary);

    m_statsTimeText.setFont(regularFont);
    m_statsTimeText.setCharacterSize(13);
    m_statsTimeText.setFillColor(colors.textPrimary);

    // 6. Alert Overlay panel
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

void TreeVisualizerScreen::updateUILayouts() {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f) return;

    float spacing = 8.f;
    float startX = bounds.left + 15.f;
    float startY = bounds.top + 10.f;

    // Row 1
    for (size_t i = 0; i < m_typeButtons.size(); ++i) {
        m_typeButtons[i]->setPosition(sf::Vector2f(startX + i * (145.f + spacing), startY));
    }

    // Row 2
    float row2Y = startY + 36.f;
    m_decValBtn->setPosition(sf::Vector2f(startX, row2Y));
    m_valInputFrame->setPosition(sf::Vector2f(startX + 24.f + spacing, row2Y));
    m_incValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing, row2Y));
    m_randValBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing + 24.f + spacing, row2Y));
    m_eduModeBtn->setPosition(sf::Vector2f(startX + 24.f + spacing + 90.f + spacing + 24.f + spacing + 65.f + 20.f, row2Y));

    // Row 3
    float row3Y = row2Y + 34.f;
    for (size_t i = 0; i < m_opButtons.size() - 4; ++i) {
        m_opButtons[i]->setPosition(sf::Vector2f(startX + i * (85.f + spacing), row3Y));
    }

    // Row 4
    float row4Y = row3Y + 32.f;
    for (size_t i = m_opButtons.size() - 4; i < m_opButtons.size(); ++i) {
        size_t idx = i - (m_opButtons.size() - 4);
        m_opButtons[i]->setPosition(sf::Vector2f(startX + idx * (110.f + spacing), row4Y));
    }

    // Explanation Y
    m_explanationText.setPosition(bounds.left + 15.f, row4Y + 35.f);

    // Live Stats right column layout
    sf::FloatRect infoBounds = m_infoPanel->getContentBounds();
    float statsStartY = infoBounds.top + 70.f;

    m_statsTitleText.setPosition(infoBounds.left, statsStartY);
    m_statsNodeCountText.setPosition(infoBounds.left, statsStartY + 22.f);
    m_statsHeightText.setPosition(infoBounds.left, statsStartY + 40.f);
    m_statsLeavesText.setPosition(infoBounds.left, statsStartY + 58.f);
    m_statsInternalText.setPosition(infoBounds.left, statsStartY + 76.f);
    m_statsCompsText.setPosition(infoBounds.left, statsStartY + 94.f);
    m_statsRotationsText.setPosition(infoBounds.left, statsStartY + 112.f);
    m_statsTraversalsText.setPosition(infoBounds.left, statsStartY + 130.f);
    m_statsTimeText.setPosition(infoBounds.left, statsStartY + 148.f);

    // Theory complex details
    m_theoryTitle.setPosition(infoBounds.left, statsStartY + 185.f);
    m_theoryBody.setPosition(infoBounds.left, statsStartY + 210.f);

    // Stacking pseudocode rows
    float pseudocodeStartY = statsStartY + 320.f;
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

void TreeVisualizerScreen::loadTypeMode(TreeType type) {
    m_treeType = type;
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

    // Complexity Theory details
    switch (type) {
        case TreeType::BST:
            m_theoryTitle.setString("Binary Search Tree");
            m_theoryBody.setString("A Binary Search Tree (BST) is a node-based binary tree data structure. Left subtrees contain values smaller than the root, and right subtrees contain values larger. Operations take average O(log N) but degrade to O(N) when skewed.");
            break;
        case TreeType::AVL:
            m_theoryTitle.setString("AVL Balanced Tree");
            m_theoryBody.setString("An AVL tree is a self-balancing binary search tree. Heights of two child subtrees of any node differ by at most one. Imbalances prompt single or double rotations (LL, RR, LR, RL), maintaining O(log N) worst-case complexities.");
            break;
    }

    loadPseudocode(type);
    onReset();
}

void TreeVisualizerScreen::loadPseudocode(TreeType type) {
    m_pseudocodeLines.clear();
    m_pseudocodeTexts.clear();

    switch (type) {
        case TreeType::BST:
            m_pseudocodeLines = {
                "insert(node, val):",
                "  if node == null: return Node(val)",
                "  if val < node.val: node.left = insert(node.left, val)",
                "  else: node.right = insert(node.right, val)",
                "  return node"
            };
            break;
        case TreeType::AVL:
            m_pseudocodeLines = {
                "insertAVL(node, val):",
                "  node = insertBST(node, val)",
                "  node.height = recalculateHeight(node)",
                "  balance = getBalanceFactor(node)",
                "  if balance > 1 & val < left: return rotateRight(node)",
                "  if balance < -1 & val > right: return rotateLeft(node)"
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
void TreeVisualizerScreen::adjustVal(int delta) {
    m_inputValue += delta;
    if (m_inputValue < 1) m_inputValue = 1;
    if (m_inputValue > 99) m_inputValue = 99;
}

void TreeVisualizerScreen::pickRandomValue() {
    m_inputValue = 5 + rand() % 90;
}

void TreeVisualizerScreen::commitValueInput() {
    m_isValActive = false;
    if (!m_inputTextBuffer.empty()) {
        int val = std::atoi(m_inputTextBuffer.c_str());
        if (val >= 1 && val <= 99) {
            m_inputValue = val;
        }
    }
}

// -------------------------------------------------------------
// STRUCTURAL LAYOUT HELPERS
// -------------------------------------------------------------
void TreeVisualizerScreen::calculateNodePositions(Node* root, float x, float y, float spacing, int depth, std::vector<TreeStepNode>& list) {
    if (!root) return;
    TreeStepNode sn;
    sn.value = root->value;
    sn.leftVal = root->left ? root->left->value : -1;
    sn.rightVal = root->right ? root->right->value : -1;
    sn.height = root->height;
    sn.balanceFactor = getBalanceFactor(root);
    sn.targetX = x;
    sn.targetY = y;
    sn.scale = 1.f;
    list.push_back(sn);

    calculateNodePositions(root->left, x - spacing, y + 60.f, spacing * 0.5f, depth + 1, list);
    calculateNodePositions(root->right, x + spacing, y + 60.f, spacing * 0.5f, depth + 1, list);
}

std::vector<TreeStepNode> TreeVisualizerScreen::captureSnapshot() {
    std::vector<TreeStepNode> list;
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    float rootX = bounds.left + bounds.width / 2.f;
    float rootY = bounds.top + 180.f + 25.f;
    float initialSpacing = bounds.width / 4.f;

    calculateNodePositions(m_root, rootX, rootY, initialSpacing, 0, list);
    return list;
}

// -------------------------------------------------------------
// BST/AVL ALGORITHMS INTERNALS
// -------------------------------------------------------------
int TreeVisualizerScreen::getHeight(Node* n) {
    return n ? n->height : 0;
}

int TreeVisualizerScreen::getBalanceFactor(Node* n) {
    return n ? getHeight(n->left) - getHeight(n->right) : 0;
}

TreeVisualizerScreen::Node* TreeVisualizerScreen::rotateRight(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;

    return x;
}

TreeVisualizerScreen::Node* TreeVisualizerScreen::rotateLeft(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;

    return y;
}

// -------------------------------------------------------------
// BST/AVL RECORDERS & TREE MODIFIERS
// -------------------------------------------------------------
TreeVisualizerScreen::Node* TreeVisualizerScreen::insertBST(Node* node, int val, int parentVal) {
    if (!node) {
        addHighlightStep(1); // if node == null
        Node* newNode = new Node(val);
        
        // Setup initial position dynamically for Growth Animation
        auto snapshot = captureInsertSnapshot(newNode, parentVal);
        
        // Find it in snapshot and set its scale to 0.05f to trigger Growth
        for (auto& sn : snapshot) {
            if (sn.value == val) {
                sn.scale = 0.05f;
            }
        }
        
        TreeStep step;
        step.type = TreeStep::Type::SnapshotState;
        step.activeNodeVal = val;
        step.parentNodeVal = parentVal;
        step.grandparentVal = -1;
        step.siblingVal = -1;
        step.line = 1;
        step.explanation = "Spawning node " + std::to_string(val) + " at target position.";
        step.nodesSnapshot = snapshot;
        m_steps.push_back(step);

        return newNode;
    }

    addCompareStep(node->value, parentVal, 2, "Comparing target value " + std::to_string(val) + " with current node " + std::to_string(node->value));
    
    if (val < node->value) {
        addHighlightStep(2); // if val < node.val
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Target " + std::to_string(val) + " < " + std::to_string(node->value) + ". Navigating into the left subtree partition.");
        }
        node->left = insertBST(node->left, val, node->value);
    } else if (val > node->value) {
        addHighlightStep(3); // else
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Target " + std::to_string(val) + " > " + std::to_string(node->value) + ". Navigating into the right subtree partition.");
        }
        node->right = insertBST(node->right, val, node->value);
    }

    node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
    return node;
}

TreeVisualizerScreen::Node* TreeVisualizerScreen::insertAVL(Node* node, int val, int parentVal) {
    if (!node) {
        addHighlightStep(1); // insert BST base
        Node* newNode = new Node(val);
        
        // Spawn growth simulation step
        auto snapshot = captureInsertSnapshot(newNode, parentVal);
        for (auto& sn : snapshot) {
            if (sn.value == val) sn.scale = 0.05f;
        }

        TreeStep step;
        step.type = TreeStep::Type::SnapshotState;
        step.activeNodeVal = val;
        step.parentNodeVal = parentVal;
        step.grandparentVal = -1;
        step.siblingVal = -1;
        step.line = 1;
        step.explanation = "Spawning new node " + std::to_string(val) + ".";
        step.nodesSnapshot = snapshot;
        m_steps.push_back(step);

        return newNode;
    }

    addCompareStep(node->value, parentVal, 1, "Traversing BST: comparing target value " + std::to_string(val) + " with current node " + std::to_string(node->value));
    
    if (val < node->value) {
        node->left = insertAVL(node->left, val, node->value);
    } else if (val > node->value) {
        node->right = insertAVL(node->right, val, node->value);
    } else {
        return node; // Duplicate values not allowed
    }

    // Heights update step
    node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
    addHeightUpdateStep(2, "Recalculating node " + std::to_string(node->value) + " height to: " + std::to_string(node->height));

    int balance = getBalanceFactor(node);
    addExplainStep("Node " + std::to_string(node->value) + " balance factor is: " + std::to_string(balance));

    // LL Case
    if (balance > 1 && val < node->left->value) {
        addRotationStep("LL Rotation", 4, "LL Case: Left subtree is heavier. Performing Right Rotation on node " + std::to_string(node->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] AVL Imbalance: Node " + std::to_string(node->value) + " left height is " + std::to_string(getHeight(node->left)) + " vs right height " + std::to_string(getHeight(node->right)) + ". BF = " + std::to_string(balance) + " (> 1). Running LL Rotation to restore height balance.");
        }
        m_rotationsCount++;
        return rotateRight(node);
    }

    // RR Case
    if (balance < -1 && val > node->right->value) {
        addRotationStep("RR Rotation", 5, "RR Case: Right subtree is heavier. Performing Left Rotation on node " + std::to_string(node->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] AVL Imbalance: Node " + std::to_string(node->value) + " right height is " + std::to_string(getHeight(node->right)) + " vs left height " + std::to_string(getHeight(node->left)) + ". BF = " + std::to_string(balance) + " (< -1). Running RR Rotation.");
        }
        m_rotationsCount++;
        return rotateLeft(node);
    }

    // LR Case
    if (balance > 1 && val > node->left->value) {
        addRotationStep("LR Rotation", 4, "LR Case: Left-Right imbalance. Double rotation needed. Left-Rotating child " + std::to_string(node->left->value));
        node->left = rotateLeft(node->left);
        addRotationStep("LL Rotation", 4, "Left rotation done. Now Right-Rotating node " + std::to_string(node->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] AVL LR Case: Double rotation. 1. Left rotate left-child. 2. Right rotate parent to pull up middle node and balance.");
        }
        m_rotationsCount++;
        return rotateRight(node);
    }

    // RL Case
    if (balance < -1 && val < node->right->value) {
        addRotationStep("RL Rotation", 5, "RL Case: Right-Left imbalance. Double rotation needed. Right-Rotating child " + std::to_string(node->right->value));
        node->right = rotateRight(node->right);
        addRotationStep("RR Rotation", 5, "Right rotation done. Now Left-Rotating node " + std::to_string(node->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] AVL RL Case: Double rotation. 1. Right rotate right-child. 2. Left rotate parent to restore tree depth symmetry.");
        }
        m_rotationsCount++;
        return rotateLeft(node);
    }

    return node;
}

TreeVisualizerScreen::Node* TreeVisualizerScreen::deleteBST(Node* node, int val, int parentVal) {
    if (!node) return nullptr;

    addCompareStep(node->value, parentVal, -1, "Traversing BST: comparing delete value " + std::to_string(val) + " with node " + std::to_string(node->value));

    if (val < node->value) {
        node->left = deleteBST(node->left, val, node->value);
    } else if (val > node->value) {
        node->right = deleteBST(node->right, val, node->value);
    } else {
        // Node found!
        addExplainStep("Node " + std::to_string(val) + " located. Commencing node deletion.");
        
        if (!node->left || !node->right) {
            Node* temp = node->left ? node->left : node->right;
            if (!temp) {
                // Leaf node
                addPopNodeStep(node->value, -1, -1, "Bypassing leaf node " + std::to_string(node->value) + ". Freeing node.");
                delete node;
                node = nullptr;
            } else {
                // One child
                addPopNodeStep(node->value, -1, -1, "Single child partition. Bypassing node " + std::to_string(node->value) + " and connecting child " + std::to_string(temp->value));
                Node* child = temp;
                delete node;
                node = child;
            }
        } else {
            // Double children successor swap
            Node* temp = node->right;
            while (temp->left) temp = temp->left;
            
            addExplainStep("Double child case. Swapping node value " + std::to_string(node->value) + " with inorder successor " + std::to_string(temp->value));
            node->value = temp->value;
            node->right = deleteBST(node->right, temp->value, node->value);
        }
    }

    if (node) {
        node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
    }
    return node;
}

TreeVisualizerScreen::Node* TreeVisualizerScreen::deleteAVL(Node* node, int val, int parentVal) {
    if (!node) return nullptr;

    addCompareStep(node->value, parentVal, -1, "Traversing AVL: comparing delete value " + std::to_string(val) + " with node " + std::to_string(node->value));

    if (val < node->value) {
        node->left = deleteAVL(node->left, val, node->value);
    } else if (val > node->value) {
        node->right = deleteAVL(node->right, val, node->value);
    } else {
        addExplainStep("Node " + std::to_string(val) + " located. Commencing node deletion.");
        if (!node->left || !node->right) {
            Node* temp = node->left ? node->left : node->right;
            if (!temp) {
                delete node;
                node = nullptr;
            } else {
                Node* child = temp;
                delete node;
                node = child;
            }
        } else {
            Node* temp = node->right;
            while (temp->left) temp = temp->left;
            node->value = temp->value;
            node->right = deleteAVL(node->right, temp->value, node->value);
        }
    }

    if (!node) return nullptr;

    node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
    int balance = getBalanceFactor(node);

    // Balancing AVL after deletions
    if (balance > 1 && getBalanceFactor(node->left) >= 0) {
        m_rotationsCount++;
        return rotateRight(node);
    }
    if (balance > 1 && getBalanceFactor(node->left) < 0) {
        m_rotationsCount++;
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && getBalanceFactor(node->right) <= 0) {
        m_rotationsCount++;
        return rotateLeft(node);
    }
    if (balance < -1 && getBalanceFactor(node->right) > 0) {
        m_rotationsCount++;
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

// -------------------------------------------------------------
// OPERATIONS RECORDER TRIGGERS
// -------------------------------------------------------------
void TreeVisualizerScreen::triggerInsert() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    // Check count capacity limits
    m_nodeCount = 0;
    m_currentTreeNodes = captureSnapshot();
    
    if (m_currentTreeNodes.size() >= static_cast<size_t>(m_capacity)) {
        addFailAlertStep("Tree Overflow", "Tree node capacity limits (12) reached.", -1, "Insert blocked.");
        return;
    }

    addSnapshotStep(-1, "Initializing Insert operation.");
    if (m_treeType == TreeType::BST) {
        m_root = insertBST(m_root, m_inputValue, -1);
    } else {
        m_root = insertAVL(m_root, m_inputValue, -1);
    }

    // Append final snapshot step to record visual growth completion
    addSnapshotStep(-1, "Insert completed. Tree structure updated.");
    addSuccessAlertStep("Insertion Completed", "Node value " + std::to_string(m_inputValue) + " inserted successfully.", -1, "Insertion Completed.");
}

void TreeVisualizerScreen::triggerDelete() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    if (!m_root) {
        addFailAlertStep("Tree Empty", "Tree is empty. Cannot delete.", -1, "Delete failed.");
        return;
    }

    m_currentTreeNodes = captureSnapshot();
    addSnapshotStep(-1, "Initializing Delete operation.");
    if (m_treeType == TreeType::BST) {
        m_root = deleteBST(m_root, m_inputValue, -1);
    } else {
        m_root = deleteAVL(m_root, m_inputValue, -1);
    }

    addSnapshotStep(-1, "Delete completed. Tree structure updated.");
    addSuccessAlertStep("Deletion Completed", "Deletion operation concluded.", -1, "Deletion Completed.");
}

void TreeVisualizerScreen::triggerSearch() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Target Search: " + std::to_string(m_inputValue));
    if (!m_root) {
        addFailAlertStep("Tree Empty", "Tree is empty. Search failed.", -1, "Search failed.");
        return;
    }

    Node* temp = m_root;
    bool found = false;
    int parentVal = -1;

    while (temp) {
        addCompareStep(temp->value, parentVal, -1, "Comparing node value " + std::to_string(temp->value) + " with search target " + std::to_string(m_inputValue));
        if (temp->value == m_inputValue) {
            addSuccessAlertStep("Search Found", "Target element " + std::to_string(m_inputValue) + " located successfully in tree.", -1, "Search Found!");
            found = true;
            break;
        } else if (m_inputValue < temp->value) {
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Target " + std::to_string(m_inputValue) + " < " + std::to_string(temp->value) + ". Moving down left child branch.");
            }
            parentVal = temp->value;
            temp = temp->left;
        } else {
            if (m_isEducationalMode) {
                addExplainStep("[Teacher] Target " + std::to_string(m_inputValue) + " > " + std::to_string(temp->value) + ". Moving down right child branch.");
            }
            parentVal = temp->value;
            temp = temp->right;
        }
    }

    if (!found) {
        addFailAlertStep("Search Not Found", "Target " + std::to_string(m_inputValue) + " is not present in the tree.", -1, "Search Not Found.");
    }
}

void TreeVisualizerScreen::triggerInorder() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Inorder traversal (Left, Root, Right).");
    if (!m_root) {
        addFailAlertStep("Tree Empty", "Tree is empty.", -1, "Traversal empty.");
        return;
    }

    // Run custom local stack recursion to record traversals
    std::vector<Node*> stack;
    Node* curr = m_root;
    while (curr || !stack.empty()) {
        while (curr) {
            stack.push_back(curr);
            curr = curr->left;
        }
        curr = stack.back();
        stack.pop_back();

        addTraversalStep(curr->value, -1, "Inorder Traverse node: " + std::to_string(curr->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Inorder reasoning: Visiting leftmost unvisited node first. Current value: " + std::to_string(curr->value));
        }

        curr = curr->right;
    }

    addSuccessAlertStep("Traversal Completed", "Inorder traversal completed successfully.", -1, "Traversal Completed.");
}

void TreeVisualizerScreen::triggerPreorder() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Preorder traversal (Root, Left, Right).");
    if (!m_root) {
        addFailAlertStep("Tree Empty", "Tree is empty.", -1, "Traversal empty.");
        return;
    }

    std::vector<Node*> stack;
    stack.push_back(m_root);
    while (!stack.empty()) {
        Node* curr = stack.back();
        stack.pop_back();

        addTraversalStep(curr->value, -1, "Preorder Traverse node: " + std::to_string(curr->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Preorder reasoning: Visiting parent node before traversing child branches. Value: " + std::to_string(curr->value));
        }

        if (curr->right) stack.push_back(curr->right);
        if (curr->left) stack.push_back(curr->left);
    }

    addSuccessAlertStep("Traversal Completed", "Preorder traversal completed successfully.", -1, "Traversal Completed.");
}

void TreeVisualizerScreen::triggerPostorder() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Postorder traversal (Left, Right, Root).");
    if (!m_root) {
        addFailAlertStep("Tree Empty", "Tree is empty.", -1, "Traversal empty.");
        return;
    }

    std::vector<Node*> stack1, stack2;
    stack1.push_back(m_root);
    while (!stack1.empty()) {
        Node* curr = stack1.back();
        stack1.pop_back();
        stack2.push_back(curr);

        if (curr->left) stack1.push_back(curr->left);
        if (curr->right) stack1.push_back(curr->right);
    }

    while (!stack2.empty()) {
        Node* curr = stack2.back();
        stack2.pop_back();
        addTraversalStep(curr->value, -1, "Postorder Traverse node: " + std::to_string(curr->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Postorder reasoning: Child branches must be completely traversed before parent visit. Value: " + std::to_string(curr->value));
        }
    }

    addSuccessAlertStep("Traversal Completed", "Postorder traversal completed successfully.", -1, "Traversal Completed.");
}

void TreeVisualizerScreen::triggerLevelOrder() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addExplainStep("Initializing Level Order traversal (Breadth-First).");
    if (!m_root) {
        addFailAlertStep("Tree Empty", "Tree is empty.", -1, "Traversal empty.");
        return;
    }

    std::queue<Node*> q;
    q.push(m_root);
    while (!q.empty()) {
        Node* curr = q.front();
        q.pop();

        addTraversalStep(curr->value, -1, "Level-Order Traverse node: " + std::to_string(curr->value));
        if (m_isEducationalMode) {
            addExplainStep("[Teacher] Level-Order: Visiting node levels sequentially from top to bottom, left to right. Value: " + std::to_string(curr->value));
        }

        if (curr->left) q.push(curr->left);
        if (curr->right) q.push(curr->right);
    }

    addSuccessAlertStep("Traversal Completed", "Level Order traversal completed successfully.", -1, "Traversal Completed.");
}

void TreeVisualizerScreen::triggerClear() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;
    m_isPlaying = true;

    addClearStep(-1, "Resetting Tree structures.");
}

void TreeVisualizerScreen::triggerGenRandom() {
    freeTree(m_root);
    m_root = nullptr;

    // Insert 5 to 7 random elements
    int count = 5 + rand() % 3;
    for (int i = 0; i < count; ++i) {
        int val = 10 + rand() % 85;
        // Avoid duplicate value inserts to maintain tree layout constraints
        Node* temp = m_root;
        bool exists = false;
        while (temp) {
            if (temp->value == val) { exists = true; break; }
            temp = val < temp->value ? temp->left : temp->right;
        }
        if (!exists) {
            if (m_treeType == TreeType::BST) m_root = insertBST(m_root, val, -1);
            else m_root = insertAVL(m_root, val, -1);
        }
    }
    onReset();
}

void TreeVisualizerScreen::triggerGenBalanced() {
    freeTree(m_root);
    m_root = nullptr;

    // Build balanced tree layout manually e.g. values (50, 25, 75, 12, 35, 60, 85)
    std::vector<int> vals = {50, 25, 75, 12, 35, 60, 85};
    for (int v : vals) {
        if (m_treeType == TreeType::BST) m_root = insertBST(m_root, v, -1);
        else m_root = insertAVL(m_root, v, -1);
    }
    onReset();
}

void TreeVisualizerScreen::triggerGenSkewed() {
    freeTree(m_root);
    m_root = nullptr;

    // Build left skewed tree values (80, 60, 40, 20)
    std::vector<int> vals = {80, 69, 52, 41};
    if (m_treeType == TreeType::AVL) {
        // AVL will auto balance! Show it skewing, then auto balancing.
        vals = {80, 60, 40, 20}; 
    }
    for (int v : vals) {
        if (m_treeType == TreeType::BST) m_root = insertBST(m_root, v, -1);
        else m_root = insertAVL(m_root, v, -1);
    }
    onReset();
}

void TreeVisualizerScreen::freeTree(Node* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// -------------------------------------------------------------
// PLAYBACK STEP TICK MAPPING
// -------------------------------------------------------------
void TreeVisualizerScreen::onStep() {
    if (m_steps.empty() || m_currentStepIndex >= static_cast<int>(m_steps.size())) {
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        const auto& colors = ThemeManager::getInstance().getColors();
        m_playPauseButton->setColors(colors.accent, colors.accentHover, colors.textPrimary);
        
        m_activeVal = -1;
        m_parentVal = -1;
        m_grandParentVal = -1;
        m_siblingVal = -1;
        m_activePseudocodeLine = -1;
        return;
    }

    const auto& step = m_steps[m_currentStepIndex];
    m_activePseudocodeLine = step.line;
    m_explanationText.setString(step.explanation);

    switch (step.type) {
        case TreeStep::Type::Compare:
            m_comparisonsCount++;
            m_activeVal = step.activeNodeVal;
            m_parentVal = step.parentNodeVal;
            m_grandParentVal = -1;
            m_siblingVal = -1;
            break;
        case TreeStep::Type::TraversalHighlight:
            m_traversalCount++;
            m_activeVal = step.activeNodeVal;
            m_parentVal = -1;
            m_grandParentVal = -1;
            m_siblingVal = -1;
            break;
        case TreeStep::Type::RotationAlert:
            m_activeVal = -1;
            m_parentVal = -1;
            m_grandParentVal = -1;
            m_siblingVal = -1;
            break;
        case TreeStep::Type::HeightUpdate:
            break;
        case TreeStep::Type::Explain:
            break;
        case TreeStep::Type::SuccessAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().success);
            break;
        case TreeStep::Type::FailAlert:
            m_showAlert = true;
            m_alertTitle = step.explanation;
            m_alertMessage = step.explanation;
            m_alertTitleText.setString(m_alertTitle);
            m_alertMsgText.setString(m_alertMessage);
            m_alertTitleText.setFillColor(ThemeManager::getInstance().getColors().danger);
            break;
        case TreeStep::Type::Clear:
            m_currentTreeNodes.clear();
            freeTree(m_root);
            m_root = nullptr;
            break;
        case TreeStep::Type::SnapshotState:
            // Load node structure snapshot coordinates
            // Align scale values to track animations
            for (const auto& snNode : step.nodesSnapshot) {
                // Find if node exists in m_currentTreeNodes
                auto it = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), 
                                       [&](const TreeStepNode& n) { return n.value == snNode.value; });
                if (it == m_currentTreeNodes.end()) {
                    // New Node! Spawn tiny for growth simulation
                    TreeStepNode newNode = snNode;
                    newNode.scale = 0.05f; // start growth
                    
                    // Spawn at parent's coordinate if parent exists
                    if (step.parentNodeVal != -1) {
                        auto pIt = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), 
                                               [&](const TreeStepNode& n) { return n.value == step.parentNodeVal; });
                        if (pIt != m_currentTreeNodes.end()) {
                            newNode.targetX = snNode.targetX;
                            newNode.targetY = snNode.targetY;
                            // visually slides down from parent coordinates
                            // start positions set to parent
                            // custom node animations
                        }
                    }
                    m_currentTreeNodes.push_back(newNode);
                } else {
                    // Existing node. Update targets
                    it->targetX = snNode.targetX;
                    it->targetY = snNode.targetY;
                    it->leftVal = snNode.leftVal;
                    it->rightVal = snNode.rightVal;
                    it->height = snNode.height;
                    it->balanceFactor = snNode.balanceFactor;
                    it->scale = snNode.scale; // normally 1.f
                }
            }

            // Remove deleted nodes
            m_currentTreeNodes.erase(
                std::remove_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), 
                    [&](const TreeStepNode& n) {
                        return std::find_if(step.nodesSnapshot.begin(), step.nodesSnapshot.end(),
                            [&](const TreeStepNode& sn) { return sn.value == n.value; }) == step.nodesSnapshot.end();
                    }),
                m_currentTreeNodes.end()
            );
            break;
    }

    m_currentStepIndex++;
}

void TreeVisualizerScreen::onReset() {
    m_steps.clear();
    m_currentStepIndex = 0;
    m_stepAccumulator = 0.f;

    m_activeVal = -1;
    m_parentVal = -1;
    m_grandParentVal = -1;
    m_siblingVal = -1;

    m_isValActive = false;
    m_showAlert = false;

    // Load active tree snapshot layout
    m_currentTreeNodes = captureSnapshot();

    // Statistics recalculations
    m_nodeCount = m_currentTreeNodes.size();
    m_treeHeight = getHeight(m_root);
    
    m_leafCount = 0;
    m_internalNodes = 0;
    for (const auto& n : m_currentTreeNodes) {
        if (n.leftVal == -1 && n.rightVal == -1) m_leafCount++;
        else m_internalNodes++;
    }

    m_comparisonsCount = 0;
    m_rotationsCount = 0;
    m_traversalCount = 0;
    m_elapsedTime = 0.f;

    m_explanationText.setString("Tree ready. Select operations above.");
    m_activePseudocodeLine = -1;

    if (m_window) {
        updateUILayouts();
    }
}

void TreeVisualizerScreen::updateVisuals(float deltaTime) {
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
            m_eduModeBtn->update(deltaTime, *m_window);
        }

        if (m_isValActive) {
            m_cursorBlinkTime += deltaTime;
        }
    }

    // Dynamic scale/position coordinate delta interpolations
    for (auto& n : m_currentTreeNodes) {
        if (n.scale < 1.f) {
            n.scale += (1.f - n.scale) * deltaTime * m_animationSpeed * 4.f;
            if (n.scale > 0.99f) n.scale = 1.f;
        }
    }

    if (m_isPlaying && !m_showAlert) {
        m_elapsedTime += deltaTime;
        m_stepAccumulator += deltaTime * m_animationSpeed;

        float stepDelay = 0.6f; // clear playback step speed
        if (m_stepAccumulator >= stepDelay) {
            m_stepAccumulator = 0.f;
            onStep();
        }
    }

    // Refresh live stats text labels
    m_statsNodeCountText.setString("Node Count: " + std::to_string(m_nodeCount));
    m_statsHeightText.setString("Tree Height: " + std::to_string(m_treeHeight));
    m_statsLeavesText.setString("Leaf Count: " + std::to_string(m_leafCount));
    m_statsInternalText.setString("Internal Nodes: " + std::to_string(m_internalNodes));
    m_statsCompsText.setString("Comparisons: " + std::to_string(m_comparisonsCount));
    
    if (m_treeType == TreeType::AVL) {
        m_statsRotationsText.setString("Rotations (AVL): " + std::to_string(m_rotationsCount));
    } else {
        m_statsRotationsText.setString("Rotations (AVL): N/A");
    }

    m_statsTraversalsText.setString("Traversal Count: " + std::to_string(m_traversalCount));

    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "Elapsed Time: %.2fs", m_elapsedTime);
    m_statsTimeText.setString(timeStr);

    // Sync input frame button text
    if (m_isValActive) {
        std::string displayStr = (static_cast<int>(m_cursorBlinkTime * 2.f) % 2 == 0) ? m_inputTextBuffer + "|" : m_inputTextBuffer;
        m_valInputFrame->setLabel(displayStr);
    } else {
        m_valInputFrame->setLabel("Value: " + std::to_string(m_inputValue));
    }
}

void TreeVisualizerScreen::handleVisualsEvent(sf::Event& event, sf::RenderWindow& window) {
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
        sf::Vector2f bounds = m_valInputFrame->getSize();
        sf::Vector2f pos = m_valInputFrame->getPosition();
        sf::FloatRect abounds(pos.x, pos.y, bounds.x, bounds.y);
        
        if (!abounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if (m_isValActive) {
                commitValueInput();
            }
        }
    }

    for (auto& btn : m_typeButtons) btn->handleEvent(event, window);
    for (auto& btn : m_opButtons) btn->handleEvent(event, window);
    m_decValBtn->handleEvent(event, window);
    m_incValBtn->handleEvent(event, window);
    m_randValBtn->handleEvent(event, window);
    m_valInputFrame->handleEvent(event, window);
    m_eduModeBtn->handleEvent(event, window);
}

void TreeVisualizerScreen::drawVisuals(sf::RenderWindow& window) {
    sf::FloatRect bounds = m_visualizationPanel->getContentBounds();
    if (bounds.width <= 0.f || bounds.height <= 0.f) return;

    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // 1. Draw UI controls
    for (auto& btn : m_typeButtons) window.draw(*btn);
    for (auto& btn : m_opButtons) window.draw(*btn);
    window.draw(*m_decValBtn);
    window.draw(*m_incValBtn);
    window.draw(*m_randValBtn);
    window.draw(*m_valInputFrame);
    window.draw(*m_eduModeBtn);
    window.draw(m_explanationText);

    // 2. Draw Edges (stretched lines connecting parent to child)
    for (const auto& n : m_currentTreeNodes) {
        if (n.leftVal != -1) {
            auto childIt = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), 
                                        [&](const TreeStepNode& c) { return c.value == n.leftVal; });
            if (childIt != m_currentTreeNodes.end()) {
                // Stretching edge line based on child growth scale
                sf::Vector2f parentPos(n.targetX, n.targetY);
                sf::Vector2f childPos(childIt->targetX, childIt->targetY);
                sf::Vector2f targetEdge = parentPos + (childPos - parentPos) * childIt->scale;

                sf::Vertex edge[] = {
                    sf::Vertex(parentPos, colors.border),
                    sf::Vertex(targetEdge, colors.border)
                };
                window.draw(edge, 2, sf::Lines);
            }
        }
        if (n.rightVal != -1) {
            auto childIt = std::find_if(m_currentTreeNodes.begin(), m_currentTreeNodes.end(), 
                                        [&](const TreeStepNode& c) { return c.value == n.rightVal; });
            if (childIt != m_currentTreeNodes.end()) {
                sf::Vector2f parentPos(n.targetX, n.targetY);
                sf::Vector2f childPos(childIt->targetX, childIt->targetY);
                sf::Vector2f targetEdge = parentPos + (childPos - parentPos) * childIt->scale;

                sf::Vertex edge[] = {
                    sf::Vertex(parentPos, colors.border),
                    sf::Vertex(targetEdge, colors.border)
                };
                window.draw(edge, 2, sf::Lines);
            }
        }
    }

    // 3. Draw Nodes (circles with size scaled for Growth Animations)
    float baseRadius = 18.f;
    for (const auto& n : m_currentTreeNodes) {
        float r = baseRadius * n.scale;
        if (r <= 0.5f) continue;

        sf::CircleShape circle(r);
        circle.setOrigin(r, r);
        circle.setPosition(n.targetX, n.targetY);
        circle.setFillColor(colors.cardBg);

        // Highlight coloring borders
        sf::Color outlineCol = colors.border;
        if (n.value == m_activeVal) outlineCol = colors.barActive;
        else if (n.value == m_parentVal) outlineCol = colors.warning;
        else if (n.value == m_grandParentVal) outlineCol = colors.danger;
        else if (n.value == m_siblingVal) outlineCol = colors.accent;
        else if (n.value == (m_root ? m_root->value : -1)) outlineCol = sf::Color(245, 158, 11); // Gold root
        else if (n.leftVal == -1 && n.rightVal == -1) outlineCol = sf::Color(139, 92, 246);      // Purple leaf

        circle.setOutlineColor(outlineCol);
        circle.setOutlineThickness(2.f);
        window.draw(circle);

        // Draw Value text inside node
        sf::Text valText;
        valText.setFont(boldFont);
        valText.setString(std::to_string(n.value));
        valText.setCharacterSize(static_cast<unsigned int>(11 * n.scale));
        valText.setFillColor(colors.textPrimary);

        sf::FloatRect valBounds = valText.getLocalBounds();
        valText.setOrigin(valBounds.left + valBounds.width / 2.f, valBounds.top + valBounds.height / 2.f);
        valText.setPosition(n.targetX, n.targetY);
        window.draw(valText);

        // Draw height and balance factor above node in AVL mode
        if (m_treeType == TreeType::AVL) {
            sf::Text avlText;
            avlText.setFont(regularFont);
            avlText.setString("H:" + std::to_string(n.height) + " BF:" + std::to_string(n.balanceFactor));
            avlText.setCharacterSize(9);
            avlText.setFillColor(colors.textSecondary);
            
            sf::FloatRect avlBounds = avlText.getLocalBounds();
            avlText.setOrigin(avlBounds.left + avlBounds.width / 2.f, avlBounds.top + avlBounds.height / 2.f);
            avlText.setPosition(n.targetX, n.targetY - r - 10.f);
            window.draw(avlText);
        }
    }

    // 4. Draw statistics in Info panel
    window.draw(m_statsTitleText);
    window.draw(m_statsNodeCountText);
    window.draw(m_statsHeightText);
    window.draw(m_statsLeavesText);
    window.draw(m_statsInternalText);
    window.draw(m_statsCompsText);
    window.draw(m_statsRotationsText);
    window.draw(m_statsTraversalsText);
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

    // 6. Draw overlay
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
void TreeVisualizerScreen::addCompareStep(int activeVal, int parentVal, int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::Compare;
    step.activeNodeVal = activeVal;
    step.parentNodeVal = parentVal;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addTraversalStep(int activeVal, int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::TraversalHighlight;
    step.activeNodeVal = activeVal;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addRotationStep(const std::string& rotName, int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::RotationAlert;
    step.activeNodeVal = -1;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = rotName + ": " + explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addHeightUpdateStep(int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::HeightUpdate;
    step.activeNodeVal = -1;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addExplainStep(const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::Explain;
    step.activeNodeVal = m_steps.empty() ? -1 : m_steps.back().activeNodeVal;
    step.parentNodeVal = m_steps.empty() ? -1 : m_steps.back().parentNodeVal;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = m_steps.empty() ? -1 : m_steps.back().line;
    step.explanation = explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addSuccessAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::SuccessAlert;
    step.activeNodeVal = -1;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = msg;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addFailAlertStep(const std::string& title, const std::string& msg, int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::FailAlert;
    step.activeNodeVal = -1;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = msg;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addSnapshotStep(int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::SnapshotState;
    step.activeNodeVal = -1;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addClearStep(int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::Clear;
    step.activeNodeVal = -1;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = std::vector<TreeStepNode>();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addHighlightStep(int line) {
    TreeStep step;
    step.type = TreeStep::Type::HighlightLine;
    step.activeNodeVal = m_steps.empty() ? -1 : m_steps.back().activeNodeVal;
    step.parentNodeVal = m_steps.empty() ? -1 : m_steps.back().parentNodeVal;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = m_steps.empty() ? "" : m_steps.back().explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::addPopNodeStep(int val, int idx, int line, const std::string& explanation) {
    TreeStep step;
    step.type = TreeStep::Type::Explain;
    step.activeNodeVal = val;
    step.parentNodeVal = -1;
    step.grandparentVal = -1;
    step.siblingVal = -1;
    step.line = line;
    step.explanation = explanation;
    step.nodesSnapshot = captureSnapshot();
    m_steps.push_back(step);
}

void TreeVisualizerScreen::onSizeChanged(int newSize) {
    // Tree height and structure remains dynamically built
}

TreeVisualizerScreen::Node* TreeVisualizerScreen::findNode(Node* root, int val) {
    if (!root) return nullptr;
    if (root->value == val) return root;
    if (val < root->value) return findNode(root->left, val);
    return findNode(root->right, val);
}

std::vector<TreeStepNode> TreeVisualizerScreen::captureInsertSnapshot(Node* newNode, int parentVal) {
    if (parentVal == -1) {
        Node* oldRoot = m_root;
        m_root = newNode;
        auto snapshot = captureSnapshot();
        m_root = oldRoot;
        return snapshot;
    }

    Node* parent = findNode(m_root, parentVal);
    Node* oldLeft = nullptr;
    Node* oldRight = nullptr;
    if (parent) {
        if (newNode->value < parent->value) {
            oldLeft = parent->left;
            parent->left = newNode;
        } else {
            oldRight = parent->right;
            parent->right = newNode;
        }
    }

    auto snapshot = captureSnapshot();

    if (parent) {
        if (newNode->value < parent->value) {
            parent->left = oldLeft;
        } else {
            parent->right = oldRight;
        }
    }

    return snapshot;
}

} // namespace core
