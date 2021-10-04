TARGET = main
CFLAGS = -lOpenMeshCore -lOpenMeshTools -lGL -lGLU -lGLEW -lglut -lm -Wall
OUT = out
SRC = src
SRCS = $(wildcard $(SRC)/*.cpp)
OBJS = $(SRCS:$(SRC)/%.cpp=$(OUT)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ -o $@ $(CFLAGS) $^

$(OUT)/%.o : $(SRC)/%.cpp | $(OUT)
	g++ $(CFLAGS) -c $< -o $@

$(OUT):
	mkdir $(OUT)

clean:
	rm -rf out
	rm main

.DEFAULT: all
.PHONY: all