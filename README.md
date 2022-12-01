# Wordle


This program allows you to play Wordle either by yourself or with the assistance of an algorithm. 
Wordle consists of multiple 5 letter guesses. The goal is to try to find the correct word.
You can also make the algorithm play itself multiple times in a row in a given word range.

When you enter a guess the program will you tell you which letters are or are not in the word.
* An "#" means the letter is in the word and is in the right place
* An "o" means the letter is in the word but not in the right place
* An "_" means the letter is not in the word at all

Keep in mind that a duplicate letter might show as #/o and _ at the same time!
To the right of this information you can see which letters of the alphabet haven't been tried yet.

## How to build
clone the repository with 
```bash
git clone https://github.com/pmanjon/ECS-36B-homework1.git
```

run make:

```bash
cd hw1 
make
```
## How to run:
```bash
./wordle 
```

You will see this message ```Enter "auto" if you want to the program to run itself```

If you enter ```auto``` then the program will ask for a word to start on and a word to end on.
The algorithm will solve each of the words in between. (Make sure your inputs are numbers)

If you enter anything else then the you will see this menu
```
NEW: start a new game.
LOAD <num>: load a specific game
HELP: More information
EXIT: End game
The game number must be in range of 1-2315
Input: 
```

If you enter ```new``` then a random word will be chosen for you. 
If you write ```load```, press enter, and then enter a number of your choosing then that word will be chosen.
Entering ```help``` explains the rules of the game.
Entering ```exit``` does what you expect.

If you at any point see this menu:
```
Enter 1 for algorithm, 0 for no algorithm: 
```
Then you can enter 1 for the algorthinm to assist you and 0 if you want to play without help.

# Changes to source code:
Changed 
```
fpAll = fopen("LISTS\\ALL.TXT", "r");
fpSol = fopen("LISTS\\SOLUTION.TXT", "r");
```
into 
```
fpAll = fopen("LISTS/ALL.TXT", "r");
fpSol = fopen("LISTS/SOLUTION.TXT", "r");
```
## Function header changes:
```bool checkWord(const char * guess)``` 
turned into 
```void checkWord(const char * guess, char* output)```
output is the string that the result is copied into.

```void gameLoop()```
turned into 
```void gameLoop(char solutionList[wordCount][WORD_LENGTH + 1])```
solutionList is a list containing all the possible solutions

## Functions' insides that were changed
I changed gameLoop, checkWord, pickWord, maybe some others.

There are new variables/functions that were made by myself. You can find descriptions of them in the beginning of wordle.c