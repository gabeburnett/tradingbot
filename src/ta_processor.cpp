#include "./ta_processor.hpp"
#include <iostream>
#include <ta-lib/ta_func.h>
#include <exception>

std::mutex processedMutex;        
std::unordered_map<std::string, std::vector<double>> processedMap;

double testCalc(OHLCV *block, size_t index) {
    int startIndex = index - 14;
    // if (startIndex < 0) return 0; // Don't store values that are -1, we only want to store ticks with full OHLC and TA data.
    // const double* close = &block->close[0];
    const double close[16] = {0};
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0; 
    std::cout << "start: " << startIndex << " end: " << index << " real len: " << block->close.size() << "\n";
    if (TA_RSI(0, 14, close, TA_INTEGER_DEFAULT, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    // } this causes stack smashing, maybe close array is casuing it
    // for (int i = 0; i < )
    return 133337;
}

void workerThread(CalculateIndicator calc, OHLCV block) {
    for (size_t i = 0; i < block.open.size(); i++) {
        double result = calc(&block, i);
        processedMutex.lock();
        std::cout << "Result: " << result << " at " << i << "\n";
        processedMutex.unlock();
    }
}

TAProcessor::TAProcessor(std::string unprocessedPath, std::string processedPath) {
    this->unprocessedPath = unprocessedPath;
    this->processedPath = processedPath;
    this->addIndictator(testCalc);
}

void TAProcessor::addIndictator(CalculateIndicator func) {
    this->taFuncs.push_back(func);
}

void TAProcessor::parseFile(std::string path) {
    std::ifstream infile(path);
    std::string line;
    std::vector<std::string> arr = {};
    int a = 0;
    while(std::getline(infile, line)) {
        a++;
        if (splitNumbers(&arr, line, ',') && arr.size() == 6) {
            open.push_back(std::stof(arr[1]));
            high.push_back(std::stof(arr[2]));
            low.push_back(std::stof(arr[3]));
            close.push_back(std::stof(arr[4]));
            volume.push_back(std::stof(arr[5]));
        }
        std::cout << "len: " << open.size() << "\n";
        if (a > 100) break;
    }
    infile.close();
}

void TAProcessor::exec() {
    for (const auto & entry : std::filesystem::directory_iterator("./unprocessed")) {
        open.clear();
        high.clear();
        low.clear();
        close.clear();
        volume.clear();
        parseFile(entry.path());
        processBlock();
    }
}

void TAProcessor::processBlock() {
    int threadSize = 1;
    std::thread threads[threadSize];
    for (int i = 0; i < threadSize; i++) {
        OHLCV block = { open, high, low, close, volume };
        threads[i] = std::thread(workerThread, taFuncs[0], block);
    }

    for (auto &thread : threads) {
        thread.join();
    }
}