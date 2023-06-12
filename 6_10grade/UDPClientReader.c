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

    if (argc != 4) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>] [<number_of_readers>]\n",
                argv[0]);
        exit(1);
    }

    // обработка сигнала прерывания
    (void)signal(SIGINT, my_handler);

    servIP = argv[1];                  /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]);      /* Use given port, if any */
    number_of_readers = atoi(argv[3]); /* Second arg: string to echo */
                                       /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        perror("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */
    srand(0);
    for (int i = 0; i < number_of_readers; ++i)
    {
        int numbers[5];

        int index = rand() % BUFFER_SIZE;
        int data = rand() % BUFFER_SIZE;

        numbers[0] = READER;
        numbers[1] = index;
        numbers[2] = data;

        /* Send the string to the server */
        if (sendto(sock, numbers, sizeof(numbers), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != sizeof(numbers))
            perror("sendto() sent a different number of bytes than expected");

        printf("%d) Received: ", i); // Setup to print the echoed string

        // массив для получения результата ответа сервера
        int result[5];
        /* Recv a response */
        fromSize = sizeof(fromAddr);

        alarm(TIMEOUT_SECS); /* Set the timeout */
        while ((respStringLen = recvfrom(sock, result, sizeof(result), 0,
                                         (struct sockaddr *)&fromAddr, &fromSize)) < 0)
            if (errno == EINTR) /* Alarm went off  */
            {
                if (tries < MAXTRIES) /* incremented by signal handler */
                {
                    printf("timed out, %d more tries...\n", MAXTRIES - tries);
                    if (sendto(sock, numbers, sizeof(numbers), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != sizeof(numbers))
                        perror("sendto() failed");
                    alarm(TIMEOUT_SECS);
                }
                else
                    perror("No Response");
            }
            else
                perror("recvfrom() failed");

        /* recvfrom() got something --  cancel the timeout */
        alarm(0);

        printf("Reader: Process %d successfully read data from index %d: %d\n", i, result[1], result[2]);
        printf("\n"); /* Print a final linefeed */
        sleep(1);
    }
    close(sock);
    return (0);
}
