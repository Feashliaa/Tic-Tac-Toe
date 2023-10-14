#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <algorithm>
#include <iostream>

// Defining some constants
#define IDC_BUTTON_START 101 // starting ID for buttons
#define SIZE 3               // size of the board

HWND buttons[SIZE][SIZE];    // button handles
int board[SIZE][SIZE] = {0}; // game state
bool turn = true;            // true for 'X', false for 'O'

int boardWidth = SIZE * 100;
int boardHeight = SIZE * 100;

HBRUSH hBrushBlue = CreateSolidBrush(RGB(0, 0, 255));  // Blue for 'X'
HBRUSH hBrushRed = CreateSolidBrush(RGB(255, 0, 0));   // Red for 'O'
HBRUSH hBrushDefault = CreateSolidBrush(RGB(0, 0, 0)); // Black for empty cells

void findBestMove(int board[SIZE][SIZE], int *bestI, int *bestJ);
int minimax(int board[SIZE][SIZE], bool isMax);
void HandleButtonClick(WPARAM wParam, HWND hwnd);
void UpdateBoard(int i, int j);
int CheckWinner();
void HandleGameEnd(int winner, HWND hwnd);
void CheckTie(HWND hwnd);
void ResetGame();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // Window procedure
{
    switch (uMsg) // Handle window messages
    {
    case WM_CLOSE: // Handle window close event
        std::cout << "exiting from window close" << std::endl;
        PostQuitMessage(0);
        return 0;
    case WM_DRAWITEM: // Handle button draw event
    {
        LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam; // Get draw item struct from lParam
        if (lpDIS->CtlType == ODT_BUTTON)                  // If the draw item is a button
        {
            char text[10];
            GetWindowText(lpDIS->hwndItem, text, sizeof(text));

            // Set the color based on the text
            if (strcmp(text, "X") == 0)
            {
                FillRect(lpDIS->hDC, &lpDIS->rcItem, hBrushBlue);
            }
            else if (strcmp(text, "O") == 0)
            {
                FillRect(lpDIS->hDC, &lpDIS->rcItem, hBrushRed);
            }
            else
            {
                FillRect(lpDIS->hDC, &lpDIS->rcItem, hBrushDefault);
            }

            // Draw the text
            SetBkMode(lpDIS->hDC, TRANSPARENT);
            DrawText(lpDIS->hDC, text, -1, &lpDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Draw the border for the button
            HBRUSH hBrushBorder = CreateSolidBrush(RGB(128, 128, 128));
            FrameRect(lpDIS->hDC, &lpDIS->rcItem, hBrushBorder);
            DeleteObject(hBrushBorder); // Clean up the brush after using it
        }
        return TRUE;
    }
    break;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);  // Width of the resized window
        int height = HIWORD(lParam); // Height of the resized window

        // Calculate button width and height based on the new window size
        int buttonWidth = width / SIZE;
        int buttonHeight = height / SIZE;

        // Reposition and resize buttons
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                SetWindowPos(buttons[i][j], NULL,
                             j * buttonWidth, i * buttonHeight,
                             buttonWidth, buttonHeight,
                             SWP_NOZORDER);
            }
        }
    }
    break;
    case WM_SHOWWINDOW:
        if (wParam == TRUE) // Window is being shown
        {
            RECT rect;
            GetClientRect(hwnd, &rect); // Get the client area of the window

            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            // Calculate button width and height based on the window size
            int buttonWidth = width / SIZE;
            int buttonHeight = height / SIZE;

            // Create and position the buttons
            for (int i = 0; i < SIZE; i++)
            {
                for (int j = 0; j < SIZE; j++)
                {
                    // Create the button, set its ID, and position it in the window
                    buttons[i][j] = CreateWindow(
                        "BUTTON", "",
                        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                        j * buttonWidth, i * buttonHeight,
                        buttonWidth, buttonHeight,
                        hwnd, (HMENU)(INT_PTR)(IDC_BUTTON_START + i * SIZE + j),
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
                }
            }
        }
        break;
    case WM_DESTROY: // Handle window destroy event
        DeleteObject(hBrushRed);
        DeleteObject(hBrushBlue);
        DeleteObject(hBrushDefault);
        return 0;
    case WM_COMMAND:
        HandleButtonClick(wParam, hwnd);
        return 0;

    default: // Handle any messages the switch statement didn't
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "Tic-Tac-Toe"; // Window class name

    WNDCLASS wc = {}; // Window class

    wc.lpfnWndProc = WindowProc;   // Window procedure
    wc.hInstance = hInstance;      // Instance handle
    wc.lpszClassName = CLASS_NAME; // Set the class name

    RegisterClass(&wc); // Register the window class

    RECT winRect = {0, 0, boardWidth, boardHeight};
    AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Get the screen resolution
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Calculate the window position
    int windowWidth = winRect.right - winRect.left;
    int windowHeight = winRect.bottom - winRect.top;

    // Center the window
    int xPos = (screenWidth - windowWidth) / 2;
    int yPos = (screenHeight - windowHeight) / 2;

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                   // Optional window styles.
        CLASS_NAME,          // Window class
        "Tic-Tac-Toe",       // Window text
        WS_OVERLAPPEDWINDOW, // Window style
        xPos, yPos,          // Window position (centered)
        windowWidth, windowHeight,
        NULL,      // Parent window
        NULL,      // Menu
        hInstance, // Instance handle
        NULL       // Additional application data
    );

    if (hwnd == NULL) // If window creation fails, return 0
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow); // Show the window

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) // Message loop
    {
        TranslateMessage(&msg); // Translate virtual-key messages into character messages
        DispatchMessage(&msg);  // Dispatches a message to a window procedure
    }

    return 0;
}

