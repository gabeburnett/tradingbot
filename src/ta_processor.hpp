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

/**
 * Mutex for controlling access to processData
 */
extern std::mutex processedMutex;

/**
 * Map for storing processed data, (TA calculations).
 */
extern std::unordered_map<std::string, double*> processedData;

/**
 * TA function definition, used for calculating one TA signal for the tick at blockIndex.
 * 
 * @param data Map containing at least minTicks of data from an unprocessed file.
 * @param blockIndex The index that the indicator must be calculated at.
 * @returns The result from calculating the indicator.
 */ 
typedef double (*CalculateIndicator) (std::unordered_map<std::string, double*> data, size_t blockIndex);

/**
 * Contains the name of a TA indicator and the function to calculate it (per tick).
 */
struct TAFunction {
    std::string name;
    CalculateIndicator func;
};
typedef struct TAFunction TAFunction;

/**
 * Reponsible for managing the calculation of TA signals for all files in a given directory.
 * Using double arrays for better capability with known libraries such as TA-Lib.
 */
class TAProcessor {
    public:
        /**
         * Initializes local variables
         * 
         * @param unprocessedPath Path to directory containing unprocessed symbol files.
         * @param processedPath Path to directory where processed symbol files will be stored.
         * @param minTicks Minimum ticks needed for all TA functions to operate.
         * @param blockSize Length of all TA processor arrays, aka the length of each chunk of data.
         */
        TAProcessor(std::string unprocessedPath, std::string processedPath, size_t minTicks, size_t blockSize);
        
        /**
         * Deallocate any tickData or processedData arrays.
         */
        ~TAProcessor();
        
        /**
         * Adds a TA function and it's name, as well as allocating it's processedData array.
         */
        void addIndictator(std::string taName, CalculateIndicator func);
        
        /**
         * Starts processing every file in the unprocessedPath.
         */ 
        void exec();
    
    private:
        /**
         * Minimum ticks needed for all TAFunctions to operate properly.
         */
        size_t minTicks;

        /**
         * Length of all TA processor arrays, aka the length of each chunk of data.
         */ 
        size_t blockSize;

        /**
         * Stores current block of data from unprocessed file.
         */ 
        std::unordered_map<std::string, double*> tickData;

        /**
         * Stores the TA functions.
         */
        std::unordered_map<std::string, CalculateIndicator> taFuncs;

        /**
         * Path to directory containing unprocessed symbol files.
         */
        std::string unprocessedPath;

        /**
         * Path to directory where processed symbol files will be stored.
         */
        std::string processedPath;

        /**
         * Parse header line, assumes its in a CSV like format with , as delimiter.
         * Initializes arrays for every column with a length of blockSize.
         * 
         * @param columnID An address of a map that will be used to store index to string name mappings
         * of each column.
         * @param header The header line, usually the first line in a csv file.
         */
        void processHeader(std::unordered_map<size_t, std::string> *columnID, std::string header);

        std::vector<std::unordered_map<std::string, double*>> copyData(size_t copies);
        void destroyData(std::vector<std::unordered_map<std::string, double*>> copies);
        void processBlock();
        void parseFile(std::string path);
        void prepareNextBlock();
        void appendHeader(std::string path);
        void appendProcessedBlock(std::string path);
};

#endif