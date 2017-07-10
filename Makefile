CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -pedantic -Wshadow -Iinclude/
LDFLAGS = -lpthread -lboost_system
LIB = src/ll_protocol.o src/async_raw_server.o
BIN = samples/eth_listener
BIN2 = samples/async_eth_listener

all: $(LIB) $(BIN) $(BIN2)

.c.o:
	$(CXX) -c $(CFLAGS) $< -o $@

$(BIN): $(BIN).o
	$(CXX) -o $(BIN) -O $(BIN).o $(LDFLAGS)

$(BIN2): $(BIN2).o
	$(CXX) -o $(BIN2) -O $(BIN2).o $(LIB) $(LDFLAGS)

doc:
	rm -rf doc/html
	doxygen doc/Doxyfile

clean:
	rm -rf $(BIN) $(BIN2) src/*.o samples/*.o doc/html

.PHONY: doc

