#include "backtest_manager.hpp"
#include "ta_processor.hpp"
#include "utils.hpp"

class TestAlgo: public BacktestThread {
    public:
        TestAlgo() : BacktestThread(50000, 14, 10000) {
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
    // TAProcessor taProcessor("./unprocessed", "./processed", 14, 100000);
    // taProcessor.exec();

    BacktestManager *manager;
    manager = manager->getInstance();
    manager->addPath("BTCUSD", "./processed/btcusd.csv");
    manager->createInstances<TestAlgo>();
    manager->exec();
    
    return 0;
}
