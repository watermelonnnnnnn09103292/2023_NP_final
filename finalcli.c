//#include<iostream>
#include "unp.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>


char id[MAXLINE];

void show_board();
bool winner_check(int i,int j,int whosturn);
bool stupid(int i, int j);
bool stupid2(int i, int j);

int board[22][22];
bool stop_flag = 0;


void xchg_data(FILE *fp, int sockfd)
{
    int       maxfdp1, stdineof, peer_exit, n;
    fd_set    rset;
    char      sendline[MAXLINE], recvline[MAXLINE];

    Writen(sockfd, id, strlen(id));
    printf("sent: %s\n", id);
	readline(sockfd, recvline, MAXLINE);
	printf("recv: %s", recvline);
	readline(sockfd, recvline, MAXLINE);
	printf("recv: %s", recvline);	
    stdineof = 0;
	peer_exit = 0;
	
	
    for ( ; ; ) {
    	
		FD_ZERO(&rset);
		maxfdp1 = 0;
        if (stdineof == 0) {
            FD_SET(fileno(fp), &rset);
			maxfdp1 = fileno(fp);
		};	
		if (peer_exit == 0) {
			FD_SET(sockfd, &rset);
			if (sockfd > maxfdp1)
				maxfdp1 = sockfd;
		};	
        maxfdp1++;
        Select(maxfdp1, &rset, NULL, NULL, NULL);
        
        //recv position from enemy
		if (FD_ISSET(sockfd, &rset)) {  /* socket is readable */
			int I=0, J=0;
			n = read(sockfd, recvline, MAXLINE);
			if (n == 0) {
 		   		if (stdineof == 1)
                    return;         /* normal termination */
		   		else {
					printf("(End of input from the peer!)");
					peer_exit = 1;
				};
            }
			else if(n > 0) {		/* successfuly get string from stdin */
					
					if(recvline[0] == '(') {	/* if message is not position, set the flag to 1 so that it won't draw board */
						stop_flag = 1;
						recvline[n] = '\0';
						printf("%s", recvline);
					}	
				
					for(int n=0; n<(int)strlen(recvline) && stop_flag == 0; n++){//處理字串keyin
		               //輸入為a~t
		                if(recvline[n]>=97 && recvline[n]<=116)
		                    J = recvline[n]-96;
		                //輸入為A~T
		                else if(recvline[n]>=65 && recvline[n]<=84)
		                    J = recvline[n]-64;
		                //輸入為數字0~9
		                else if(recvline[n]>=48 && recvline[n]<=57)
		                    I = I*10+recvline[n]-48;
		                else if(recvline[n] == '\n')
		                    break;
		                
		            }
		            if(stupid2(I, J)) continue;
                	else board[I][J] = 2;
		            //draw it on the board, 1:owner, 2:others
		            if(stop_flag == 0){
		            	system("clear");
		            	recvline[n] = '\0';
		            	printf("Black Stone: %s", recvline);
		            	show_board();
		            }
		            
		            if(winner_check(I,J,2)){
		            	printf("You Loss :)\n");
		            	stop_flag = 1;
		            	printf("(The game ends.)\n");
						//stdineof = 1;
						//Shutdown(sockfd, SHUT_WR); 
		            }
		            
                
			};
        }
		
		//send position to enemy
        if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
			int I=0, J=0;
            if (Fgets(sendline, MAXLINE, fp) == NULL) {
				if (peer_exit)
					return;
				else {
					printf("(leaving...)\n");
					stdineof = 1;
					Shutdown(sockfd, SHUT_WR);      /* send FIN */
				};
            }
			else {								/* successfuly get string from stdin */
				n = strlen(sendline);
				sendline[n] = '\n';
				Writen(sockfd, sendline, n+1);
				for(int n=0; n<(int)strlen(sendline) && stop_flag == 0; n++){//處理字串keyin
                   //輸入為a~t
                    if(sendline[n]>=97 && sendline[n]<=116)
                        J = sendline[n]-96;
                    //輸入為A~T
                    else if(sendline[n]>=65 && sendline[n]<=84)
                        J = sendline[n]-64;
                    //輸入為數字0~9
                    else if(sendline[n]>=48 && sendline[n]<=57)
                        I = I*10+sendline[n]-48;
                    else if(sendline[n] == ' ')
                        continue;
                    
                }
               
                //draw it on the board, 1:owner, 2:others
                if(stupid(I, J)) continue;
                else board[I][J] = 1;
                //printf("\033[2J\033[1;1H");
                system("clear");
                show_board();
                //check if someone wins
                if(winner_check(I,J,1)){
                	printf("You Win!\nPress ctrl+D to leave.\n");
        
                }
			};
        }
    }
};

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 3)
		err_quit("usage: tcpcli <IPaddress> <ID>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT+3);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	strcpy(id, argv[2]);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	
	//棋盤的初始化設定
    for(int i=0; i<=21; i++){
        for(int j=0; j<=21; j++){
            if(i==0 && j==0){
                 board[i][j] = 0;
            }
            else if(i==21 || j==21){
                 board[i][j] = 0;
            }
            //set the column index
            else if(i==0){
                 board[i][j] = j+30;
            }
            //set the row index
            else if(j==0){
                 board[i][j] = i+10;
            }
            //set the line of board
            else if(i==1 && j==1){
                 board[i][j] = -7;
            }
            else if(i==1 && j==20){
                 board[i][j] = -9;
            }
            else if(i==20 && j==1){
                 board[i][j] = -1;
            }
            else if(i==20 && j==20){
                 board[i][j] = -3;
            }
            else if(i==1){
                 board[i][j] = -8;
            }
            else if(j==1){
                 board[i][j] = -4;
            }
            else if(j==20){
                 board[i][j] = -6;
            }
            else if(i==20){
                 board[i][j] = -2;
            }
            else{
                 board[i][j] = -5;
            }
        }
     }

	xchg_data(stdin, sockfd);		/* do it all */

	exit(0);
}

