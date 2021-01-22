g++-8 -std=c++17 \
src/main.cpp \
src/backtest_manager.cpp \
src/backtest_manager.hpp \
src/ta_processor.cpp \
src/ta_processor.hpp \
src/utils.cpp \
src/utils.hpp \
-lpthread -lstdc++fs -o program \
-Wall \
-L/usr/local/lib/ \
-lta_lib \
-I/usr/local/include/ \
&& ./program
