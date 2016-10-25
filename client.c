#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>


#define SERVER_IP "127.0.0.1"

void *startListener(void *sock_fd);
int randPort();
char *sendMessage(int dst_port, char* msg, int sock_fd, int server);
int createSocket(int *port);
int *insertPort(int node, int* array, int size);
void stopListening();

int *client_ports, client_count = 0, listenCondition = 1, server_port;

int main(void)
{
  int c;
  int port, temp, i;
  int sockfd = createSocket(&port);
  pthread_t pth;
  int option;
  int dst_port;
  char temp_port[4], ch, test[255];
  char message[255];

  
  printf("Created socket running on %d.\n", port);

  printf("Please enter the server port:");
  scanf("%d",&server_port);

  sprintf(temp_port, "%d",  port);
  

  pthread_create(&pth,NULL, startListener,(void*)sockfd);
  sendMessage(server_port, temp_port, sockfd, 1);
  

  while(1)
  {

    option = -1;
   
    printf("1.\tInitiate Chat.\n");
    printf("2.\tTo close.");
    fscanf(stdin, "%d", &option);

    switch(option)
    {
      case 3:
        sendMessage(server_port, temp_port, sockfd, 1);
        break;
      case 1:

       printf("Enter the client port:");
          scanf("%d", &dst_port);
        
        printf("Enter a message:");
        // Check for newline and flush it!
        while ((c = getchar()) != '\n' && c != EOF);
        fgets(message, sizeof(message), stdin);

        sendMessage(dst_port, message, sockfd, 0);
        break;
      case 2:
        stopListening();
        shutdown(sockfd, SHUT_RDWR);
        pthread_join(pth, (void*) NULL);
        return 0;
      default:
        printf("Wrong input:%d\n", option);
        return -1;
    }
    
    
  }
  
  return 0;
}

char *sendMessage(int dst_port, char* msg, int sock_fd, int server)
{
  int sockfd = 0, n = 0, connfd = 0;
  char recvBuff[1024];
  struct sockaddr_in serv_addr;
  socklen_t s_len;
  char message[30];
  char port[4];
  char sendBuff[1025];

  
  if(server == 1)
  {
    strcpy(port, msg);
  }
  else
  {
    strcpy(message, msg);
  }         

  memset(sendBuff, '\0', sizeof(sendBuff));
  memset(recvBuff, '\0', sizeof(recvBuff));
  sockfd = sock_fd;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(dst_port);
  serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

  char dst[255];
  struct sockaddr *sa = (struct sockaddr*)&serv_addr;
  inet_ntop(AF_INET, &(((struct sockaddr_in*)sa)->sin_addr),dst,  sizeof(serv_addr));
  
  if(server == 1)
  {
    strcpy(sendBuff, port);
    
  }
  else
  {
    strcpy(sendBuff, message);
  }
  
  n = sendto(sockfd, sendBuff, strlen(sendBuff), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if( n < 0)  
  {
    printf("Send Error \n");
  }
  else  
  {
    printf("Sent:%s\n", sendBuff);
  }
  return NULL;
}

void *startListener(void *sockfd){
  int listenfd = (int) sockfd;
  char sendBuff[1025];

  while(listenCondition == 1)
  {
    int i ,n;
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    socklen_t c_len = sizeof(client);
    char recvBuff[1024];
    int connfd;
    
  	memset(recvBuff, '\0', sizeof(recvBuff));
memset(sendBuff, '\0', sizeof(sendBuff));
    n = recvfrom(listenfd,recvBuff, sizeof(recvBuff),0, (struct sockaddr *)&client, &c_len);
    if(listenCondition != 1)
      break;

    if( n < 0)  
    {
      printf("Read Error \n");
      continue;
    }

    if(ntohs(client.sin_port) == server_port)
    {
      int *temp_client_ports, temp_port, temp_count, i;
      char *token, *rep;
      rep = recvBuff;
      printf("Server Replied: %s\n", recvBuff);
      temp_count = client_count;
      client_count = 0;

      while ((token = strsep(&rep, " ")) != '\0' )
      {
        sscanf(token, "%d", &temp_port);    
        client_count++;

        if(client_count > temp_count)
        {
          client_ports = realloc(client_ports, (client_count) * sizeof(int));
          client_ports[client_count - 1] = temp_port;
          temp_count = client_count;
        }
      }
      continue;
    }
    else if( strcmp(recvBuff,"Received") == 0 )
    {
      printf("\nReply: %s\n", recvBuff);
      continue;
    }

    printf("'\nHere is the message: %s\n", recvBuff);
	
    strcpy(sendBuff, "Received");
	
    sendMessage(ntohs(client.sin_port), sendBuff, listenfd, 0 );
  }

  printf("Socket shutdown and listening stopped...\n");

}

int createSocket(int *port){
  int listenfd;
  struct sockaddr_in serv_addr;
  socklen_t s_len;

  listenfd = socket(AF_INET, SOCK_DGRAM, 0);

  printf("Socket retrieve success.\n");

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(0);

  bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
  s_len = sizeof(serv_addr);
  getsockname(listenfd, (struct sockaddr*) &serv_addr, &s_len);
  *port = ntohs(serv_addr.sin_port);
  return listenfd;

}

int randPort()
{
  srand(time(NULL));
  int r = rand() % 60000 ;
  return r + 10000;
}

void stopListening()
{
  listenCondition = -1;
}
