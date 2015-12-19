#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "position.h"

struct position findPos(int pos)
{
    struct position posPlay = {0, 0};
    posPlay.row = --pos/3;
    posPlay.column = pos%3;
    return posPlay;
}

int isFinish(char board[3][3])
{
    int line;
    // Check for a winning line - diagonals first
    if((board[0][0] == board[1][1] && board[0][0] == board[2][2]) ||
         (board[0][2] == board[1][1] && board[0][2] == board[2][0]))
        return 1;
    else
    // Check rows and columns for a winning line
        for(line=0;line<=2;++line)
            if((board[line][0] == board[line][1] && board[line][0] == board[line][2])||
              (board[0][line] == board[1][line] && board[0][line] == board[2][line]))
               return 1;
    return 0;
}

void draw(int new_fd)
{
    const char drawGame[] = "Match nul!/n";
    if (send(new_fd, drawGame, sizeof(drawGame), 0) == -1)
        perror("send");
    askPlay(new_fd);
}

void win(int new_fd)
{
    const char winGame[] = "Vous avez gagné!/n";
    if (send(new_fd, winGame, sizeof(winGame), 0) == -1)
        perror("send");
    askPlay(new_fd);
}

void loss(int new_fd)
{
    const char lossGame[] = "Vous avez perdu!/n";
    if (send(new_fd, lossGame, sizeof(lossGame), 0) == -1)
        perror("send");
    askPlay(new_fd);
}

