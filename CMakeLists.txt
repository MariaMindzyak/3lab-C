cmake_minimum_required(VERSION 3.25)
project(lab3 C)

set(CMAKE_C_STANDARD 99)

add_executable(lab3 main.c table.c table.h dialog.c dialog.h menu.h dialog.c fileHelper.c)
