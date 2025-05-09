// Include necessary libraries
#include <iostream>         // For input/output
#include <vector>           // For std::vector
#include <queue>            // For priority_queue
#include <utility>          // For std::pair
#include <chrono>           // For timing
#include <cmath>            // For abs()
#include <unordered_set>    // For fast hash set
#include <cstdlib>          // For std::abs and other utilities

using namespace std;

/****************************************
*              BOARD CLASS              *
*****************************************/
// Represents the puzzle board
class Board {
private:
    vector<vector<int>> state;  // 2D vector holding the puzzle configuration
                                // A vector of vectors, basically a matrix/table

public:
    // Constructor to initialize the board
    Board(const vector<vector<int>>& initialState) : state(initialState) {}

    // Getter to retrieve the board state
    // Can you not just write Board.state??
    vector<vector<int>> getState() const {
        return state;
    }

    // Check if current state matches the goal state
    bool isGoalState(const Board& goalBoard) const {
        return state == goalBoard.getState();
    }

    // Print the board to console
    void printBoard() const {
        for (const auto& row : state) {     // For each row vector
            for (int cell : row) {          // For each cell in the vector
                cout << cell << " ";        // Print out the cell value
            }
            cout << endl;                   // New line (row) 
        }
    }

    // Find the (row, col) position of a given tile value
    // Make it a pair for easier access and use later
    pair<int, int> findTilePosition(int tileValue) const {
       for (size_t i = 0; i < state.size(); ++i) {
            for (size_t j = 0; j < state[i].size(); ++j) {
                if (state[i][j] == tileValue) {
                    return make_pair(i, j);
                }
            }
        }
        return make_pair(-1, -1);  // If not found (shouldn't happen in valid puzzles)
    }

    // Move the empty tile (0) in the given direction (L, R, U, D)
    bool move(char direction) {
        pair<int, int> emptyPos = findTilePosition(0);  // Locate empty tile
        bool success = false;                           // bool for returning
        int emptyRow = emptyPos.first;                  // Position for empty tile (used to swap)
        int emptyCol = emptyPos.second;                 // Position for empty tile (used to swap)

        // Move Left: valid if not on first column
        if (direction == 'L' && emptyCol > 0) {
            swap(state[emptyRow][emptyCol], state[emptyRow][emptyCol - 1]);
            success = true;
        }
        // Move Right: valid if not on last column
        else if (direction == 'R' && emptyCol < state[0].size() - 1) {
            swap(state[emptyRow][emptyCol], state[emptyRow][emptyCol + 1]);
            success = true;
        }
        // Move Up: valid if not on first row
        else if (direction == 'U' && emptyRow > 0) {
            swap(state[emptyRow][emptyCol], state[emptyRow - 1][emptyCol]);
            success = true;
        }
        // Move Down: valid if not on last row
        else if (direction == 'D' && emptyRow < state.size() - 1) {         
            swap(state[emptyRow][emptyCol], state[emptyRow + 1][emptyCol]);
            success = true;
        }
        return success;
    }

    // Equality comparison operator for Board (used in sets and comparisons)
    bool operator==(const Board& other) const {
        return state == other.getState();
    }
};

