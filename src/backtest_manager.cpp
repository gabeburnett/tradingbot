#include "./backtest_manager.hpp"
#include <iostream>

BacktestManager::BacktestManager(std::string processedPath) {
    this->processedPath = processedPath;
    std::cout << this->processedPath << "\n";
}