# TradingBot

A trading bot with a focus on backtesting and the generation of technical analysis indicators, while utilizing all hardware threads where possible.

<br/>

## Backtesting

After you have ran the TAProcessor and generated clean files, you can start working on your trading algorithm.
- Don't forget to check out the backtest_manager header file to view all the variables and functions you have access to.

<br/>

Creating your algorithm:
- Your algorithm will be a child of the BacktestThread class, which will run on its own thread, so the **class must be thread-safe.**
- Call the super class to setup the block size of data, minimum ticks needed for the algorithm to operate, pass the file path and the starting balance of the algorithm.
- The buy and sell boolean functions act as buy/sell indicators for the riskStrategy function. **The buy, sell and riskStrategy functions must be implemented.**
```
class TestAlgo: public BacktestThread {
    public:
        TestAlgo(std::string tickPath) : BacktestThread(50000, 14, tickPath, 10000) {
            // Setup your algorithm's data structures
        }

        void riskStrategy(bool buy, bool sell) {
            COMPLETE ME
        }
        
        bool buy() {
            return tickData["rsi"][index] < 40;
        }

        bool sell() {
            return tickData["rsi"][index] > 55;
        }
};
```

<br/>

Running your algorithm:
1. Access backtest manager class.
2. Add symbols and their processed file paths.
3. Create instances of your algorithm class.
4. Start backtesting.

```
int main() {
    BacktestManager COMPLETE ME;
    manager.addPath("tsla", "./processed/btcusd.csv");
    manager.createInstances<TestAlgo>();
    manager.exec();

    return 0;
}
```

<br/>

## Indicator generation

To generate a indicator, you need to create a function using the following format:

`double functionName(std::unordered_map<std::string, double*> data, size_t blockIndex)`
- This **function must be thread-safe**, as the entire process of generating indicators is multithreaded.
- blockIndex is the current index of the arrays in the data map.
- The data map corresponds to the data of each column from the file being processed along with its column name as the key.
- The returned result must be the indicator value for blockIndex.
- This function assumes you **don't access any indexes after the given blockIndex** on all of the data arrays.

<br/>

In this example I'm using a third-party library called TA-Lib to help generate the RSI indicator.
```
double rsiIndicator(std::unordered_map<std::string, double*> data, size_t blockIndex) {    
    double outReal = 0;
    int outBegIdx = 0;
    int outNBElement = 0;

    if (TA_RSI(blockIndex, blockIndex, std::as_const(data["close"]), 14, &outBegIdx, &outNBElement, &outReal) != TA_SUCCESS) {
        throw std::logic_error("Failed to calculate indicator: RSI.");
    }
    return outReal;
}
```

<br/>

To start processing, simply create the TAProcessor object, with the corresponding  directory paths, minimum ticks and block size values.
- The unprocessed directory must contain files with a comma as the delimiter, and usually OHLC values (specific values don't matter, as long as they support being stored as a double).
- The processed directory is where the processed files will be stored.
- Minimum ticks, is just the minimum amount of time-steps/ticks needed to generate ALL of your indicators, so you don't have any empty values in the processed file.
- The block size is to help with the processing of very large files, it's recommended to set this between 10-100k or higher.
- The exec function will begin processing every file in the unprocessed directory.

```
int main() {
    TAProcessor taProcessor("./unprocessed", "./processed", 14, 10000);
    taProcessor.addIndictator("rsi", rsiIndicator);
    taProcessor.exec();

    return 0;
}
```