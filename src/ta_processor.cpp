#include "./ta_processor.hpp"
#include <iostream>
#include <ta-lib/ta_func.h>
#include <exception>
#include <math.h>
#include <atomic>
#include <algorithm>
#include <cstdio>

std::mutex processedMutex;
std::unordered_map<std::string, double*> processedData = {};

double testCalc(std::unordered_map<std::string, double*> data, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;
    if (TA_RSI(blockIndex, blockIndex, std::as_const(data["close"]), 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}

void workerThread(std::vector<TAFunction> taFuncs, std::unordered_map<std::string, double*> data, size_t minTicks, size_t blockSize) {
    for (size_t i = minTicks; i < blockSize; i++) {
        
        // Stop loop once initialized values are reached
        if (data[data.begin()->first][i] == 0) break;

        for (TAFunction ta : taFuncs) {
            double res = ta.func(data, i);      

            processedMutex.lock();
            processedData[ta.name][i] = res;
            processedMutex.unlock();
        }
    }
}

TAProcessor::TAProcessor(std::string unprocessedDir, std::string processedDir, size_t minTicks, size_t blockSize) {
    processedData = {};
    
    this->blockSize = blockSize;
    this->unprocessedDir = unprocessedDir + "/";
    this->processedDir = processedDir + "/";
    this->minTicks = minTicks;
    this->tickData = {};
    
    this->addIndictator("rsi", testCalc);
}

void TAProcessor::processHeader(std::unordered_map<size_t, std::string> *columnID, std::string header) {
    std::vector<std::string> names = {};
    split(&names, header, ',', false);
    for (size_t i = 0; i < names.size(); i++) {
        std::string name = names[i];
        (*columnID)[i] = name;
        
        double* column = (double*)calloc(sizeof(double), blockSize);
        tickData[name] = column;
    }
}

void TAProcessor::addIndictator(std::string taName, CalculateIndicator func) {
    taFuncs[taName] = func;
    processedData[taName] = (double*)calloc(sizeof(double), blockSize);
}

void TAProcessor::prepareNextBlock() {
    for (auto it : tickData) {
        int startCopyIndex = blockSize - minTicks;
        memcpy(it.second, it.second + startCopyIndex, sizeof(double) * minTicks);
        memset(it.second + minTicks, 0x00, sizeof(double) * (blockSize - minTicks));
    }

    for (auto it : processedData) {
        memset(it.second, 0x00, sizeof(double) * blockSize);
    }
}

void TAProcessor::writeHeader(std::string path) {
    std::ofstream file;
    std::stringstream ss;

    file.open(path, std::ios::out | std::ios::app);
    if (file.fail()) throw std::ios_base::failure(std::strerror(errno));    
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    // Append original tick data and processed column names
    for (auto it : tickData) {
        if (ss.str().empty()) {
            ss << it.first;
        } else {
            ss << "," << it.first;
        }
    }
    for (auto it : processedData) {
        ss << "," << it.first;
    }

    file << ss.str() << "\r\n";
    file.close();
}

void TAProcessor::appendProcessedBlock(std::string path) {
    std::ofstream file;
    std::stringstream ss;

    file.open(path, std::ios::out | std::ios::app);
    if (file.fail()) throw std::ios_base::failure(std::strerror(errno));    
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    for (size_t tick = minTicks; tick < blockSize; tick++) {        
        for (auto it : tickData) {
            auto num = it.second[tick];

            if (num == 0) {
                file.close();
                return;
            }
            
            if (ss.str().empty()) {
                ss << num;
            } else {
                ss << "," << num;
            }
        }

        for (auto it : processedData) {
            ss << "," << it.second[tick];
        }

        file << ss.str() << "\r\n";

        ss.str(std::string());
    }

    file.close();
}

void TAProcessor::processFile(std::string path) {
    std::string processedPath = processedDir + std::string(std::filesystem::path(path).filename());
    // Remove old processed file, make sure directory exists.
    std::remove(processedPath.c_str());
    std::filesystem::create_directory(processedDir);
    
    std::ifstream infile(path);
    std::string line;

    // Matches order of parsed string to column name.
    std::unordered_map<size_t, std::string> columnID = {};
    // Stores the parsed line strings
    std::vector<std::string> lineSplit = {};

    size_t blockIndex = 0;
    bool hasProcessedHeader = false;
    while(std::getline(infile, line)) {
        if (columnID.size() == 0) {
            processHeader(&columnID, line);
        }

        if (blockIndex == blockSize) {
            // Process current block (tickData)
            processBlock();
            blockIndex = minTicks;
            
            // Add header with original tick data and TA column names
            if (!hasProcessedHeader) {
                writeHeader(processedPath);
                hasProcessedHeader = true;
            }

            // Append results to file
            appendProcessedBlock(processedPath);

            // Clear arrays and maps for next block.
            prepareNextBlock();
            continue;
        }

        if (split(&lineSplit, line, ',', true) && lineSplit.size() == tickData.size()) {
            for (auto it : columnID) {
                tickData[it.second][blockIndex] = std::stod(lineSplit[it.first]);
            }
            blockIndex++;
        }
    }

    // Process remainder ticks.
    processBlock();            
    if (!hasProcessedHeader) writeHeader(processedPath);
    appendProcessedBlock(processedPath);
    
    
    for (auto it : tickData) {
        free(it.second);
    }

    infile.close();
    tickData.clear();
}

void TAProcessor::exec() {
    for (const auto & entry : std::filesystem::directory_iterator(unprocessedDir)) {
        processFile(entry.path());
    }
}

std::vector<std::unordered_map<std::string, double*>> TAProcessor::copyData(size_t copies) {
    std::vector<std::unordered_map<std::string, double*>> data = {};
    for (size_t i = 0; i < copies; i++) {
        std::unordered_map<std::string, double*> copy = {};
        for (auto it : tickData) {
            copy[it.first] = (double*)malloc(sizeof(double) * blockSize);
            memcpy(copy[it.first], it.second, sizeof(double) * blockSize);
        }
        data.push_back(copy);
    }
    return data;
}

 void TAProcessor::destroyData(std::vector<std::unordered_map<std::string, double*>> copies) {
    for (auto copy : copies) {
        for (auto it : copy) {
            free(it.second);
        }
    }
}

TAProcessor::~TAProcessor() {
    for (auto it : tickData) {
        free(it.second);
    }
    for (auto it : processedData) {
        free(it.second);
    }
}

void TAProcessor::processBlock() {
    // Minimum functions per hardware thread
    const size_t funcsPerThread = floor(taFuncs.size() / std::thread::hardware_concurrency());
    // Remainder functions that need to be allocated to an existing thread
    size_t extraThreadFuncs = taFuncs.size() % std::thread::hardware_concurrency();
    
    const size_t threadSize = funcsPerThread > 0 ? std::thread::hardware_concurrency() : extraThreadFuncs; 
    std::thread threads[threadSize];

    std::vector<std::string> usedFuncs = {};
    std::vector<std::unordered_map<std::string, double*>> dataPointers = copyData(threadSize);
    
    processedMutex.lock();
    
    for (size_t i = 0; i < threadSize; i++) {
        size_t funcs = funcsPerThread;
        if (extraThreadFuncs > 0) {
            extraThreadFuncs--;
            funcs++;
        }
        
        std::vector<TAFunction> threadFuncs = {};
        
        // Iterate through TA function map, building a 
        std::unordered_map<std::string, CalculateIndicator>::iterator it = taFuncs.begin();
        while (it != taFuncs.end() && funcs > 0) {
            if (std::find(usedFuncs.begin(), usedFuncs.end(), it->first) == usedFuncs.end()) {
                // Add ta name and function to a threads function list.
                threadFuncs.push_back({
                    it->first,
                    it->second
                });

                // Track functions already assigned and functions left to fetch for current thread.
                funcs--;
                usedFuncs.push_back(it->first);
            }
            it++;
        }

        threads[i] = std::thread(workerThread, threadFuncs, dataPointers[i], minTicks, blockSize);
    }

    processedMutex.unlock();

    for (auto &thread : threads) {
        thread.join();
    } 

    std::cout << "Block processed.\n";
    
    destroyData(dataPointers);
}