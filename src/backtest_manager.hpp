#ifndef BACKTEST_MANAGER_H
#define BACKTEST_MANAGER_H

#include <string>

class BacktestManager {
    public:
        BacktestManager(std::string processedPath);
    private:
        std::string processedPath;
};

#endif