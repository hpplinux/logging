CC=g++
CC_FLAGS=-O2 -Wall -std=c++11
TARGET=test_logging
SOURCES=main.cpp logging.cpp
HEADERS=logging.h

main: $(SOURCES) $(HEADERS)
	$(CC) $(CC_FLAGS) -o $(TARGET) $(SOURCES)
