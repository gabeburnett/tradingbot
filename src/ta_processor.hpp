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

struct DynDoubleArray {
    double *array;
    size_t length;
};

typedef struct DynDoubleArray DynDoubleArray;

extern std::mutex processedMutex;
extern std::unordered_map<std::string, double*> processedData;

typedef double (*CalculateIndicator) (std::unordered_map<std::string, double*> data, size_t blockIndex);

struct TAFunction {
    std::string name;
    CalculateIndicator func;
};

typedef struct TAFunction TAFunction;

class TAProcessor {
    public:
        TAProcessor(std::string unprocessedPath, std::string processedPath, size_t minTicks, size_t blockSize);
        ~TAProcessor();
        void addIndictator(std::string taName, CalculateIndicator func);
        void exec();
    
    private:
        size_t minTicks;
        size_t blockSize;

        /**
         * Stores current block of tickData and processed data.
         */ 
        std::unordered_map<std::string, double*> tickData;
        std::unordered_map<std::string, CalculateIndicator> taFuncs;
        std::string unprocessedPath;
        std::string processedPath;

        std::vector<std::unordered_map<std::string, double*>> copyData(size_t copies);
        void destroyData(std::vector<std::unordered_map<std::string, double*>> copies);
        void processBlock();
        void parseFile(std::string path);
        void prepareNextBlock();
        
        /**
         * Parse header line, assumes its in a CSV like format with , as delimiter.
         * Initializes arrays for every column with a length of blockSize.
         * 
         * @param columnID An address of a map that will be used to store index to string name mappings
         * of each column.
         * @param header The header line, usually the first line in a csv file.
         */
        void processHeader(std::unordered_map<size_t, std::string> *columnID, std::string header);
        void appendHeader(std::string path);
        void appendProcessedBlock(std::string path);
};

#endif