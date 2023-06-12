#include "GeneralData.h"

#define HISTORYBUFFER 20000
#define MAXPENDING 30 /* Maximum outstanding connection requests */
int sock;         /* Socket descriptor for server */

void my_handler(int nsig)
{
    close(sock);
    exit(0);
}

int cmp(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

// структура сохраненного запроса
struct request
{
    int client_type;
    int client_index;
    int client_value;
};

int main(int argc, char *argv[])
{

    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    int history_iterator = 0;

    //cоздаю историю для хранения запрососов
    struct request history[HISTORYBUFFER];

    if (argc != 3) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }
    // инициализация базы данных и заполнение ее данными
    int dataBase[BUFFER_SIZE];

    for (int i = 0; i < BUFFER_SIZE; ++i)
    {
        dataBase[i] = i;
    }

    echoServPort = atoi(argv[2]); /* First arg:  local port */
    (void)signal(SIGINT, my_handler);
    /* Create socket for sending/receiving datagrams */
    if (((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))) < 0)
        perror("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        perror("bind() failed");
        
     printf("Server IP address = %s. Wait...\n", inet_ntoa(echoClntAddr.sin_addr));
    for (;;)
    { /* Run forever */
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
        int request[5];
        int answer[5];
        int recvMsgSize; /* Size of received message */

        cliAddrLen = sizeof(echoClntAddr);
        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, request, sizeof(request), 0,
                                    (struct sockaddr *)&echoClntAddr, &cliAddrLen)) < 0)
            perror("recvfrom() failed");

        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        int index = request[1];
        int data = request[2];

        // обработка клиента писателя
        if (request[0] == WRITER)
        {
            dataBase[index] = data;
            printf("Сlient writer wanted to write to a cell with an index %d number: %d\n", index, data);
            qsort(dataBase, BUFFER_SIZE, sizeof(int), cmp);
            answer[0] = request[0];
            answer[1] = request[1];
            answer[2] = dataBase[index];
            if (history_iterator < HISTORYBUFFER)
            {
                history[history_iterator].client_type = answer[0];
                history[history_iterator].client_index = answer[1];
                history[history_iterator].client_value = answer[2];
                history_iterator++;
            }
            else
            {
                printf("Server can't save history \n");
            }
        }
         // обработка клиента читателя
        if (request[0] == READER)
        {
            printf("Сlient read wanted to read to a cell with an index %d number: %d\n", index, dataBase[index]);
            answer[0] = request[0];
            answer[1] = request[1];
            answer[2] = dataBase[index];
            if (history_iterator < HISTORYBUFFER)
            {
                history[history_iterator].client_type = answer[0];
                history[history_iterator].client_index = answer[1];
                history[history_iterator].client_value = answer[2];
                history_iterator++;
            }
            else
            {
                printf("Server can't save history \n");
            }
        }

         // обработка клиента наблюдателя
        if (request[0] == VISITOR)
        {   
            if(request[3] < HISTORYBUFFER && request[3] <= history_iterator){
                // передаю сохраненные данные
                answer[0] = history[request[3]].client_type;
                answer[1] = history[request[3]].client_index;
                answer[2] = history[request[3]].client_value;
                answer[4] = 1;
                // обновляю итератор клиента
                answer[3] = request[3] + 1;
            }
            else{
                printf("Can't send new information to visitor\n");
                answer[4] = 0;
            }
            printf("Send to client visitor information\n");
        }


        /* Send received datagram back to the client */
        if (sendto(sock, answer, sizeof(answer), 0,
                   (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
            perror("sendto() sent a different number of bytes than expected");
    }
    /* NOT REACHED */
}
