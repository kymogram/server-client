# server-client

GDB dit:

avec bt:

#0 0x08048a07 in play()
#1 0x08048848 in askPlay()
#2 0x08048bac in main()

avec -g:

x0x08048a07 in plat (sockfd=3) at client.c:83
83                            board[IAPlay.row][AIPlay.column] = 'X';

avec print(IAPlay) sur GDB :

$1 = {row = 1937076054, column = 1702256928}

OR normalement avec ma structure findPos, j'utilise des % et / pour éviter cela...
