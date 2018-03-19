.PHONY: all

CC=g++
FLAGS=-std=c++17 -O2
FILES=main.cpp
OBJ=game
LIBS=-g -lsfml-graphics -lsfml-window -lsfml-system

build:
	$(CC) $(FLAGS) $(FILES) -o $(OBJ) $(LIBS)

run:
	./$(OBJ)

all: build run
