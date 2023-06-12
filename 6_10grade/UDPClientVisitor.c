#include "GeneralData.h"
#define ECHOMAX 255    /* Longest string to echo */
#define TIMEOUT_SECS 2 /* Seconds between retransmits */
#define MAXTRIES 5     /* Tries before giving up */

int tries = 0; /* Count of times sent - GLOBAL for signal-handler access */

int sock; /* Socket descriptor */

void CatchAlarm(int ignored); /* Handler for SIGALRM */

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
    int number_of_readers;

    if (argc != 3) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>]\n",
                argv[0]);
        exit(1);
    }

    // обработка сигнала прерывания
    (void)signal(SIGINT, my_handler);

    if (argc != 3) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>]\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];             /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]); /* Use given port, if any */

    int request[5];

    request[0] = VISITOR;

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        perror("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */
    srand(0);
    while (1)
    { /* Create a reliable, stream socket using TCP */

        /* Send the string to the server */
        if (sendto(sock, request, sizeof(request), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != sizeof(request))
            perror("sendto() sent a different number of bytes than expected");

        // массив для получения результата ответа сервера
        int answer[5];

        alarm(TIMEOUT_SECS); /* Set the timeout */
        while ((respStringLen = recvfrom(sock, answer, sizeof(answer), 0,
                                         (struct sockaddr *)&fromAddr, &fromSize)) < sizeof(answer))
            if (errno == EINTR) /* Alarm went off  */
            {
                if (tries < MAXTRIES) /* incremented by signal handler */
                {
                    printf("timed out, %d more tries...\n", MAXTRIES - tries);
                    if (sendto(sock, request, sizeof(request), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != sizeof(request))
                        perror("sendto() failed");
                    alarm(TIMEOUT_SECS);
                }
                else
                    perror("No Response");
            }
            else
                perror("recvfrom() failed");

        if (answer[4])
        {
            if (answer[0] == READER)
            {
                printf("Visitor: Process successfully read data from index %d: %d\n", answer[1], answer[2]);
            }
            else
            {
                printf("Visitor: Process successfully write data from index %d: %d\n", answer[1], answer[2]);
            }
            printf("\n");
            // передаю итератор клиента
            request[3] = answer[3];
        }
        sleep(1);
    }
    close(sock);
    return (0);
}
