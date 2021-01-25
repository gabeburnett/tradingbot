#include "./ta_processor.hpp"
#include <iostream>
#include <ta-lib/ta_func.h>
#include <exception>
#include <math.h>
#include <atomic>
#include <algorithm>

std::mutex processedMutex;        
std::unordered_map<std::string, std::vector<double>> processedMap = {};
double testing = 0;

double testCalc(const OHLCV *block, size_t blockIndex) {    
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
        if (block->close[i] == 0) break;

        for (const TAFunction &ta : taFuncs) {
            double res = ta.func(block, i);

            processedMutex.lock();
            // std::cout << ta.name << ": " << res << "\n";
            processedMap[ta.name].push_back(res);//seg fault YYYYY
            testing = res;//works
            processedMutex.unlock();
        }
    }
}

TAProcessor::TAProcessor(std::string unprocessedPath, std::string processedPath, int minTicksForAllTA, size_t maxTicks) {
    this->open = (double*)calloc(sizeof(double), maxTicks);
    this->high = (double*)calloc(sizeof(double), maxTicks);
    this->low = (double*)calloc(sizeof(double), maxTicks);
    this->close = (double*)calloc(sizeof(double), maxTicks);
    this->volume = (double*)calloc(sizeof(double), maxTicks);
    this->maxTicks = maxTicks;

    this->unprocessedPath = unprocessedPath;
    this->processedPath = processedPath;
    this->minTicksForAllTA = minTicksForAllTA;

    this->addIndictator("z", testCalc);
    this->addIndictator("a", testCalc);
    this->addIndictator("b", testCalc);
    this->addIndictator("c", testCalc);
    this->addIndictator("d", testCalc);
    this->addIndictator("e", testCalc);
    this->addIndictator("f", testCalc);
    this->addIndictator("g", testCalc);
    this->addIndictator("h", testCalc);
    this->addIndictator("i", testCalc);
}

void TAProcessor::addIndictator(std::string taName, CalculateIndicator func) {
    taFuncs[taName] = func;
}

void TAProcessor::prepareArray(double *arr) {
    int startCopyIndex = maxTicks - minTicksForAllTA;
    memcpy(arr, arr + startCopyIndex, sizeof(double) * minTicksForAllTA);
    memset(arr + minTicksForAllTA, 0x00, maxTicks - minTicksForAllTA);
}

void TAProcessor::prepareNextBlock() {
    prepareArray(open);
    prepareArray(high);
    prepareArray(low);
    prepareArray(close);
    prepareArray(volume);
    processedMap.clear();
}

void TAProcessor::appendProcessedBlock(std::string path) {}

void TAProcessor::parseFile(std::string path) {
    //TODO: Delete processed file if exitsts
    std::ifstream infile(path);
    std::string line;
    std::vector<std::string> arr = {};
    size_t blockIndex = 0;
    while(std::getline(infile, line)) {
        if (blockIndex == maxTicks) {
            // Process current block (OHLCV arrays)
            processBlock();
            blockIndex = minTicksForAllTA;

            // Append results to file
            // appendProcessedBlock(path);

            // Clear arrays and maps for next block.
            prepareNextBlock();
            continue;
        }

        if (splitNumbers(&arr, line, ',') && arr.size() == 6) {
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
    memset(open, 0x00, maxTicks);
    memset(high, 0x00, maxTicks);
    memset(low, 0x00, maxTicks);
    memset(close, 0x00, maxTicks);
    memset(volume, 0x00, maxTicks);
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
        
        memcpy(newPointer->open, open, maxTicks);
        memcpy(newPointer->high, high, maxTicks);
        memcpy(newPointer->low, low, maxTicks);
        memcpy(newPointer->close, close, maxTicks);
        memcpy(newPointer->volume, volume, maxTicks);
        
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

                // std::cout << "assigning: " << it->first << " to thread: " << i << "\n";
                
                // Init TA result array
                processedMap[it->first] = {};

                // Track functions already assigned and functions left to fetch for current thread.
                funcs--;
                usedFuncs.push_back(it->first);
            }
            it++;
        }
        // std::cout << "starting new thread with funcs: " << threadFuncs.size() << "\n";
        threads[i] = std::thread(workerThread, threadFuncs, dataPointers[i], minTicksForAllTA);
    }

    for (auto &thread : threads) {
        thread.join();
    } 

    std::cout << "Block processed.\n";
    
    destroyData(dataPointers);
}