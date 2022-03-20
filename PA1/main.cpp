#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

// #include <fstream>

static int MAX_NUM;

struct obj
{
    char cval;
    int dec;
    std::string code;
    int frequency = 0;
    std::string mess;

    int index;
};

struct D
{
    std::map<std::string, char> ht;
    std::string encoded;
    std::string decoded;
    int index;
};

void readInput(int &numthread, std::vector<obj> &objL, std::string &msg)
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
        int curnum = stoi(line.substr(2)); // gets the rest of the line into a string, so any length number. but it skips over the whitespace

        obj object;

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
    std::string message = temp_ptr->mess;

    (*temp_ptr).code = dectobin((*temp_ptr).dec, MAX_NUM);

    (*temp_ptr).frequency = countFreq((*temp_ptr).code, MAX_NUM, message);

    // std::cout << "Thread #: " << (*temp_ptr).index << std::endl;

    return nullptr;
}

void *DecSolve(void *y_ptr)
{
    struct D *temp_ptr = (struct D *)y_ptr;

    std::string msg = temp_ptr->encoded;

    (*temp_ptr).decoded = (*temp_ptr).ht[msg];

    // std::cout << "HT ADDRESS: " << &(*temp_ptr).ht << std::endl;

    // std::cout << "DecSolve Thread #: " << (*temp_ptr).index << std::endl;

    return nullptr;
}

int main()
{
    std::vector<obj> objlist;
    std::string message;
    std::map<std::string, char> hashmap;

    int NTHREADS;

    readInput(NTHREADS, objlist, message);

    pthread_t th[NTHREADS];

    for (int i = 0; i < NTHREADS; i++)
    {
        objlist[i].mess = message;
        objlist[i].index = i;
        if (pthread_create(&th[i], nullptr, Solve, &objlist[i]))
        {
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_join(th[i], nullptr);
    }

    for (int i = 0; i < objlist.size(); i++)
    {

        hashmap[objlist[i].code] = objlist[i].cval;
    }

    std::cout << "Alphabet:" << std::endl;

    for (int i = 0; i < objlist.size(); ++i)
    {
        std::cout << "Character: " << objlist[i].cval << ", Code: " << objlist[i].code << ", Frequency: " << objlist[i].frequency << std::endl;
    }

    int MTHREADS = message.size() / MAX_NUM;

    pthread_t mth[MTHREADS];

    std::vector<struct D> charArr(MTHREADS);

    for (int i = 0; i < MTHREADS; i++)
    {

        std::string newstr = message.substr(0, MAX_NUM);

        message = message.substr(MAX_NUM);

        struct D dobj;
        dobj.ht = hashmap;
        dobj.encoded = newstr;
        dobj.index = i;
        charArr[i] = dobj;

        if (pthread_create(&mth[i], nullptr, DecSolve, &charArr[i]))
        {
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < MTHREADS; i++)
    {
        pthread_join(mth[i], nullptr);
    }

    std::string decompressed = "";

    for (int i = 0; i < charArr.size(); i++)
    {
        decompressed += charArr[i].decoded;
    }

    std::cout << std::endl;

    std::cout << "Decompressed message: " << decompressed << std::endl;

    return 0;
}