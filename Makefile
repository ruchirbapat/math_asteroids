.PHONY: all

CC=g++
#OPT=-O3
FLAGS=-std=c++17 -g
FILES=main.cpp Collision.cpp
OBJ=game
LIBS=-lsfml-graphics -lsfml-window -lsfml-system

build:
	$(CC) $(FLAGS) $(OPT) $(FILES) -o $(OBJ) $(LIBS)

run:
	./$(OBJ)

backup:
	python backup.py

all: build run
