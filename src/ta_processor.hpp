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
    double *open;
    double *high;
    double *low;
    double *close;
    double *volume;
    size_t maxTicks;
};

typedef struct OHLCV OHLCV;

typedef double (*CalculateIndicator) (const OHLCV *block, size_t blockIndex);

struct TAFunction {
    std::string name;
    CalculateIndicator func;
};

typedef struct TAFunction TAFunction;

class TAProcessor {
    public:
        TAProcessor(std::string unprocessedPath, std::string processedPath, int minTicksForAllTA, size_t maxTicks);
        ~TAProcessor();
        void addIndictator(std::string taName, CalculateIndicator func);
        void exec();
    
    private:
        int minTicksForAllTA;
        double *open;
        double *high;
        double *low;
        double *close;
        double *volume;
        size_t maxTicks;


        std::unordered_map<std::string, CalculateIndicator> taFuncs;
        std::string unprocessedPath;
        std::string processedPath;

        std::vector<OHLCV*> copyData(size_t copies);
        void destroyData(std::vector<OHLCV*> pointers);
        void prepareArray(double *arr);
        void processBlock();
        void parseFile(std::string path);
        void clearArrays();
        void prepareNextBlock();
        void appendProcessedBlock(std::string path);
};

#endif