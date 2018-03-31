.PHONY: all

CC=g++
#OPT=-O3
FLAGS=-std=c++17 -g
FILES=Collision.cpp main.cpp
OBJ=game
LIBS=-lsfml-graphics -lsfml-window -lsfml-system -lBox2D

build:
	$(CC) $(FLAGS) $(OPT) $(FILES) -o $(OBJ) $(LIBS)

run:
	./$(OBJ)

backup:
	python backup.py

all: build run
