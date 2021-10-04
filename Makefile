MAKEFLAGS += -j4

TARGET = main
CFLAGS = -lOpenMeshCore -lOpenMeshTools -lGL -lGLU -lGLEW -lglut -lm -Wall -Og
OUT = out
SRC = src
SRCS = $(wildcard $(SRC)/*.cpp)
OBJS = $(SRCS:$(SRC)/%.cpp=$(OUT)/%.o)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	g++ -o $@ $(CFLAGS) $^

$(OUT)/%.o : $(SRC)/%.cpp | $(OUT)
	g++ $(CFLAGS) -c $< -o $@

$(OUT):
	mkdir $(OUT)

clean:
	rm -rf out
	rm main

.DEFAULT: run
.PHONY: run