#include "GeneralData.h"

int sock; /* Socket descriptor */
void my_handler(int nsig)
{
    close(sock);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    unsigned short echoServPort;     /* Echo server port */
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    char *servIP;                    /* IP address of server */
    char *echoString;                /* String to send to echo server */
    int echoStringLen;               /* Length of string to echo */
    int respStringLen;               /* Length of received response */
    int number_of_writers;

    if (argc != 4) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>] [<number_of_writers>]\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];                  /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]);      /* Use given port, if any */
    number_of_writers = atoi(argv[3]); /* Second arg: string to echo */

    // обработка сигнала прерывания
    (void)signal(SIGINT, my_handler);

    // создаю клиентов
    srand(0);
    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        perror("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */

    for (int i = 0; i < number_of_writers; ++i)
    {
        int numbers[3];
        // echoLen = sizeof(numbers); /* Determine input length */

        int index = rand() % BUFFER_SIZE;
        int data = rand() % 1000000;

        numbers[0] = WRITER;
        numbers[1] = index;
        numbers[2] = data;

        /* Send the string to the server */
        if (sendto(sock, numbers, sizeof(numbers), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != sizeof(numbers))
            perror("sendto() sent a different number of bytes than expected");

        printf("%d) Received: ", i); // Setup to print the echoed string

        // массив для получения результата ответа сервера
        int result[3];

        /* Recv a response */
        fromSize = sizeof(fromAddr);
        if ((respStringLen = recvfrom(sock, result, sizeof(result), 0,
                                      (struct sockaddr *)&fromAddr, &fromSize)) != sizeof(numbers))
            perror("recvfrom() failed");

        // if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
        // {
        //     fprintf(stderr, "Error: received a packet from unknown source.\n");
        //     exit(1);
        // }

        printf("Writer: Process %d successfully write data from index %d: %d\n", i, result[1], result[2]);
        printf("\n"); /* Print a final linefeed */
        sleep(1);
    }
    close(sock);
    exit(0);
}
