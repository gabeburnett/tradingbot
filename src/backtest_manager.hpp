#ifndef BACKTEST_MANAGER_H
#define BACKTEST_MANAGER_H

#include <thread>
#include <unordered_map>
#include <queue>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>
#include <vector>
#include "utils.hpp"

class BacktestThread {
    public:
        /**
         * Initializes local variables.
         * 
         * @param blockSize Length of tick data arrays.
         * @param minTicks Minimum ticks needed for the algorithm to operate.
         * @param tickPath Path to a processed file containing a header and OHLC + TA data.
         * @param startBalance Initial starting balanace.
         */ 
        BacktestThread(size_t blockSize, size_t minTicks, std::string tickPath, double startBalance);

        /** 
         * Deallocates all arrays in tickData.
         */
        ~BacktestThread();
        
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
         * The length of arrays used to store tick data, this must be more than minTicks.
         * For large datasets, its recommended to use a large blockSize, such as 10-100k or more.
         */
        size_t blockSize;

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
         * Contains current open positions, 
         * key = buy-in price and value = quantity brought.
         */
        std::unordered_map<double, double> openPositions;

        /**
         * Available balance for buying positions.
         */
        double balance;

        /**
         * The index of the latest tick for all arrays in tickData.
         */
        size_t index;

        
};

class BacktestManager {
    public:
        BacktestManager(int minTicks);
        void addPath(std::string symbol, std::string path);

        template <class T> void createInstances() {
            threadInstances = {};
            for (auto it : tickPaths) {
                BacktestThread* instance = new T(it.second);
                threadInstances.push(instance);
            }
        }

        void exec();
    private:
        int minTicks;
        std::queue<BacktestThread*> threadInstances;
        std::unordered_map<std::string, std::string> tickPaths;
};

#endif