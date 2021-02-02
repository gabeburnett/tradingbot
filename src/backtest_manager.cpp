#include "./backtest_manager.hpp"

BacktestThread::BacktestThread(size_t blockSize, size_t minTicks, std::string tickPath, double startBalance) {
    this->blockSize = blockSize;
    this->minTicks = minTicks;
    this->tickPath = tickPath;
    this->tickData = {};
    this->openPositions = {};
    this->balance = startBalance;
}

BacktestThread::~BacktestThread() {
    for (auto it : tickData) {
        free(it.second);
    }
}

void BacktestThread::processHeader(std::unordered_map<size_t, std::string> *columnID, std::string header) {
    std::vector<std::string> names = {};
    split(&names, header, ',', false);
    for (size_t i = 0; i < names.size(); i++) {
        std::string name = names[i];
        (*columnID)[i] = name;
        
        double* column = (double*)calloc(sizeof(double), blockSize);
        tickData[name] = column;
    }
}

void BacktestThread::prepareArrays() {
    for (auto it : tickData) {
        // move and zero array
        int startCopyIndex = blockSize - minTicks;
        memmove(it.second, it.second + startCopyIndex, sizeof(double) * minTicks);
        memset(it.second + minTicks, 0x00, sizeof(double) * (blockSize - minTicks));
    }

    index = minTicks;
}

void BacktestThread::run() {
    std::cout << "Opening: " << tickPath << "\n";
    std::unordered_map<size_t, std::string> columnID = {};
    std::vector<std::string> values = {};
    std::ifstream infile(tickPath);
    std::string line;
    bool gotHeader = false;
    
    while(std::getline(infile, line)) {
        if (!gotHeader) {
            processHeader(&columnID, line);
            gotHeader = true;
            std::cout << "Processed header\n";
            continue;
        }

        if (index == blockSize) {
            prepareArrays();
            std::cout << "Prepared arrays for next block\n";
        }

        if (split(&values, line, ',', true) && values.size() == columnID.size()) {
            for (size_t i = 0; i < values.size(); i++) {
                tickData[columnID[i]][index] = std::stod(values[i]);
            }
            index++;
        }
    }

    infile.close();
}


BacktestManager::BacktestManager(int minTicks) {
    this->minTicks = minTicks;
    tickPaths = {};
}

void BacktestManager::addPath(std::string symbol, std::string path) {
    if (tickPaths.size() == std::thread::hardware_concurrency()) {
        throw std::logic_error("Can't process more symbols than hardware threads simultaneously, due to a significant decrease in performance.");
    }
    tickPaths[symbol] = path;
}

void BacktestManager::exec() {
    if (tickPaths.size() == 0) {
        throw std::logic_error("No tick paths have been added.");
    } else if (tickPaths.size() != threadInstances.size()) {
        throw std::logic_error("Invalid amount of BacktestThread instances.");
    }

    std::thread threads[tickPaths.size()];

    for (size_t i = 0; i < tickPaths.size(); i++) {
        threads[i] = std::thread(&BacktestThread::run, threadInstances.front());
        threadInstances.pop();
    }

    for (auto &thread : threads) {
        thread.join();
    }
}