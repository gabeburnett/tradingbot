#include "./ta_processor.hpp"
#include <iostream>
#include <ta-lib/ta_func.h>
#include <exception>

std::mutex processedMutex;        
std::unordered_map<std::string, std::array<double, OHLCV_BLOCK_SIZE>> processedMap = {};

double testCalc(const OHLCV *block, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;
    if (TA_RSI(blockIndex, blockIndex, block->close, 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}

void workerThread(std::vector<TAFunction> taFuncs, OHLCV block, int minTicksForAllTA) {
    for (size_t i = minTicksForAllTA; i < OHLCV_BLOCK_SIZE; i++) {
        // Stop loop once initialized values are reached
        if (block.close[i] == 0) break;
        const size_t mapIndex = i - minTicksForAllTA;
        
        for (const TAFunction &ta : taFuncs) {
            double res = ta.func(&block, i);
            processedMutex.lock();
            std::cout << "Result: " << res << " at " << i << " using: " << block.open[i] << "\n";
            processedMap[ta.name][mapIndex] = res; 
            processedMutex.unlock();
        }
    }
}

TAProcessor::TAProcessor(std::string unprocessedPath, std::string processedPath, int minTicksForAllTA) {
    this->unprocessedPath = unprocessedPath;
    this->processedPath = processedPath;
    this->minTicksForAllTA = minTicksForAllTA;

    this->addIndictator("rsi", testCalc);
}

void TAProcessor::addIndictator(std::string taName, CalculateIndicator func) {
    taFuncs[taName] = func;
}

void TAProcessor::prepareArray(double *arr) {
    int startCopyIndex = OHLCV_BLOCK_SIZE - minTicksForAllTA;
    memcpy(arr, arr + startCopyIndex, sizeof(double) * minTicksForAllTA);
    memset(arr + minTicksForAllTA, 0x00, OHLCV_BLOCK_SIZE - minTicksForAllTA);
}

void TAProcessor::prepareStructures() {
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
    int blockIndex = 0;
    while(std::getline(infile, line)) {
        if (blockIndex == OHLCV_BLOCK_SIZE) {
            // Process current block (OHLCV arrays)
            processBlock();
            blockIndex = minTicksForAllTA;

            // Append results to file
            appendProcessedBlock(path);

            // Clear arrays and maps for next block.
            prepareStructures();
            break;
        }

        if (splitNumbers(&arr, line, ',') && arr.size() == 6) {
            open[blockIndex] = std::stod(arr[1]);
            high[blockIndex] = std::stod(arr[2]);
            low[blockIndex] = std::stod(arr[3]);
            close[blockIndex] = std::stod(arr[4]);
            volume[blockIndex] = std::stod(arr[5]);
            blockIndex++;
            std::cout << "loading: " << blockIndex << "\n";
        }
    }
    infile.close();
}

void TAProcessor::clearArrays() {
    memset(open, 0x00, OHLCV_BLOCK_SIZE);
    memset(high, 0x00, OHLCV_BLOCK_SIZE);
    memset(low, 0x00, OHLCV_BLOCK_SIZE);
    memset(close, 0x00, OHLCV_BLOCK_SIZE);
    memset(volume, 0x00, OHLCV_BLOCK_SIZE);
}

void TAProcessor::exec() {
    for (const auto & entry : std::filesystem::directory_iterator("./unprocessed")) {
        parseFile(entry.path());
    }
}

void TAProcessor::processBlock() {
    OHLCV block = {};
    memcpy(block.open, open, sizeof(open));
    memcpy(block.high, high, sizeof(high));
    memcpy(block.low, low, sizeof(low));
    memcpy(block.close, close, sizeof(close));
    memcpy(block.volume, volume, sizeof(volume));

    //TODO: fix this mess
    int threadSize = 1;
    std::thread threads[threadSize];
    for (int i = 0; i < threadSize; i++) {
        threads[i] = std::thread(workerThread, taFuncs[0], block, minTicksForAllTA);
    }

    for (auto &thread : threads) {
        thread.join();
    }
}