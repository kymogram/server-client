#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "position.h"
#include "client.h"

#define PORT 3490 // the port client will be connecting to 

void displayBoard(char board[3][3])
{
    printf("\n\n");
    printf(" %c | %c | %c\n", board[0][0], board[0][1], board[0][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c\n", board[1][0], board[1][1], board[1][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c\n", board[2][0], board[2][1], board[2][2]);
}

void answerPlay(int sockfd)
{
    char question[100];
    char answer;
    if ((recv(sockfd, question, 100, 0)) == -1)
    {
        perror("recv");
    }
    printf("%s", question);
    scanf("%c", &answer);
    if ((send(sockfd, &answer, 1, 0)) == -1)
    {
        perror("send");
    }
    if (answer == '1')
        play(sockfd);
}

void play(int sockfd)
{
    char board[3][3] = {
                        {'1', '2', '3'},
                        {'4', '5', '6'},
                        {'7', '8', '9'}
                       };
    struct position position;
    int notEnd = 1;
    char question[100];
    char winSentence[100];
    char lossSentence[100];
    char drawSentence[100];
    int answer;
    int counter = 0;
    struct position IAPlay;

    displayBoard(board);
    while (notEnd)
    {
        if (recv(sockfd, &IAPlay, sizeof(IAPlay), 0) == -1)
                perror("recv");
        board[IAPlay.row][IAPlay.column] = 'O';
        displayBoard(board);
        if (isFinish(board))
        {
            notEnd = 0;
            if (recv(sockfd, lossSentence, 100, 0) == -1)
                perror("recv");
            printf("%s", lossSentence);
            answerPlay(sockfd);
        }
        else
        {
            do
            {
                if (recv(sockfd, question, 100, 0) == -1)
                    perror("recv");
                printf("%s", question);
                scanf("%d", &answer);
                position = findPos(answer);
            }while ((answer>9 || answer<0) && checkIfUsed(board, position));
            board[position.row][position.column] = 'X';
            displayBoard(board);
            if (send(sockfd, &answer, sizeof(int), 0) == -1)
                perror("send");
            if (isFinish(board))
            {
                notEnd = 0;
                if (recv(sockfd, winSentence, 100, 0) == -1)
                    perror("recv");
                printf("%s", winSentence);
                answerPlay(sockfd);
            }
            else
            {
                if (counter == 8)
                {
                    if (recv(sockfd, drawSentence, 100, 0) == -1)
                        perror("recv");
                    printf("%s", drawSentence);
                    answerPlay(sockfd);
                }
                ++counter;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information 

    if (argc != 2)
    {
        printf("usage: ./client <HOST>");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info 
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;    // host byte order 
    their_addr.sin_port = htons(PORT);  // short, network byte order 
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(1);
    }
    answerPlay(sockfd);
    close(sockfd);

    return 0;
}