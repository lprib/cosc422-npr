TARGET = main
CFLAGS = -lOpenMeshCore -lOpenMeshTools -lGL -lGLU -lGLEW -lglut -lm -Wall
OUT = out
SRC = src
SRCS = $(wildcard $(SRC)/*.cpp)
OBJS = $(SRCS:$(SRC)/%.cpp=$(OUT)/%.o)

$(TARGET): $(OBJS)
	g++ -o $@ $(CFLAGS) $^

$(OBJS): $(OUT)/%.o : $(SRC)/%.cpp setup_dirs
	g++ $(CFLAGS) -c $< -o $@

setup_dirs:
	mkdir -p $(OUT)

clean:
	rm -rf out
	rm main

.PHONY: setup_dirs