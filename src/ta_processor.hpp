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

struct OHLCV {
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
    std::vector<double> volume;
};

typedef struct OHLCV OHLCV;

typedef double (*CalculateIndicator) (OHLCV *block, size_t blockIndex);

class TAProcessor {
    public:
        TAProcessor(std::string unprocessedPath, std::string processedPath);
        void addIndictator(CalculateIndicator func);
        void processBlock();
        void parseFile(std::string path);
        void exec();
    
    private:
        std::vector<double> open;
        std::vector<double> high;
        std::vector<double> low;
        std::vector<double> close;
        std::vector<double> volume;

        std::vector<CalculateIndicator> taFuncs;
        std::string unprocessedPath;
        std::string processedPath;
};

#endif