#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <vector>
#include <cmath>

#include <fstream>

struct obj
{
    char cval;
    int dec;
    std::string code;
    int frequency = 0;
};

void readInput(std::vector<obj> &objL, std::string &msg, int &numLength)
{
    std::string count;
    // std::cout << "Enter you loop size: ";
    getline(std::cin, count);

    int maxnum = 0;

    std::string line;

    for (int i = 0; i < stoi(count); i++)
    {
        // std::cout << "Enter input: ";
        getline(std::cin, line);

        // line = std::regex_replace(line, std::regex("^ +| +$|( ) +"), "$1"); // removes leading, trailing and extra spacing;

        char c = line[0];                  // gets the first character in the line
        int curnum = stoi(line.substr(2)); // gets the rest of the line into a string, so any length number. but it skips over the whitespace

        obj object;

        object.cval = c;
        object.dec = curnum;

        objL.push_back(object);

        maxnum = std::max(maxnum, curnum);
    }

    // std::cout << "maxnum: " << maxnum << std::endl;

    numLength = std::ceil(log2(maxnum + 1));

    // std::cout << "Enter decompressed message: ";
    getline(std::cin, line);
    msg = line;
}


std::string dectobin(int num, int maxsize)
{
    std::string longbin = std::bitset<32>(num).to_string();

    std::string cur = "";

    cur = longbin.substr(32 - maxsize);

    return cur;
}

void countFreq(obj &curobj, int numL, std::string message)
{
    for (int j = 0; j < message.size(); j += numL)
    {
        std::string cur = message.substr(j, numL);

        if (cur == curobj.code)
        {
            curobj.frequency++;
        }
    }
}


std::string decompressStr(std::string message, std::map<std::string, char> &ht, int maxL)
{
    std::string decompressed = "";
    for (int i = 0; i < message.size(); i += maxL)
    {
        std::string curstr = message.substr(i, maxL);
        decompressed += ht[curstr];
    }

    return decompressed;
}

int main()
{
    std::vector<obj> objlist;
    std::string message;
    int numL;
    std::map<std::string, char> hashmap;

    readInput(objlist, message, numL);

    // std::cout << "Message: " << message << std::endl;
    // std::cout << "Max numlength: " << numL << std::endl;

    // std::cout << "objlist size: " << objlist.size() << std::endl;

    std::cout << "Alphabet:" << std::endl;

    for (int i = 0; i < objlist.size(); i++)
    {
        objlist[i].code = dectobin(objlist[i].dec, numL);

        countFreq(objlist[i], numL, message);

        hashmap[objlist[i].code] = objlist[i].cval;

        std::cout << "Character: " << objlist[i].cval << ", Code: " << objlist[i].code << ", Frequency: " << objlist[i].frequency << std::endl;
    }

    // for(auto const& x : hashmap) {
    //     std::cout << x.first << " " << x.second << std::endl;
    // }

    std::string decompressed = decompressStr(message, hashmap, numL);

    std::cout << std::endl;

    std::cout << "Decompressed message: " << decompressed << std::endl;

    return 0;
}