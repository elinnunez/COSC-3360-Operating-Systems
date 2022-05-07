#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

struct og
{
    char cval;
    int dec;
};

struct obj
{
    std::map<std::string, char> ht;
    char cval;
    int dec;
    std::string code;
    int frequency = 0;
    std::string *mess;
    int *max_num;
    int index;
    pthread_mutex_t *bsem;
    pthread_cond_t *wait;
    int *turn;
};

struct arg
{
    std::map<std::string, char> ht;
    std::string *encoded;
    int index;
    pthread_mutex_t *msem;
    pthread_cond_t *wait;
    int *turn;
    std::string cur;
};

void readInput(int &numthread, std::vector<og> &objL, std::string &msg, int &MAX_NUM)
{
    std::string count;
    getline(std::cin, count);

    int maxi = 0;

    numthread = stoi(count);

    std::string line;

    for (int i = 0; i < stoi(count); i++)
    {
        getline(std::cin, line);

        char c = line[0];
        int curnum = stoi(line.substr(2));

        struct og object;

        object.cval = c;
        object.dec = curnum;

        objL.push_back(object);

        maxi = std::max(maxi, curnum);
    }

    MAX_NUM = std::ceil(log2(maxi + 1));

    getline(std::cin, line);
    msg = line;
}

std::string dectobin(int num, int maxsize)
{
    std::string longbin = std::bitset<64>(num).to_string();

    std::string cur = "";

    cur = longbin.substr(64 - maxsize);

    return cur;
}

int countFreq(std::string curcode, int numL, std::string message)
{
    int frequency = 0;
    for (int j = 0; j < message.size(); j += numL)
    {
        std::string cur = message.substr(j, numL);

        if (cur == curcode)
        {
            frequency++;
        }
    }

    return frequency;
}

void *Solve(void *x_ptr)
{
    struct obj *temp_ptr = (struct obj *)x_ptr;

    char charval = temp_ptr->cval;
    
    int tp_dec = temp_ptr->dec;
    
    int tp_maxnum = *temp_ptr->max_num;
    
    std::string tp_mess = *temp_ptr->mess;

    int tp_curidx = temp_ptr->index;

    // std::cout << "Thread " << temp_ptr->index << " copying\n";

    pthread_mutex_unlock(temp_ptr->bsem);

    // std::cout << "Thread " << temp_ptr->index << " work\n";
    
    std::string curcode = dectobin(tp_dec, tp_maxnum);

    int curfreq = countFreq(dectobin(tp_dec, tp_maxnum), tp_maxnum, tp_mess);

    pthread_mutex_lock(temp_ptr->bsem);

    
    while(*temp_ptr->turn != tp_curidx)
    {
        pthread_cond_wait(temp_ptr->wait, temp_ptr->bsem);
    }

    pthread_mutex_unlock(temp_ptr->bsem);

    std::cout << "Character: " << charval << ", Code: " << curcode << ", Frequency: " << curfreq << std::endl;

    pthread_mutex_lock(temp_ptr->bsem);

    *(temp_ptr->turn) = *(temp_ptr->turn) + 1;

    pthread_cond_broadcast(temp_ptr->wait);
    pthread_mutex_unlock(temp_ptr->bsem);

    return nullptr;
}

void *DecSolve(void *y_ptr)
{
    struct arg *temp_ptr = (struct arg *)y_ptr;

    char tp_cval = temp_ptr->ht[*temp_ptr->encoded];

    int tp_curidx = temp_ptr->index;

    pthread_mutex_unlock(temp_ptr->msem);

    pthread_mutex_lock(temp_ptr->msem);

    while(*temp_ptr->turn != tp_curidx)
    {
        pthread_cond_wait(temp_ptr->wait, temp_ptr->msem);
    }

    temp_ptr->cur += tp_cval;

    pthread_mutex_unlock(temp_ptr->msem);

    pthread_mutex_lock(temp_ptr->msem);

    *(temp_ptr->turn) = *(temp_ptr->turn) + 1;

    pthread_cond_broadcast(temp_ptr->wait);
    pthread_mutex_unlock(temp_ptr->msem);

    return nullptr;
}

int main()
{
    static pthread_mutex_t bsem;
    static pthread_cond_t waitTurn;
    int turn = 0;

    std::vector<og> objlist;
    std::string message;
    std::map<std::string, char> hashmap;
    static int MAX_NUM;

    int NTHREADS;

    readInput(NTHREADS, objlist, message, MAX_NUM);

    pthread_t th[NTHREADS];

    struct obj arg;
    arg.mess = &message;
    arg.max_num = &MAX_NUM;
    arg.bsem = &bsem;     // binary semaphore
    arg.wait = &waitTurn; // cond var
    arg.ht = hashmap;
    arg.turn = &turn;

    std::cout << "Alphabet:\n";

    for (int i = 0; i < NTHREADS; i++)
    {
        std::string htkey = dectobin(objlist[i].dec, MAX_NUM);

        hashmap[htkey] = objlist[i].cval;

        pthread_mutex_lock(&bsem);

        arg.dec = objlist[i].dec;
        arg.cval = objlist[i].cval;
        arg.index = i;

        // std::cout << "Thread " << arg.index << " modify\n";

        if (pthread_create(&th[i], nullptr, Solve, &arg))
        {
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_join(th[i], nullptr);
    }

    int MTHREADS = message.size() / MAX_NUM;

    pthread_t mth[MTHREADS];

    static pthread_mutex_t msem;
    static pthread_cond_t cond;
    turn = 0;

    struct arg argd;
    argd.ht = hashmap;
    argd.msem = &msem;
    argd.wait = &cond;
    argd.turn = &turn;

    for (int i = 0; i < MTHREADS; i++)
    {
        pthread_mutex_lock(&msem);

        std::string newstr = message.substr(0, MAX_NUM);

        message = message.substr(MAX_NUM);

        argd.encoded = &newstr;
        argd.index = i;

        if (pthread_create(&mth[i], nullptr, DecSolve, &argd))
        {
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < MTHREADS; i++)
    {
        pthread_join(mth[i], nullptr);
    }

    std::cout << "\nDecompressed message: " << argd.cur << std::endl;

    return 0;
}
