struct position
{
    int row, column;
};

void draw(int new_fd);
struct position findPos(int pos);
void askPlay(int new_fd);
void win(int new_fd);
void loss(int new_fd);
int isFinish(char board[3][3]);
