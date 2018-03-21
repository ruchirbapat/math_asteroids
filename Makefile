.PHONY: all

CC=g++
FLAGS=-std=c++17 -O3
FILES=main.cpp
OBJ=game
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

build:
	$(CC) $(FLAGS) $(FILES) -o $(OBJ) $(LIBS)

run:
	./$(OBJ)

backup:
	python backup.py

all: build run
