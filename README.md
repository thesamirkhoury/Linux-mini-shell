# Mini Shell

---

## Description

The program takes an input from the user, and runs it as a command under the Linux system, ith support for up to 2 pipes, and nohup and ampersand.

## Main Functions

1- wordCounter - Counts how many words in a string, and the sum of letters in every word despite spaces..
2- writeHistory - Writes a strong to a locally stored txt file.
3- printHistory - Prints the strings written in the locally stored txt file.
4- createCommand - Takes a string and parsed in a 2D array where ever word is a separate string terminated with Null at the end.
5- prompt - Prints the current directory, with a shell command line style.
6- containsSpace - Checks if a history command contains spaces.
7- getLine - Converts a history command number to an int.
8- getContent - Gets the command stored in the history file according to the received line number.
9- isHistory - Checks if an input is a history command.
10- countPipe - counts how many pipes in the input.
11- splitPipe - splites the pipe splitted input into separate commands.
12- clearQuotation - Removes the quotation from the input if available.
13- isAmpersand - Checks if the input contains ampersand symbol.
14- isNoHup - Checks if the input contains ampersand symbol in the beginning.
15- createNoHupFile - creates the nohup.txt file, and redirects the output to it.

## How to run

To compile:
in a linux/unix terminal run: `gcc miniShell.c -o miniShell`
a compiled file will be created with the name proxy.

To run:
in a linux/unix terminal run: `./miniShell`
