#!/usr/bin/env python

import os

final = ""

files = [
    "prompt.md",
    "headers/raylib.h",
    "headers/raymath.h",
    "src/core.cpp",
    "src/input.cpp",
    "src/raudio.cpp",
    "src/raymath.cpp",
    "src/rcolors.cpp",
    "src/rtext.cpp",
    "src/rtextures.cpp",
    "src/shapes.cpp",
    "Makefile"
    ]

"""

"""

for file in files:
    try:
        with open(os.path.join(".", file), "r", encoding="utf-8") as f:
            final += file + ": \n" + f.read() + "\n\n"
    except Exception as e:
        print(str(e))

with open("source.txt", "w", encoding="utf-8") as f:
    f.write(final.strip())
