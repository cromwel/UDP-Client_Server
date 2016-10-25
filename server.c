#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

int randPort();
void *handleClient(void *args);

struct sockaddr_in serv_addr; 

char client_adrs [100][15];
char str_addresses[1025];
int client_ports[100];
int client_count = 0, found = 0;;

int port, sockfd;

int main(void)
{
  int connfd = 0; // listen descriptor, connection descriptor
  socklen_t s_len;
  port = randPort();
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  printf("Socket retrieve success\n");

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  // Well use OS provided ports
  serv_addr.sin_port = htons(0);
  bind(sockfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
  s_len = sizeof(serv_addr);
  getsockname(sockfd, (struct sockaddr*) &serv_addr, &s_len);
  port = ntohs(serv_addr.sin_port);

  printf("Running on port: %d\n", port);



  while(1)
  {
    int i ;
    int n = 0;
    char recvBuff[1024];
    pthread_t conn_thread;

    struct sockaddr_in client;
    socklen_t c_len = sizeof(client);


    memset(recvBuff, '\0', sizeof(recvBuff));
    n = recvfrom(sockfd,recvBuff, sizeof(recvBuff), 0, (struct sockaddr *)&client, &c_len); 
    if( n < 0)  
    {
      printf("Read Error!\n");
      continue;
    }
    printf("Message: %s\n", recvBuff);
    // Fork thread to handle multiple clients 
    pthread_create(&conn_thread,NULL, handleClient,(void*)ntohs(client.sin_port) );
  }


  return 0;
}

void *handleClient(void *args)
{ 
  int i ,client_port, cur_index;
  int n = 0;
  char recvBuff[1024];
  char sendBuff[1025];

  pthread_t conn_thread;

  struct sockaddr_in client;
  char temp[4];

  found = 0;
  memset(sendBuff, '\0', sizeof(sendBuff));

  client_port = (int) args;

  client.sin_family = AF_INET;
  client.sin_addr.s_addr = inet_addr("127.0.0.1");
  client.sin_port = htons(client_port);

  for( i = 0; i < client_count; i++)
  {
    if(client_ports[i] == client_port )
    {
      cur_index = i;
      printf(" user found\n");
      found += 1;
    }
  }

  if(found == 0){
    client_ports[client_count] = client_port;

    sprintf(temp, "%d", client_port);
    if(client_count != 0)
          strcat(str_addresses, " ");
    strcat(str_addresses, temp);
    cur_index = client_count;
  }
  
  char dst[255]; 

  strcpy(client_adrs[cur_index], dst);
  printf("Connected to 127.0.0.1:%d\n", client_ports[cur_index]);

  printf("Clients connected: %s\n\n", str_addresses);
  client_count ++;

  strcpy(sendBuff, str_addresses);

  n = sendto(sockfd, sendBuff, strlen(sendBuff), 0,(struct sockaddr *)&client, sizeof(client));
  if( n < 0)  
  {
    perror("Send error\n");
  }
}

int randPort(){
  srand(time(NULL));
  int r = rand() % 60000 ;
  return r + 10000;
}
