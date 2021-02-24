#ifndef BACKTEST_MANAGER_H
#define BACKTEST_MANAGER_H

#include <thread>
#include <unordered_map>
#include <queue>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>
#include <filesystem>
#include <vector>
#include "utils.hpp"
#include <mutex>

extern std::mutex LogManagerMutex;

/**
 * Responsible for managing logging from multiple threads via a queue.
 */
class LogManager {
    public:
        /**
         * Get the Singleton instance.
         */
        static LogManager* getInstance() {
            static LogManager instance;
            return &instance;
        }

        /**
         * Sets the log file path, and whether you can to append to an already existing file or overwrite it.
         */
        void setFilePath(std::string path, bool append);

        /**
         * Writes a string to the log.
         * Throws a logic error if no file path has been set.
         * 
         * @param line Important infomation to log.
         */
        void write(std::string str);

        /**
         * Writes the entire queue to the log file.
         */
        void processQueue();
    private:
        /**
         * Contains all the strings waiting to be written to a file.
         */
        std::queue<std::string> logQueue;

        /**
         * Path to the log file.
         */
        std::string logPath;
};


/**
 * Provides basic structure for backtesting an algorithm, including 
 */
class BacktestThread {
    public:
        /**
         * Initializes local variables.
         * 
         * @param blockSize Length of tick data arrays.
         * @param minTicks Minimum ticks needed for the algorithm to operate.
         * @param startBalance Initial starting balanace.
         */ 
        BacktestThread(size_t blockSize, size_t minTicks, double startBalance);

        /** 
         * Deallocates all arrays in tickData.
         */
        ~BacktestThread();
        
        /**
         * Sets the file path containing all the useful tick data which will be feed into the algorithm.
         * 
         * @param tickPath Path to a processed file containing a header and OHLC + TA data.
         */
        void setTickPath(std::string path);

        /**
         * Thread function, reads data from the file, manages each block and 
         * triggers algorithm functions.
         */
        void run();

        /**
         * Algorithm function, handles risk management via
         * openPositions.
         * 
         * @param buy The returned value of the algorithm buy function for the current tick.
         * @param sell The returned value of the algorithm sell function for the current tick.
         */
        virtual void riskStrategy(bool buy, bool sell) = 0;
        
        /**
         * Algorithm function, returns if the algorithm should consider buying a position.
         */
        virtual bool buy() = 0;

        /**
         * Algorithm function, returns if the algorithm should consider selling a position.
         */
        virtual bool sell() = 0;
    
    private:
        /**
         * Minimum ticks needed for the algorithm to function.
         */
        size_t minTicks;

        /**
         * The path to a processed file, that must have a header.
         */
        std::string tickPath;

        /**
         * Parse header line, assumes its in a CSV like format with , as delimiter.
         * Initializes arrays for every column with a length of blockSize.
         * 
         * @param columnID An address of a map that will be used to store index to string name mappings
         * of each column.
         * @param header The header line, usually the first line in a csv file.
         */
        void processHeader(std::unordered_map<size_t, std::string> *columnID, std::string header);

        /**
         * Copies the minimum needed ticks for operation, from the end to the beginning of the array,
         * and zeros every other index. This applies to every array in tickData.
         * Also updates the index variable.
         */
        void prepareArrays();

    protected:
        /**
         * Contains all the OHLC and TA arrays.
         * The key is the column name from the header from the given file.
         * Every array has a length of blockSize.
         */
        std::unordered_map<std::string, double*> tickData;

        /**
         * Available balance for buying positions.
         */
        double balance;

        /**
         * The length of arrays used to store tick data, this must be more than minTicks.
         * For large datasets, its recommended to use a large blockSize, such as 10-100k or more.
         */
        size_t blockSize;

        /**
         * The index of the latest tick for all arrays in tickData.
         */
        size_t index;
};

/**
 * Responsible for creating and assigning threads to symbols, for algorithmic processing.
 */
class BacktestManager {
    public:
        /**
         * Get the Singleton instance.
         */
        static BacktestManager* getInstance() {
            static BacktestManager instance;
            return &instance;
        }

        /**
         * Add a symbol with a path to the file created by TAProcessor.
         */
        void addPath(std::string symbol, std::string path);

        /**
         * Creates an instance of your algorithm class, which should be a child of the BacktestThread class.
         */
        template <class T> void createInstances() {
            threadInstances = {};
            for (auto it : tickPaths) {
                BacktestThread* instance = new T();
                instance->setTickPath(it.second);
                threadInstances.push(instance);
            }
        }

        /**
         * Assigns an instance of the algorithm class to a thread, then waits for all threads to finish computation.
         * 
         * Note: The createInstances function, must be ran exactly once, before this function.
         */
        void exec();
    private:
        /**
         * A queue containing all algorithm class instances
         */
        std::queue<BacktestThread*> threadInstances;

        /**
         * A map containing symbol names and a path to their processed file.
         */
        std::unordered_map<std::string, std::string> tickPaths;


        /**
         * Initializes local variables.
         */
        BacktestManager();
};

#endif