#include <iostream>
#include "backtest_manager.hpp"
#include "ta_processor.hpp"
#include "utils.hpp"

/*
    TODO:
    - Storage for current positions and balance (and fee)
    - Logging for summary and each buy and sell
    - Queue for backtesting tons of symbols when there isnt enough hardware threads available.
*/

class TestAlgo: public BacktestThread {
    public:
        TestAlgo(std::string tickPath) : BacktestThread(50000, 14, tickPath, 10000) {
            return;
        }

        void riskStrategy(bool buy, bool sell) {
            return;
        }
        
        bool buy() {
            return false;
        }

        bool sell() {
            return false;
        }
};

int main() {
    std::cout << "Running...\n";

    TAProcessor taProcessor("./unprocessed", "./processed", 14, 100000);
    taProcessor.exec();

    // BacktestManager manager(1337);
    // manager.addPath("tsla", "./processed/btcusd.csv");
    // manager.createInstances<TestAlgo>();

    // manager.exec();
    return 0;
}

// https://medium.com/heuristics/c-application-development-part-1-project-structure-454b00f9eddc