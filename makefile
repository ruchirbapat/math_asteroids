.PHONY: all

CC=clang++
FLAGS=-std=c++14 -O2
FILES=main.cpp
OBJ=game
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

build:
	$(CC) $(FLAGS) $(FILES) -o $(OBJ) $(LIBS)

run:
	./$(OBJ)

all: build run
