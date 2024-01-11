#include	"unp.h"
#include  <string.h>
#include  <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>


/*
void sig_chld(int signo)
{
    pid_t   pid;
    int     stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0){
    	kill(pid, SIGKILL);
    }
    return;
}
*/
void xchg_data(int client1, int client2, char *id1, char *id2) {
    fd_set rset;
    int maxfdp1, n, nready;
    char recvline1[100], recvline2[100];

    FD_ZERO(&rset);
    maxfdp1 = max(client1, client2) + 1;

    for (;;) {
        FD_SET(client1, &rset);
        FD_SET(client2, &rset);

        nready = Select(maxfdp1, &rset, NULL, NULL, NULL);
        if(nready <= 0) continue;
		

        if (FD_ISSET(client1, &rset)) {
            if ((n = Read(client1, recvline1, MAXLINE)) == 0) {
                char s1[MAXLINE];
		        snprintf(s1, sizeof(s1), "(%s left the room. Press Ctrl+D to leave.)\n", id1);
		        Writen(client2, s1, strlen(s1));
		        Shutdown(client2, SHUT_WR);  
		        
		        if (Read(client2, recvline2, MAXLINE) == 0) {
		        	char s5[MAXLINE];
				    snprintf(s5, sizeof(s5), "(%s left the room.)\n", id2);
				    Writen(client1, s5, strlen(s5));
				    Shutdown(client1, SHUT_WR); 
		        }
		        
            }
            recvline1[n] = '\0';
            char s2[MAXLINE];
            snprintf(s2, sizeof(s2), "%s", recvline1);
            Writen(client2, s2, strlen(s2));
            if(--nready <= 0) continue;
        }

        else if (FD_ISSET(client2, &rset)) {
            if ((n = Read(client2, recvline2, MAXLINE)) == 0) {
                char s3[MAXLINE];
		        snprintf(s3, sizeof(s3), "(%s left the room. Press Ctrl+D to leave.)\n", id2);
		        Writen(client1, s3, strlen(s3));
		        Shutdown(client1, SHUT_WR);  
		        
		        if (Read(client1, recvline1, MAXLINE) == 0) {
		        	char s6[MAXLINE];
				    snprintf(s6, sizeof(s6), "(%s left the room.)\n", id1);
				    Writen(client2, s6, strlen(s6));
				    Shutdown(client2, SHUT_WR);
		        }
            }
            recvline2[n] = '\0';
            char s4[MAXLINE];
            snprintf(s4, sizeof(s4), "%s", recvline2);
            Writen(client1, s4, strlen(s4));
            if(--nready <= 0) continue;
        }
    }
}

int main(int argc, char **argv)
{
    int listenfd, maxfd, maxi, i, nready, connfd, clients[FD_SETSIZE], port[FD_SETSIZE], cnt;
    fd_set allset, rset;
 
    pid_t       childpid;
    socklen_t   clilen;
    struct sockaddr_in  cliaddr, servaddr;
    char      sendline[MAXLINE];
    char id[FD_SETSIZE][MAXLINE];
    
	cnt = -1;
	
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT+3);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    //Signal(SIGCHLD, sig_chld);
    

    // TCP Server
    
    maxfd = listenfd;
    maxi = -1;
	for (i = 0; i < FD_SETSIZE; i++) {
	    clients[i] = -1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
    
    
    for(;;){
        rset = allset;
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
		if(nready <= 0) continue;
			
        
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
            for (i = 0; i < FD_SETSIZE; i++){
				if (clients[i] < 0) { 
					clients[i] = connfd;
					cnt++;
					break;
				}
			}
			if (i == FD_SETSIZE) err_quit("too many clients");
			
            FD_SET(connfd , &allset);
            if(connfd > maxfd) maxfd = connfd;
            if(i > maxi) maxi = i;
            if(--nready < 0) continue;
            
            port[maxi] = ntohs(cliaddr.sin_port);
            // Receive id from client
            Read(clients[maxi], id[maxi], MAXLINE);
            //printf("maxi: %d\n", maxi);
            if(cnt%2 == 0)
            {
            	char s[MAXLINE] = "You are the 1st user, wait for the second one!\n";
				Writen(clients[cnt], s, strlen(s));	
				//printf("cnt: %d\n", cnt);
            }
            else
            {
            	
				sprintf(sendline, "The second user is %s from %d\nThe white stone is yours, black is %s's!\n", id[cnt], port[cnt], id[cnt]);
				Writen(clients[cnt-1], sendline, strlen(sendline));
				
				char s2[MAXLINE];
				sprintf(s2, "You are the 2nd user. The white stone is yours, black is %s from %d.\n", id[cnt-1], port[cnt-1]);
				Writen(clients[cnt], s2, strlen(s2));
				
				char s3[MAXLINE] = "You can start enter the coordinate of your first stone!\n";
				Writen(clients[cnt], s3, strlen(s3));
				
				if ( (childpid = Fork()) == 0) {	/* child process */
					//Close(listenfd);
					xchg_data(clients[cnt-1], clients[cnt], id[cnt-1], id[cnt]);	/* process */
					exit(0);
				}else if (childpid > 0){
					pid_t   pid;
					int     stat;

					while ((pid = waitpid(-1, &stat, WNOHANG)) > 0);
					return 0;
				}
				Close(clients[cnt-1]); /* parent closes connected socket */
				Close(clients[cnt]);
				FD_CLR(clients[cnt-1], &allset);
				FD_CLR(clients[cnt], &allset);
				clients[cnt-1] = -1;
				clients[cnt] = -1;
				
            }
            
           	
        }
        
     }

	
     return 0;
}

