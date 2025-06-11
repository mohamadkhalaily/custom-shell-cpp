# Smash Shell

A lightweight Unix-style shell implemented in C++, designed as part of an academic operating systems course.

## Features

- Built-in commands (`cd`, `pwd`, `showpid`, `chprompt`, `jobs`, `fg`, `kill`, `quit`, `chmod`)
- External command execution (foreground & background using `&`)
- Signal handling (e.g., `SIGINT`, `SIGTSTP`, `SIGALRM`)
- Jobs list management (`jobs`, `fg`, `kill`, `quit kill`)
- I/O redirection support (`>`, `>>`) 
- Pipe support (`|`, `|&`) 

## File Structure

- `Commands.h/cpp` – Command parsing and execution logic
- `signals.h/cpp` – Signal handler definitions
- `smash.cpp` – Entry point and shell loop
- `Makefile` – Compilation script