void HandleButtonClick(WPARAM wParam, HWND hwnd)
{
    if (LOWORD(wParam) >= IDC_BUTTON_START && LOWORD(wParam) < IDC_BUTTON_START + SIZE * SIZE)
    {
        int id = LOWORD(wParam) - IDC_BUTTON_START; // Get the button ID
        int i = id / SIZE;
        int j = id % SIZE;

        if (board[i][j] == 0)
        {
            UpdateBoard(i, j);
            if (!turn) // If it's the computer's turn
            {
                int bestI, bestJ; // Best move coordinates

                int winner = CheckWinner();
                if (winner != 0)
                {
                    HandleGameEnd(winner, hwnd);
                    return;
                }

                findBestMove(board, &bestI, &bestJ); // Find the best move for the computer, sending the address of bestI and bestJ
                UpdateBoard(bestI, bestJ);
            }

            int winner = CheckWinner();

            if (winner != 0)
            {
                HandleGameEnd(winner, hwnd);
            }
            else
            {
                CheckTie(hwnd);
            }
        }
    }
}

void UpdateBoard(int i, int j)
{
    board[i][j] = turn ? 1 : 2; // 1 for 'X', 2 for 'O'
    SetWindowText(buttons[i][j], turn ? "X" : "O");
    turn = !turn; // Switch turns
}

int CheckWinner()
{
    // Check rows
    for (int i = 0; i < SIZE; i++)
    {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][1] == board[i][2])
        {
            return board[i][0];
        }
    }

    // Check columns
    for (int j = 0; j < SIZE; j++)
    {
        if (board[0][j] != 0 && board[0][j] == board[1][j] && board[1][j] == board[2][j])
        {
            return board[0][j];
        }
    }

    // Check diagonals
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2])
    {
        return board[0][0];
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0])
    {
        return board[0][2];
    }

    // Check if the board is full
    bool isBoardFull = true;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] == 0)
            {
                isBoardFull = false;
                break; // exit the inner loop as soon as an empty cell is found
            }
        }
        if (!isBoardFull)
            break; // exit the outer loop as soon as an empty cell is found
    }

    if (isBoardFull)
        return 3;

    return 0;
}

void HandleGameEnd(int winner, HWND hwnd)
{
    char text[20];

    // check winner, 1 = Player 1, 2 = Computer
    // if winner = 3, then it's a tie

    if (winner == 3)
        sprintf(text, "Tie!\nPlay Again?");
    else
        sprintf(text, "Player %d wins!\nPlay Again?", winner);

    int result = MessageBox(hwnd, text, "Game Over", MB_YESNO);

    if (result == IDYES)
    {
        ResetGame();
    }
    else
    {
        PostQuitMessage(0);
    }
}

void CheckTie(HWND hwnd)
{
    bool tie = true;
    bool breakOuterLoop = false; // Flag to break out of the outer loop

    for (int i = 0; i < SIZE && !breakOuterLoop; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] == 0)
            {
                tie = false;
                breakOuterLoop = true; // Set the flag when an empty cell is found
                break;
            }
        }
    }

    if (tie)
    {
        int result = MessageBox(hwnd, "Tie!\nPlay Again?", "Game Over", MB_YESNO);

        if (result == IDYES)
        {
            ResetGame();
        }
        else
        {
            PostQuitMessage(0);
        }
    }
}

void ResetGame()
{
    turn = true;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            board[i][j] = 0;
            SetWindowText(buttons[i][j], "");
        }
    }
}

int minimax(int board[SIZE][SIZE], bool isMax)
{
    int winner = CheckWinner(); // Check if there is a winner

    if (winner == 1)
        return -10; // Player wins
    if (winner == 2)
        return 10; // AI wins

    // Check for tie
    bool tie = true;
    for (int i = 0; i < SIZE && tie; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == 0)
                tie = false;

    if (tie)
        return 0; // Tie game

    if (isMax)
    {
        int maxEval = -9999;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (board[i][j] == 0)
                {
                    board[i][j] = 2; // AI's move
                    int eval = minimax(board, false);
                    board[i][j] = 0; // Undo move
                    maxEval = std::max(maxEval, eval);
                }
        return maxEval;
    }
    else
    {
        int minEval = 9999;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (board[i][j] == 0)
                {
                    board[i][j] = 1; // Player's move
                    int eval = minimax(board, true);
                    board[i][j] = 0; // Undo move
                    minEval = std::min(minEval, eval);
                }
        return minEval;
    }
}

void findBestMove(int board[SIZE][SIZE], int *bestI, int *bestJ)
{

    int bestScore = -9999;

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] == 0)
            {                                      // If cell is empty
                board[i][j] = 2;                   // Try the computer's move
                int score = minimax(board, false); // Start minimax for the player's turn
                board[i][j] = 0;                   // Reset the move

                if (score > bestScore)
                {
                    bestScore = score;
                    *bestI = i;
                    *bestJ = j;
                }
            }
        }
    }
}
