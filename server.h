void startGame(int new_fd);
void sigchld_handler(int);
void IA(char board[3][3], int new_fd);
void askPlay(int new_fd);
void loss(int new_fd);
void win(int new_fd);
void draw(int new_fd);