#include "./ta_processor.hpp"
#include <iostream>
#include <ta-lib/ta_func.h>
#include <exception>
#include <math.h>
#include <atomic>
#include <algorithm>
#include <fstream>
#include <cstdio>

std::mutex processedMutex;
std::unordered_map<std::string, DynDoubleArray> processedMap = {};

double testCalc(const OHLCV *block, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;
    if (TA_RSI(blockIndex, blockIndex, block->close, 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}

double atestCalc(const OHLCV *block, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;
    if (TA_RSI(blockIndex, blockIndex, block->close, 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}

double btestCalc(const OHLCV *block, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;
    if (TA_RSI(blockIndex, blockIndex, block->close, 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}

double ctestCalc(const OHLCV *block, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;
    if (TA_RSI(blockIndex, blockIndex, block->close, 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}

double dtestCalc(const OHLCV *block, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;
    if (TA_RSI(blockIndex, blockIndex, block->close, 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}

void workerThread(std::vector<TAFunction> taFuncs, OHLCV *block, int minTicksForAllTA) {
    for (size_t i = minTicksForAllTA; i < block->maxTicks; i++) {
        // Stop loop once initialized values are reached
        if (block->open[i] == 0 || i >= block->maxTicks) break;
        for (const TAFunction &ta : taFuncs) {
            double res = ta.func(block, i);      

            // processedMutex.lock();
            // std::cout << "attempting \n";
            // std::cout << "res: " << res << std::endl;
            processedMap[ta.name].array[i] = res;

            // std::cout << "res: " << res << " ADDED" << std::endl;
            
            // processedMutex.unlock();
        }
    }
}

TAProcessor::TAProcessor(std::string unprocessedPath, std::string processedPath, int minTicksForAllTA, size_t maxTicks) {
    processedMap = {};
    this->open = (double*)calloc(sizeof(double), maxTicks);
    this->high = (double*)calloc(sizeof(double), maxTicks);
    this->low = (double*)calloc(sizeof(double), maxTicks);
    this->close = (double*)calloc(sizeof(double), maxTicks);
    this->volume = (double*)calloc(sizeof(double), maxTicks);
    this->maxTicks = maxTicks;

    this->unprocessedPath = unprocessedPath;
    this->processedPath = processedPath;
    this->minTicksForAllTA = minTicksForAllTA;

    this->addIndictator("rsi", testCalc);
    // this->addIndictator("a", atestCalc);
    // this->addIndictator("b", btestCalc);
    // this->addIndictator("c", ctestCalc);
    // this->addIndictator("d", dtestCalc);
    // this->addIndictator("e", testCalc);
    // this->addIndictator("f", testCalc);
    // this->addIndictator("g", testCalc);
    // this->addIndictator("h", testCalc);
    // this->addIndictator("i", testCalc);
}

void TAProcessor::addIndictator(std::string taName, CalculateIndicator func) {
    taFuncs[taName] = func;
}

void TAProcessor::prepareArray(double *arr) {
    int startCopyIndex = maxTicks - minTicksForAllTA;
    memcpy(arr, arr + startCopyIndex, sizeof(double) * minTicksForAllTA);
    memset(arr + minTicksForAllTA, 0x00, sizeof(double) * (maxTicks - minTicksForAllTA));
}

void TAProcessor::prepareNextBlock() {
    prepareArray(open);
    prepareArray(high);
    prepareArray(low);
    prepareArray(close);
    prepareArray(volume);

    for (auto it : processedMap) {
        memset(it.second.array, 0x00, it.second.length);
    }

    processedMap.clear();
}

void appendLineToFile(std::string filepath, std::string line)
{
    std::ofstream file;
    //can't enable exception now because of gcc bug that raises ios_base::failure with useless message
    //file.exceptions(file.exceptions() | std::ios::failbit);
    file.open(filepath, std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));

    //make sure write fails with exception if something is wrong
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file << line << std::endl;
}

void TAProcessor::appendHeader(std::string path) {
    std::ofstream file;
    std::stringstream ss;

    file.open(path, std::ios::out | std::ios::app);
    if (file.fail()) throw std::ios_base::failure(std::strerror(errno));    

    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    ss << "open,high,low,close,volume";

    for (auto it : processedMap) {
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

    for (size_t tick = minTicksForAllTA; tick < maxTicks; tick++) {
        ss << open[tick] << "," << high[tick] << "," << low[tick] << "," << close[tick] << "," << volume[tick];
        
        for (auto it : processedMap) {
            if (it.second.length < tick) continue;
            ss << "," << it.second.array[tick];
        }

        file << ss.str() << "\r\n";

        ss.str(std::string());
    }

    file.close();
}

void TAProcessor::parseFile(std::string path) {
    //TODO: Delete processed file if exitsts
    std::string processedPath = std::string("./processed/") + std::string(std::filesystem::path(path).filename());
    std::remove(processedPath.c_str());
    std::filesystem::create_directory("./processed");
    std::ifstream infile(path);
    std::string line;
    std::vector<std::string> arr = {};
    size_t blockIndex = 0;
    bool hasHeader = false;
    while(std::getline(infile, line)) {
        if (blockIndex == maxTicks) {
            // Process current block (OHLCV arrays)
            processBlock();
            blockIndex = minTicksForAllTA;
            
            // Add header with OHCLV and TA names in the correct order.
            if (!hasHeader) {
                appendHeader(processedPath);
                hasHeader = true;
            }
            // Append results to file
            appendProcessedBlock(processedPath);

            // Clear arrays and maps for next block.
            prepareNextBlock();
            continue;
        }

        if (split(&arr, line, ',', true) && arr.size() == 6) {
            open[blockIndex] = std::stod(arr[1]);
            high[blockIndex] = std::stod(arr[2]);
            low[blockIndex] = std::stod(arr[3]);
            close[blockIndex] = std::stod(arr[4]);
            volume[blockIndex] = std::stod(arr[5]);
            blockIndex++;
            // std::cout << "loading: " << blockIndex << "\n";
        }
    }
    infile.close();
}

void TAProcessor::clearArrays() {
    memset(open, 0x00, sizeof(double) * maxTicks);
    memset(high, 0x00, sizeof(double) * maxTicks);
    memset(low, 0x00, sizeof(double) * maxTicks);
    memset(close, 0x00, sizeof(double) * maxTicks);
    memset(volume, 0x00, sizeof(double) * maxTicks);
}

void TAProcessor::exec() {
    for (const auto & entry : std::filesystem::directory_iterator("./unprocessed")) {
        parseFile(entry.path());
    }
}

std::vector<OHLCV*> TAProcessor::copyData(size_t copies) {
    std::vector<OHLCV*> pointers = {};
    for (size_t i = 0; i < copies; i++) {
        OHLCV* newPointer = (OHLCV*)malloc(sizeof(OHLCV));
        newPointer->open = (double*)malloc(sizeof(double) * maxTicks);
        newPointer->high = (double*)malloc(sizeof(double) * maxTicks);
        newPointer->low = (double*)malloc(sizeof(double) * maxTicks);
        newPointer->close = (double*)malloc(sizeof(double) * maxTicks);
        newPointer->volume = (double*)malloc(sizeof(double) * maxTicks);
        
        memcpy(newPointer->open, open, sizeof(double) * maxTicks);
        memcpy(newPointer->high, high, sizeof(double) * maxTicks);
        memcpy(newPointer->low, low, sizeof(double) * maxTicks);
        memcpy(newPointer->close, close, sizeof(double) * maxTicks);
        memcpy(newPointer->volume, volume, sizeof(double) * maxTicks);
        
        pointers.push_back(newPointer);
    }
    return pointers;
}

void TAProcessor::destroyData(std::vector<OHLCV*> pointers) {
    for (auto pointer : pointers) {
        free(pointer->open);
        free(pointer->high);
        free(pointer->low);
        free(pointer->close);
        free(pointer->volume);
        free(pointer);
    }

    // for (auto it : processedMap) {
    //     free(it.second.array);
    // }

    // processedMap.clear();
}

TAProcessor::~TAProcessor() {
    free(this->open);
    free(this->high);
    free(this->low);
    free(this->close);
    free(this->volume);
}

void TAProcessor::processBlock() {
    const size_t funcsPerThread = floor(taFuncs.size() / std::thread::hardware_concurrency());
    size_t extraThreadFuncs = taFuncs.size() % std::thread::hardware_concurrency();
    const size_t threadSize = funcsPerThread > 0 ? std::thread::hardware_concurrency() : extraThreadFuncs; 
    std::thread threads[threadSize];

    std::vector<std::string> usedFuncs = {};
    std::vector<OHLCV*> dataPointers = copyData(threadSize);
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
                
                // Init TA result array
                double *results = (double*)calloc(sizeof(double), maxTicks);
     
                processedMap[it->first] = {
                    results,
                    maxTicks
                };

                // processedMap[it->first].array[0] = 420;

                // Track functions already assigned and functions left to fetch for current thread.
                funcs--;
                usedFuncs.push_back(it->first);
            }
            it++;
        }

        std::cout << "gen len: " << processedMap.size() << "\n";
        std::cout << "starting new thread with funcs: " << threadFuncs.size() << "\n";
        threads[i] = std::thread(workerThread, threadFuncs, dataPointers[i], minTicksForAllTA);
    }

    for (auto &thread : threads) {
        thread.join();
    } 

    std::cout << "Block processed.\n";
    
    destroyData(dataPointers);
}