/****************************************
*        GOD KNOWS HOW THIS WORK        *
*****************************************/
// Custom hash function for the Board class to use in unordered_set
struct BoardHash {
    size_t operator()(const Board& board) const {
        size_t hash = 0;
        for (const auto& row : board.getState()) {
            for (int value : row) {
                // Combine hash values for each tile using a common technique
                hash ^= std::hash<int>()(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
        }
        return hash;
    }
};


/****************************************
*               NODE CLASS              *
*****************************************/
// Node class representing a state in the search tree
class Node {
private:
    Board board;    // Each node has a Board with a state 
    int heuristic;  // Heauristic value (depends on chosen function)
    int pathCost;   // Cost from start node to this.node
    Node* pre;      // Pointer to parent node (used in backtracking)
    char direction; // Direction to move from parent node to this node

public:
    // Constructor
    Node(const Board& boardState, int h, int cost, Node* parent, char dir)
        : board(move(boardState)), heuristic(h), pathCost(cost), pre(parent), direction(dir) {}

    /****************************************
    *               FUNCTIONS               *
    *****************************************/
    Board getBoard() const { return board; }
    int getHeuristic() const { return heuristic; }
    int getPathCost() const { return pathCost; }
    Node* getParent() const { return pre; }
    char getDirection() const { return direction; }


    // Comparison based on total cost for priority queue (pathCost + heuristic)
    bool operator<(const Node& other) const {
        return heuristic + pathCost > other.heuristic + other.pathCost;
    }
};

// Comparison functor for the priority queue
struct CompareNode {
    bool operator()(Node* lhs, Node* rhs) const {
        return *lhs < *rhs;
    }
};

/****************************************
*             PUZZLE CLASS              *
*****************************************/
// Main puzzle solver class
class Puzzle {
private:
    priority_queue<Node*, vector<Node*>, CompareNode> frontierList;  // A* frontier
    unordered_set<Board, BoardHash> uniqueSet;                       // Visited states
    Board initialState;                                              // Initial state
    Board goalState;                                                 // Goal state
    int heuristicOption;                                             // Heuristic choice by user

    // Heuristic #1: Count of misplaced tiles
    int h1(const Board& currentBoard, const Board& goalBoard) const {
        int misplacedTiles = 0;
        auto currentState = currentBoard.getState();
        auto goalState = goalBoard.getState();

        for (size_t i = 0; i < currentState.size(); ++i) {
            for (size_t j = 0; j < currentState[i].size(); ++j) {
                if (currentState[i][j] != goalState[i][j] && currentState[i][j] != 0) {
                    ++misplacedTiles;
                }
            }
        }
        return misplacedTiles;
    }

    // Heuristic #2: Total Manhattan distance
    int h2(const Board& currentBoard, const Board& goalBoard) const {
        int manhattanDistance = 0;
        auto currentState = currentBoard.getState();

        for (size_t i = 0; i < currentState.size(); ++i) {
            for (size_t j = 0; j < currentState[i].size(); ++j) {
                int tileValue = currentState[i][j];
                if (tileValue != 0) {
                    auto goalPosition = goalBoard.findTilePosition(tileValue);
                    manhattanDistance += abs(int(i) - goalPosition.first) + abs(int(j) - goalPosition.second);
                }
            }
        }
        return manhattanDistance;
    }

public:
    // Constructor initializes the first node and heuristic
    Puzzle(const Board& initial, const Board& goal, int option)
        : initialState(initial), goalState(goal), heuristicOption(option) {
        int initialHeuristic = calculateHeuristic(initialState);
        Node* initialNode = new Node(initialState, initialHeuristic, 0, nullptr, '0');
        addToFrontier(initialNode);
    }

    // Destructor cleans up dynamic memory for nodes
    ~Puzzle() {
        while (!frontierList.empty()) {
            delete frontierList.top();
            frontierList.pop();
        }
    }

    // Add node to the priority queue
    void addToFrontier(Node* node) {
        frontierList.push(node);
    }

    // Check if frontier is empty (no solution)
    bool isFrontierEmpty() const {
        return frontierList.empty();
    }

    // Generate successor states by trying all directions
    void expand(Node* currentNode) {
        Board currentBoard = currentNode->getBoard();
        vector<char> directions = { 'L', 'R', 'U', 'D' };

        for (char dir : directions) {
            Board successorBoard = currentBoard;           // Copy board
            if (successorBoard.move(dir)) {                // Try move
                int successorHeuristic = calculateHeuristic(successorBoard);
                int successorPathCost = currentNode->getPathCost() + 1;
                Node* successorNode = new Node(successorBoard, successorHeuristic, successorPathCost, currentNode, dir);
                addToFrontier(successorNode);              // Add new node
            }
        }
    }

    // Compute heuristic for a board
    int calculateHeuristic(const Board& board) const {
        return heuristicOption == 1 ? h1(board, goalState) : h2(board, goalState);
    }

    // Main A* solver
    void solve() {
        while (!isFrontierEmpty()) {
            Node* currentNode = frontierList.top();   // Get node with lowest cost
            frontierList.pop();                       // Remove from frontier

            // Insert returns true if board was not in set before
            if (uniqueSet.insert(currentNode->getBoard()).second) {
                if (currentNode->getBoard().isGoalState(goalState)) {
                    // Reconstruct path
                    Node* currentPoint = currentNode;
                    while (currentPoint != nullptr) {
                        cout << "------------" << endl;
                        cout << "Node " << currentPoint->getPathCost() << endl;
                        cout << "shape: " << endl;
                        currentPoint->getBoard().printBoard();
                        cout << "Previous direction from parent: " << currentPoint->getDirection() << endl;
                        currentPoint = currentPoint->getParent();  // Move up the tree
                    }
                    cout << "------------" << endl;
                    cout << "Path Cost: " << currentNode->getPathCost() << endl;
                    return;
                }
                // Expand if not goal
                expand(currentNode);
            }
        }

        // If loop finishes without finding goal
        cout << "No Solution Found!" << endl;
    }
};

/****************************************
*                  MAIN                 *
*****************************************/
int main() {
    int option;

    // Ask user for which heuristic to use
    cout << "Choose heuristic:\n";
    cout << "1 - Misplaced Tiles\n";
    cout << "2 - Manhattan Distance\n";
    cout << "Enter 1 or 2: ";
    cin >> option;

    // Ensure valid input
    while (option != 1 && option != 2) {
        cout << "Invalid choice! Please enter 1 or 2: ";
        cin >> option;
    }

    cout << "Using heuristic: " << option << endl;

    // Define initial and goal board states
    vector<vector<int>> initialBoardState = {
        {8, 6, 7},
        {2, 5, 4},
        {3, 0, 1}
    };

    vector<vector<int>> goalBoardState = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 0}
    };

    // Create Board and Puzzle objects
    Board initialBoard(initialBoardState);
    Board goalBoard(goalBoardState);
    Puzzle puzzle(initialBoard, goalBoard, option);

    // Measure and output how long solving takes
    auto startTime = chrono::high_resolution_clock::now();
    puzzle.solve();
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Time taken to solve the puzzle: " << duration.count() << " milliseconds" << endl;

    return 0;
}