void show_board(){
    for(int i=0 ;i<=21 ;i++){
        for(int j=0 ;j<=21 ;j++){
            switch(board[i][j]){
                 case -1:
                    printf("└");
                    break;
                case -2:
                    printf("┴");
                    break;
                case -3:
                    printf("┘");
                    break;
                case -4:
                    printf("├");
                    break;
                case -5:
                    printf("┼");
                    break;
                case -6:
                    printf("┤");
                    break;
                case -7:
                    printf("┌");
                    break;
                case -8:
                    printf("┬");
                    break;
                case -9:
                    printf("┐");
                    break;
                case 0:
                    printf("  ");
                    break;
                case 1:
                    printf("○");
                    break;
                case 2:
                    printf("●");
                    break;
                case 11:
                    printf("1 ");
                    break;
                case 12:
                    printf("2 ");
                    break;
                case 13:
                    printf("3 ");
                    break;
                case 14:
                    printf("4 ");
                    break;
                case 15:
                    printf("5 ");
                    break;
                case 16:
                    printf("6 ");
                    break;
                case 17:
                    printf("7 ");
                    break;
               case 18:
                    printf("8 ");
                    break;
                case 19:
                    printf("9 ");
                    break;
                case 20:
                    printf("10");
                    break;
                case 21:
                    printf("11");
                    break;
                case 22:
                    printf("12");
                    break;
                case 23:
                    printf("13");
                    break;
                case 24:
                    printf("14");
                    break;
                case 25:
                    printf("15");
                    break;
                case 26:
                    printf("16");
                    break;
                case 27:
                    printf("17");
                    break;
                case 28:
                    printf("18");
                    break;
                case 29:
                    printf("19");
                    break;
                case 30:
                    printf("20");
                    break;
                case 31:
                    printf("A");
                    break;
                case 32:
                    printf("B");
                    break;
                case 33:
                    printf("C");
                    break;
                case 34:
                    printf("D");
                    break;
                case 35:
                    printf("E");
                    break;
                case 36:
                    printf("F");
                    break;
                case 37:
                    printf("G");
                    break;
                case 38:
                    printf("H");
                    break;
                case 39:
                    printf("I");
                    break;
                case 40:
                    printf("J");
                    break;
                case 41:
                    printf("K");
                    break;
                case 42:
                    printf("L");
                    break;
                case 43:
                    printf("M");
                    break;
                case 44:
                    printf("N");
                    break;
                case 45:
                    printf("O");
                    break;
                case 46:
                    printf("P");
                    break;
                case 47:
                    printf("Q");
                    break;
                case 48:
                    printf("R");
                    break;
                case 49:
                    printf("S");
                    break;
                case 50:
                    printf("T");
                    break;
                }
        }
        printf("\n");
    }
}

bool winner_check(int i,int j,int whosturn){//如果win = 1代表有人勝利了
    int count,temp1,temp2;
    bool win=0;
	//判斷左右有否五顆連線
    count=1;
    temp1=i;
    while(board[--temp1][j] == whosturn)
        count++;
    temp1=i;
    while(board[++temp1][j] == whosturn)
        count++;
    if(count >= 5){
        win=1;
        goto winner_check_end;
    }
    //判斷上下有否五顆連線
    count=1;
    temp2=j;
    while(board[i][--temp2] == whosturn)
        count++;
    temp2 = j;
    while(board[i][++temp2] == whosturn)
        count++;
    if(count >= 5){
        win=1;
        goto winner_check_end;
    }
    //判斷左上到右下有否五顆連線
    count=1;
    temp1=i;
    temp2=j;
    while(board[--temp1][--temp2] == whosturn)
        count++;
    temp1=i;
    temp2=j;
    while(board[++temp1][++temp2] == whosturn)
        count++;
    if(count >= 5){
        win=1;
        goto winner_check_end;
    }
    //判斷左下到右上有否五顆連線
    count=1;
    temp1=i;
    temp2=j;
    while(board[++temp1][--temp2] == whosturn)
        count++;
    temp1=i;
    temp2=j;
    while(board[--temp1][++temp2] == whosturn)
        count++;
    if(count >= 5){
        win=1;
        goto winner_check_end;
    }
winner_check_end:
    return win;
}

bool stupid(int i, int j){

	if(board[i][j] == 1 || board[i][j] == 2){
		printf("Already taken! Choose again.\n");
		return 1;
	}
	else if( i<1 || i>20){
		printf("Beyond the board! Choose again.\n");
		return 1;
	}
	else if( j<1 || j>20){
		printf("Beyond the board! Choose again.\n");
		return 1;
	}
	return 0;
}

bool stupid2(int i, int j){

	if(board[i][j] == 1 || board[i][j] == 2){
		return 1;
	}
	else if( i<1 || i>20){
		return 1;
	}
	else if( j<1 || j>20){
		return 1;
	}
	return 0;
}

