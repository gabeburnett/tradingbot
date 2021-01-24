#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ta-lib/ta_func.h>
#include "backtest_manager.hpp"
#include "ta_processor.hpp"
#include "utils.hpp"

int main() {
    std::cout << "Running...\n";
    // TAProcessor taProcessor("./unprocessed", "./processed", 14);
    // taProcessor.exec();

    std::unordered_map<std::string, std::array<double, OHLCV_BLOCK_SIZE>> asd = {};
    asd["rsi"] = {};
    asd["rsi"][420] = 1337;
    std::cout << asd["rsi"].size() << "\n";
    std::cout << asd["rsi"][420] << "\n";
    return 0;//
}

// https://medium.com/heuristics/c-application-development-part-1-project-structure-454b00f9eddc