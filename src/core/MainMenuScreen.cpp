#include "core/MainMenuScreen.hpp"
#include "core/ResourceManager.hpp"
#include "core/ThemeManager.hpp"
#include "core/ScreenManager.hpp"
#include "core/AboutScreen.hpp"
#include "core/PlaceholderVisualizerScreen.hpp"
#include "core/SortingVisualizerScreen.hpp"
#include "core/SearchingVisualizerScreen.hpp"
#include "core/StackQueueVisualizerScreen.hpp"
#include "core/TreeVisualizerScreen.hpp"
#include "core/GraphVisualizerScreen.hpp"
#include "core/RecursionVisualizerScreen.hpp"
#include "core/DPVisualizerScreen.hpp"
#include "core/SettingsScreen.hpp"
#include "core/LinkedListVisualizerScreen.hpp"
#include <iostream>

namespace core {

MainMenuScreen::MainMenuScreen(sf::RenderWindow& window) : m_window(&window) {
    initUI();
    updateLayout(window.getSize());
}

void MainMenuScreen::initUI() {
    const auto& colors = ThemeManager::getInstance().getColors();
    sf::Font& boldFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Bold.ttf");
    sf::Font& regularFont = ResourceManager::getInstance().getFont("assets/fonts/Inter-Regular.ttf");

    // Title
    m_titleText.setFont(boldFont);
    m_titleText.setString("DSA VISUALIZER");
    m_titleText.setCharacterSize(28);
    m_titleText.setFillColor(colors.textPrimary);

    // Subtitle
    m_subtitleText.setFont(regularFont);
    m_subtitleText.setString("An interactive suite for learning, testing, and debugging structures & algorithms");
    m_subtitleText.setCharacterSize(13);
    m_subtitleText.setFillColor(colors.textSecondary);

    // Section Headers
    m_algoHeader.setFont(boldFont);
    m_algoHeader.setString("ALGORITHMS");
    m_algoHeader.setCharacterSize(13);
    m_algoHeader.setFillColor(colors.accent);

    m_dsHeader.setFont(boldFont);
    m_dsHeader.setString("DATA STRUCTURES");
    m_dsHeader.setCharacterSize(13);
    m_dsHeader.setFillColor(colors.accent);

    m_systemHeader.setFont(boldFont);
    m_systemHeader.setString("SYSTEM OPTIONS");
    m_systemHeader.setCharacterSize(13);
    m_systemHeader.setFillColor(colors.accent);

    // 1. Algorithms Section
    auto sorting = std::make_unique<ui::Card>("Sorting", "Visualize popular sorting techniques: Bubble Sort, Selection Sort, Insertion Sort, Merge Sort, and Quick Sort.");
    sorting->setStatus("Stable", colors.success);
    sorting->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<SortingVisualizerScreen>(*m_window));
    });
    m_algoCards.push_back(std::move(sorting));

    auto searching = std::make_unique<ui::Card>("Searching", "Step through searching methods like Linear Search and Binary Search on sorted/unsorted data.");
    searching->setStatus("Stable", colors.success);
    searching->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<SearchingVisualizerScreen>(*m_window));
    });
    m_algoCards.push_back(std::move(searching));

    auto recursion = std::make_unique<ui::Card>("Recursion", "Visualize recursive call trees, backtracking trails (N-Queens, Sudoku), and call stack trace frames.");
    recursion->setStatus("Beta", colors.info);
    recursion->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<RecursionVisualizerScreen>(*m_window));
    });
    m_algoCards.push_back(std::move(recursion));

    auto dp = std::make_unique<ui::Card>("Dynamic Programming", "Interactive DP grids, cache memoization hits, tabulation transitions (Knapsack, LCS, Coin Change, Edit Distance).");
    dp->setStatus("Beta", colors.info);
    dp->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<DPVisualizerScreen>(*m_window));
    });
    m_algoCards.push_back(std::move(dp));

    // 2. Data Structures Section
    auto linkedList = std::make_unique<ui::Card>("Linked List", "Dynamic visualization of Singly and Doubly Linked Lists: insertion, deletion, and traversal operations.");
    linkedList->setStatus("Stable", colors.success);
    linkedList->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<LinkedListVisualizerScreen>(*m_window));
    });
    m_dsCards.push_back(std::move(linkedList));

    auto stack = std::make_unique<ui::Card>("Stack", "LIFO (Last In, First Out) representation displaying Push, Pop, and Peek operations with color-coded steps.");
    stack->setStatus("Stable", colors.success);
    stack->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<StackQueueVisualizerScreen>(*m_window, StackQueueMode::Stack));
    });
    m_dsCards.push_back(std::move(stack));

    auto queue = std::make_unique<ui::Card>("Queue", "FIFO (First In, First Out) representation showing Enqueue and Dequeue actions on linear and circular buffers.");
    queue->setStatus("Stable", colors.success);
    queue->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<StackQueueVisualizerScreen>(*m_window, StackQueueMode::Queue));
    });
    m_dsCards.push_back(std::move(queue));

    auto tree = std::make_unique<ui::Card>("Binary Tree", "Interactive tree visualizer with node insert, delete, and traversals (Pre-order, In-order, Post-order).");
    tree->setStatus("Stable", colors.success);
    tree->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<TreeVisualizerScreen>(*m_window));
    });
    m_dsCards.push_back(std::move(tree));

    auto graph = std::make_unique<ui::Card>("Graph", "BFS/DFS traversals and shortest path algorithms (Dijkstra) on interactive node maps.");
    graph->setStatus("Stable", colors.success);
    graph->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<GraphVisualizerScreen>(*m_window));
    });
    m_dsCards.push_back(std::move(graph));

    // 3. System Options Section
    auto complexity = std::make_unique<ui::Card>("Complexities", "Compare time and space complexities (Big O notation) across different algorithms.");
    complexity->setStatus("System", colors.accent);
    complexity->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<PlaceholderVisualizerScreen>(*m_window, "Complexity Analysis", "Understand Big O complexity classes: O(1), O(log N), O(N), O(N log N), O(N^2), and O(2^N). This module lets you review theoretical runtime bounds of common structures."));
    });
    m_systemCards.push_back(std::move(complexity));

    auto about = std::make_unique<ui::Card>("About", "Details on development stack, CMake configurations, framework features, and project authors.");
    about->setStatus("Info", colors.info);
    about->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<AboutScreen>(*m_window));
    });
    m_systemCards.push_back(std::move(about));

    auto settings = std::make_unique<ui::Card>("Settings", "Adjust visual themes (Light/Dark/High Contrast) and customize default array sizes/parameters.");
    settings->setStatus("Config", colors.textSecondary);
    settings->setCallback([this]() {
        ScreenManager::getInstance().changeScreen(std::make_unique<SettingsScreen>(*m_window));
    });
    m_systemCards.push_back(std::move(settings));

    auto exitApp = std::make_unique<ui::Card>("Exit", "Close the application and cleanly release loaded resources.");
    exitApp->setStatus("Quit", colors.danger);
    exitApp->setCallback([this]() {
        m_window->close();
    });
    m_systemCards.push_back(std::move(exitApp));
}

