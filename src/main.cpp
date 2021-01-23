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
    TAProcessor taProcessor("./unprocessed", "./processed", 14);
    taProcessor.exec();
    return 0;
}

// https://medium.com/heuristics/c-application-development-part-1-project-structure-454b00f9eddc