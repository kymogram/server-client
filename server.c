#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "position.h"
#include "server.h"

#define MYPORT 3490    // the port users will be connecting to

#define BACKLOG 10     // how many pending comnnections queue will hold

void startGame(int new_fd)
{
    char board[3][3] = {
                        {'1', '2', '3'},
                        {'4', '5', '6'},
                        {'7', '8', '9'}
                       };
    int counter = 0; // game counter, if this goes to 8 and no winner, draw
    int notEnd = 1;
    const char askChoice[] = "Où voulez vous jouer votre coup ?\n";
    int pos;
    struct position position;
    
    while (notEnd)
    {
        // IA start first
        IA(board, new_fd);
        // check if the IA won (otherwise client loose)
        if (isFinish(board))
        {
            loss(new_fd);
            notEnd = 0;
        }
        else
        {
            do
            {
                // ask client to play and receive its response
                if (send(new_fd, askChoice, sizeof(askChoice), 0) == -1)
                    perror("send");
                if (recv(new_fd, &pos, sizeof(int), 0) == -1)
                    perror("recv");
                // convert the position receive for the board
                position = findPos(pos);
            // has to be in the board and not already used
            }while ((pos>9 || pos<0) || checkIfUsed(board, position));
            board[position.row][position.column] = 'X';
            if (isFinish(board))
            {
                win(new_fd);
                notEnd = 0;
            }
            else
            {
                // if counter is 8, every square is taken -> draw
                if (counter == 8)
                {
                    draw(new_fd);
                    notEnd = 0;
                }
            }
        }
        ++counter;
    }
}

void draw(int new_fd)
{
    const char drawGame[] = "Match nul!\n";
    if (send(new_fd, drawGame, sizeof(drawGame), 0) == -1)
        perror("send");
}

void win(int new_fd)
{
    const char winGame[] = "Vous avez gagné!\n";
    if (send(new_fd, winGame, sizeof(winGame), 0) == -1)
        perror("send");
}

void loss(int new_fd)
{
    const char lossGame[] = "Vous avez perdu!\n";
    if (send(new_fd, lossGame, sizeof(lossGame), 0) == -1)
        perror("send");
}

void IA(char board[3][3], int new_fd)
{
    struct position position;
    int pos;
    do
    {
        // take random number between 1 and 9
        pos = rand()%9 + 1;
        // convert it for the board
        position = findPos(pos);
    }while (checkIfUsed(board, position));
    board[position.row][position.column] = 'O';
    if (send(new_fd, &position, sizeof(position), 0) == -1)
        perror("send");
}

void askPlay(int new_fd)
{
    const char wannaPlay[] = "Si voulez vous jouer une partie, envoyez 1, sinon 2 pour quitter\n";
    char answer;
    if (send(new_fd, wannaPlay, sizeof(wannaPlay), 0) == -1)
        perror("send");
    if (recv(new_fd, &answer, 1, 0) == -1)
        perror("recv");
    if (answer == '1')
        startGame(new_fd);
}

void sigchld_handler(int s)
{
    while(wait(NULL) > 0);
    (void)s;
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    while(1)
    {  // main accept() loop
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        // srand is used for the IA when it makes random number. It's a better way to get random number
        srand(time(NULL));
        // new player
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            askPlay(new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }
    return 0;
}