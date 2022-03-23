// socket creation code from rincon blackboard template

#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

// struct test { // testing
//     char binn[256];
//     int idx;
// };

struct D
{
    std::string encoded;
    char decoded;
    int *portno;
    char *servername;
    // int index; // testing
};

void *Send(void *x_ptr)
{
    struct D *temp_ptr = (struct D *)x_ptr;

    int newsockfd, portnumber;

    struct sockaddr_in server_address;
    struct hostent *server;

    portnumber = *temp_ptr->portno;

    newsockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (newsockfd < 0)
    {
        std::cout << "Error creating socket in thread\n";
        exit(0);
    }

    server = gethostbyname(temp_ptr->servername);

    if (server == nullptr)
    {
        std::cout << "Error, no such host\n";
        exit(0);
    }

    memset((char *)&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;

    memcpy((char *)&server_address.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

    server_address.sin_port = htons(portnumber);

    int connection_status = connect(newsockfd, (struct sockaddr *)&server_address, sizeof(server_address));

    if (connection_status < 0)
    {
        std::cout << "Connection Error in thread\n";
        exit(0);
    }

    char charr[256];

    memset(charr, 0, 256);

    strcpy(charr, temp_ptr->encoded.c_str());

    // std::cout << "Writing: " << charr << std::endl;

    // struct test tester; // testing

    // strcpy(tester.binn, temp_ptr->encoded.c_str()); // testing
    // tester.idx = temp_ptr->index; // testing

    // int n = write(newsockfd, &tester, sizeof(struct test)); // testing

    int n = write(newsockfd, &charr, sizeof(charr));

    if (n < 0)
    {
        std::cout << "Error writing to socket from thread.\n";
        exit(0);
    }

    n = read(newsockfd, &temp_ptr->decoded, sizeof(char));

    if (n < 0)
    {
        std::cout << "Error reading from socket from thread.\n";
    }

    // std::cout << "Server to Thread Response:\n";
    // std::cout << "---------------------\n";
    // std::cout << "Encoded Char: " << temp_ptr->encoded << "\n";
    // std::cout << "Decoded Char: " << temp_ptr->decoded << "\n";
    // std::cout << "---------------------\n";

    // std::cout << "Thread: " << temp_ptr->index << "\n"; // testing

    close(newsockfd);

    return nullptr;
}

int main(int argc, char *argv[]) // argc == argument count, argv is the array of the arguments input
{

    int sockfd, portnumber, n, numbits;

    struct sockaddr_in server_address;
    struct hostent *server;

    char binmsg[256];

    if (argc < 3)
    {
        std::cout << "usage " << argv[0] << " hostname port\n";
        exit(0);
    }

    portnumber = atoi(argv[2]); // argument at arg[2] to int

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // 0 is default protocol

    if (sockfd < 0)
    {
        std::cout << "Error creating socket\n";
        exit(0);
    }

    server = gethostbyname(argv[1]); // replace with getaddrinfo()?

    if (server == nullptr)
    {
        std::cout << "Error, no such host\n";
        exit(0);
    }

    memset((char *)&server_address, 0, sizeof(server_address)); // memset(void *s, int c, size_t n): Fills te first n bytes of the memory area pointed to by s with the constant byte c

    server_address.sin_family = AF_INET;

    memcpy((char *)&server_address.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

    server_address.sin_port = htons(portnumber);

    int connection_status = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));

    if (connection_status < 0)
    {
        std::cout << "Connection Error\n";
        exit(0);
    }

    memset(binmsg, 0, 256);

    // std::cout << "Enter Binary Message: \n";
    std::string stringval;
    getline(std::cin >> std::ws, stringval);
    strcpy(binmsg, stringval.c_str());

    n = write(sockfd, &binmsg, sizeof(binmsg));

    if (n < 0)
    {
        std::cout << "Error writing to socket.\n";
        exit(0);
    }

    n = read(sockfd, &numbits, sizeof(int));

    if (n < 0)
    {
        std::cout << "Error reading from socket.\n";
    }

    // std::cout << "Server Response:\n";
    // std::cout << "---------------------\n";
    // std::cout << "Num of bits: " << numbits << "\n";
    // std::cout << "---------------------\n";

    close(sockfd);

    int MTHREADS = stringval.size() / numbits;

    pthread_t mth[MTHREADS];

    struct D charArr[MTHREADS];

    for (int i = 0; i < MTHREADS; i++)
    {

        std::string newstr = stringval.substr(0, numbits);

        stringval = stringval.substr(numbits);

        struct D dobj;
        dobj.encoded = newstr;
        dobj.portno = &portnumber;
        dobj.servername = argv[1];
        // dobj.index = i; // testing

        charArr[i] = dobj;

        if (pthread_create(&mth[i], nullptr, Send, &charArr[i]))
        {
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
    }

    std::string decompressed = "";

    for (int i = 0; i < MTHREADS; i++)
    {
        pthread_join(mth[i], nullptr);
        decompressed += charArr[i].decoded;
    }

    std::cout << "Decompressed message: " << decompressed << "\n";

    return 0;
}