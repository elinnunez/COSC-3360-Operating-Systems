// socket creation code from rincon blackboard template
// fireman function code from rincon template and https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
// binding issues code fix from: https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr

#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cmath>
#include <regex>
#include <unordered_map>

struct obj
{
    char cval;
    int dec;
    std::string bin;
};

std::string dectobin(int num, int maxsize)
{
    std::string longbin = std::bitset<64>(num).to_string();

    std::string cur = "";

    cur = longbin.substr(64 - maxsize);

    return cur;
}

void readInput(std::vector<obj> &objL, int &MAX_SIZE)
{
    std::string count;
    getline(std::cin, count);

    int maxi = 0;

    std::string line;

    for (int i = 0; i < stoi(count); i++)
    {

        getline(std::cin, line);

        char c = line[0];
        int curnum = stoi(line.substr(2)); // gets the rest of the line into a string, so any length number. but it skips over the whitespace

        obj object;

        object.cval = c;
        object.dec = curnum;

        objL.push_back(object);

        maxi = std::max(maxi, curnum);
    }

    MAX_SIZE = std::ceil(log2(maxi + 1));
}

void fireman(int)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
        // std::cout << "A child process ended" << std::endl;
}

int main(int argc, char *argv[])
{

    int MAX_SIZE;
    std::vector<obj> objL;
    readInput(objL, MAX_SIZE);

    std::unordered_map<std::string, char> hashmap;

    for (auto &curobj : objL)
    {
        curobj.bin = dectobin(curobj.dec, MAX_SIZE);
        hashmap[curobj.bin] = curobj.cval;
        // std::cout << "Char: " << curobj.cval << " Dec: " << curobj.dec << " Bin: " << curobj.bin << "\n";
    }

    int sockfd, newsockfd, portnumber, cli_len;

    char binmsg[256];

    struct sockaddr_in server_addr, cli_addr;

    int n;

    if (argc < 2)
    {
        std::cout << "ERROR, no port provided\n";
        exit(1);
    }

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        std::cout << "ERROR opening socket\n";
        exit(1);
    }


    //binding issue fix from https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    // Set all bits of the field to 0
    memset((char *)&server_addr, 0, sizeof(server_addr));

    // Set argument to int for portnumber
    portnumber = atoi(argv[1]);

    // Configure settings of the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portnumber);

    // Bind the address struct to the socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        // std::cout << "ERROR on binding\n";
        perror("Binding Error");
        exit(1);
    }

    // Listen on the socket with max_size+1 max connections requests queued
    listen(sockfd, MAX_SIZE+1);

    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_len);

    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        exit(1);
    }

    memset(binmsg, 0, 256);

    n = read(newsockfd, binmsg, sizeof(binmsg));

    if (n < 0)
    {
        perror("ERROR reading from socket");
        exit(1);
    }

    // std::cout << "Binary message from client: " << binmsg << "\n";

    n = write(newsockfd, &MAX_SIZE, sizeof(int));

    if (n < 0)
    {
        perror("ERROR writing to socket");
       
        exit(1);
    }

    close(newsockfd);

    signal(SIGCHLD, fireman);
    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_len);

        if (newsockfd < 0)
        {
            perror("New socket error on accept");
            exit(1);
        }

        pid_t pid = fork();

        if(pid < 0) {
            perror("Error on fork");
            exit(1);
        }

        if (pid == 0)
        {
            // std::cout << "A child process started" << std::endl;
            close(sockfd);

            char buffer[256];

            memset(buffer, 0, 256);

            int num = read(newsockfd, buffer, sizeof(buffer));

            if (num < 0)
            {
                perror("ERROR reading from socket in fork");
                close(newsockfd);
                exit(1);
            }

            // std::cout << "Here is the bin str: " << buffer << "\n";

            n = write(newsockfd, &hashmap[buffer], sizeof(char));

            if (n < 0)
            {
                perror("ERROR writing to socket from fork");
                close(newsockfd);
                exit(1);
            }

            close(newsockfd);
            _exit(0);
        }
        else
        {
            close(newsockfd);
        }
        std::cin.get();
    }

    close(newsockfd);

    close(sockfd);

    return 0;
}