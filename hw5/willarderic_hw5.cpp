#include <cstdlib>
#include <stdio.h>
#include <strings.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>

void mapperJob();
void reducerJob();
int readFile(const std::string &file);
bool parseConfig();
bool stob(const std::string &str);

// Map of all the configuration options defaults. Only works with c++11
std::map<std::string,std::string> configurationMap = {{"mappers", "4"},
                                                     {"reducers", "4"},
                                                     {"caseSensitive", "true"},
                                                     {"mapAndReduce", "true"}};

std::mutex fileMtx;
std::mutex countMtx;
std::queue<std::string> fileQueue;
std::queue<int> wordCountQueue;
std::string target;
int mappers;

int main() {
    if (!parseConfig()) return 0;

    int numMappers = stoi(configurationMap.at("mappers"));
    int numReducers = stoi(configurationMap.at("reducers"));
    mappers = numMappers;
    bool mapAndReduce = stob(configurationMap.at("mapAndReduce"));
    // arrays of mappers and reducer threads
    std::thread mappers[numMappers];
    std::thread reducers[numReducers];
    
    std::ifstream file;
    std::string line;
    // read the list of files to map-reduce
    std::cout << "Enter in the string you would like to search for: ";
    std::cin >> target;
    file.open("files.dat");
    while(file >> line) {
        fileQueue.push(line);
    }
    // launcher all mapper and reducer threads
    for (int i = 0; i < numMappers; ++i) {
         mappers[i] = std::thread(mapperJob);
    }
    // join mapper threads and run reducer threads
    // order depends on config file
    for (int i = 0; i < (mapAndReduce ? numReducers : numMappers); ++i) {
        if (mapAndReduce) {
            reducers[i] = std::thread(reducerJob);
        } else {
            mappers[i].join();
        }
    }
    for (int i = 0; i < (mapAndReduce ? numMappers : numReducers); ++i) {
        if (mapAndReduce) {
            mappers[i].join();
        } else {
            reducers[i] = std::thread(reducerJob);
        }
    }
    for (int i = 0; i < numReducers; ++i) {
        reducers[i].join();
    }
    // int count = wordCountQueue.front();
    // wordCountQueue.pop();
    int total = wordCountQueue.front();
    std::cout << "\nTotal count of " << target << ": " << total << std::endl;
    // std::cout << "The total count for the word \"" << wordToCount << "\" is " << count << std::endl;

    return 0;
}

void mapperJob() {
    while (!fileQueue.empty()) {
        // file name that the mapper reads
        std::string fileName;
        // lock the file queue
        fileMtx.lock();
        if (!fileQueue.empty()) {
            fileName = fileQueue.front();
            fileQueue.pop();
        }
        fileMtx.unlock();
        // count occurences of word
        int count = readFile(fileName);

        std::cout << fileName << ": " << count << std::endl;
        // lock the count queue
        countMtx.lock();
        wordCountQueue.push(count);
        countMtx.unlock();
    }
    // mapper is finished with it's job
    mappers--;
}

void reducerJob() {
    while (wordCountQueue.size() > 1 || mappers > 0) {
        int first = 0, second = 0;
        // lock count queue
        countMtx.lock();
        // make sure there are two elements to add
        if (wordCountQueue.size() > 1) {
            first = wordCountQueue.front();
            wordCountQueue.pop();
            second = wordCountQueue.front();
            wordCountQueue.pop();
        }
        countMtx.unlock();
        int sum = first + second;
        // if sum is 0, first and second may have not been assigned values from the queue.
        // even if they were, this reduces a computation because adding 0 to the next value will
        // not do anything.
        if (sum > 0) {
            // lock the count queue
            countMtx.lock();
            wordCountQueue.push(sum);
            countMtx.unlock();
        }
    }
}

// reads the file and counts the number of the string provided
int readFile(const std::string &fileName) {
    std::string word;
    std::ifstream file;
    int count = 0;
    std::string tmp;
    file.open(fileName);
    bool match = false;
    bool caseSensitive = stob(configurationMap.at("caseSensitive"));
    while (file >> word) {
        tmp = word;
        if (caseSensitive) {
            if (word == target) {
                count++;
            }
        } else {
            if (strcasecmp(word.c_str(),target.c_str()) == 0) {
                count++;
            }
        }
    }

    return count;
}

// parses config file named 'config.cfg'
// config file can have comments on their own line starting with '#'
/**
 * Example Configuration options
 * mappers = 5
 * reducers = 5
 * mapAndReduce = true
 * caseSensitive = true
 * punctuationSensitive = true
**/
bool parseConfig() {
    std::ifstream cfg;
    cfg.open("config.cfg");
    if (!cfg) {
        std::cout << "No config file found. Using defaults: " << std::endl;
    } else { 
        std::cout << "Using configurations: " << std::endl;
    }
    
    std::string line, key, value;
    while (getline(cfg, line)) {
        if (line[0] != '#') {
            replace(line.begin(), line.end(), '=', ' ');
            std::cout << line << std::endl;
            std::stringstream stream(line);
            stream >> key >> value;
            configurationMap.at(key) = value;
        }
    }
    std::map<std::string,std::string>::iterator it = configurationMap.begin();
    while (it != configurationMap.end()) {
        std::cout <<"\t\t"<< it->first << ": " << it->second << std::endl;
        it++;
    }
    return true;
}

// String to boolean
bool stob(const std::string &str) {
    if (strcasecmp(str.c_str(), "true") == 0) {
        return true;
    } else {
        return false;
    } 
}
