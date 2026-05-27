# Tic-Tac-Toe

A Windows desktop Tic-Tac-Toe game written in C++ using the Win32 API. 
You play as X against a computer opponent that uses the Minimax algorithm with alpha-beta pruning.

## Features

- 3x3 game board rendered as clickable buttons
- Resizable window — buttons scale with it
- AI opponent using Minimax with alpha-beta pruning (unbeatable)
- Color-coded cells: blue for X, red for O
- Detects wins, losses, and ties with a play-again prompt

## Requirements

- Windows OS
- A C++ compiler with Win32 support (e.g. MSVC or MinGW)

## Building

**MSVC:**
```
cl /EHsc tictactoe.cpp /link user32.lib gdi32.lib
```

**MinGW:**
```
g++ tictactoe.cpp -o tictactoe.exe -lgdi32 -luser32 -mwindows
```

## How to Play

1. Run the executable.
2. You are X and always go first.
3. Click any empty cell to place your move.
4. The computer responds automatically.
5. First to get three in a row wins. If the board fills with no winner, it's a tie.
6. At game over, choose to play again or quit.

## Notes

The AI is optimal - at best you can force a draw. It will never lose.