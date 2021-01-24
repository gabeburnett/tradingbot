#ifndef TA_PROCESSOR_H
#define TA_PROCESSOR_H

#include <vector>
#include <thread>
#include <string>
#include <fstream>
#include <unordered_map>
#include <mutex>
#include <filesystem>
#include "utils.hpp"

#define OHLCV_BLOCK_SIZE 1000

struct TAFunction {
    std::string name;
    CalculateIndicator func;
};

typedef struct TAFunction TAFunction;

struct OHLCV {
    double open[OHLCV_BLOCK_SIZE] = {0};
    double high[OHLCV_BLOCK_SIZE] = {0};
    double low[OHLCV_BLOCK_SIZE] = {0};
    double close[OHLCV_BLOCK_SIZE] = {0};
    double volume[OHLCV_BLOCK_SIZE] = {0};
};

typedef struct OHLCV OHLCV;

typedef double (*CalculateIndicator) (const OHLCV *block, size_t blockIndex);

class TAProcessor {
    public:
        TAProcessor(std::string unprocessedPath, std::string processedPath, int minTicksForAllTA);
        void addIndictator(std::string taName, CalculateIndicator func);
        void exec();
    
    private:
        int minTicksForAllTA;
        double open[OHLCV_BLOCK_SIZE] = {0};
        double high[OHLCV_BLOCK_SIZE] = {0};
        double low[OHLCV_BLOCK_SIZE] = {0};
        double close[OHLCV_BLOCK_SIZE] = {0};
        double volume[OHLCV_BLOCK_SIZE] = {0};

        std::unordered_map<std::string, CalculateIndicator> taFuncs;
        std::string unprocessedPath;
        std::string processedPath;

        void prepareArray(double *arr);
        void processBlock();
        void parseFile(std::string path);
        void clearArrays();
        void prepareStructures();
        void appendProcessedBlock(std::string path);
};

#endif