void MainMenuScreen::updateLayout(const sf::Vector2u& windowSize) {
    float w = static_cast<float>(windowSize.x);
    
    float marginX = 40.f;
    float startY = 40.f;
    float itemSpacing = 20.f;
    float sectionSpacing = 25.f;
    float cardW = 220.f;
    float cardH = 140.f;

    // Header layout
    m_titleText.setPosition(marginX, startY);
    m_subtitleText.setPosition(marginX, startY + 36.f);

    float currentY = startY + 70.f;

    // Calculate maximum columns we can fit in available width
    float availableW = w - 2.f * marginX;
    int cols = std::max(1, static_cast<int>((availableW + itemSpacing) / (cardW + itemSpacing)));

    auto layoutSection = [&](sf::Text& header, std::vector<std::unique_ptr<ui::Card>>& cards) {
        header.setPosition(marginX, currentY);
        currentY += 24.f;

        int count = 0;
        for (auto& card : cards) {
            int row = count / cols;
            int col = count % cols;
            
            float cx = marginX + col * (cardW + itemSpacing);
            float cy = currentY + row * (cardH + itemSpacing);
            
            card->setPosition(sf::Vector2f(cx, cy));
            count++;
        }

        // Advance currentY by section height
        int totalRows = (static_cast<int>(cards.size()) + cols - 1) / cols;
        currentY += totalRows * (cardH + itemSpacing) - itemSpacing + sectionSpacing;
    };

    layoutSection(m_algoHeader, m_algoCards);
    layoutSection(m_dsHeader, m_dsCards);
    layoutSection(m_systemHeader, m_systemCards);
}

void MainMenuScreen::handleEvent(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::Resized) {
        sf::FloatRect visibleArea(0.f, 0.f, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
        window.setView(sf::View(visibleArea));
        updateLayout(window.getSize());
    }

    for (auto& card : m_algoCards) card->handleEvent(event, window);
    for (auto& card : m_dsCards) card->handleEvent(event, window);
    for (auto& card : m_systemCards) card->handleEvent(event, window);
}

void MainMenuScreen::update(float deltaTime) {
    if (m_window) {
        for (auto& card : m_algoCards) card->update(deltaTime, *m_window);
        for (auto& card : m_dsCards) card->update(deltaTime, *m_window);
        for (auto& card : m_systemCards) card->update(deltaTime, *m_window);
    }
}

void MainMenuScreen::draw(sf::RenderWindow& window) {
    window.clear(ThemeManager::getInstance().getColors().background);

    window.draw(m_titleText);
    window.draw(m_subtitleText);

    window.draw(m_algoHeader);
    for (auto& card : m_algoCards) window.draw(*card);

    window.draw(m_dsHeader);
    for (auto& card : m_dsCards) window.draw(*card);

    window.draw(m_systemHeader);
    for (auto& card : m_systemCards) window.draw(*card);
}

} // namespace core
