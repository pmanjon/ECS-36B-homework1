//Used for exit constants mostly
#include <stdlib.h>
//printf and puts
#include <stdio.h>
//string comparison functions
#include <string.h>
//Used to seed the rng for random games
#include <time.h>
//I added this to use toupper which turns a char into its uppercase
#include <ctype.h>

//Length of a word.
//Note: You cannot change this without changing the word list too.
//You also must adapt the scanf calls for the new length
#define WORD_LENGTH 5
//Number of tries allowed to find the word
#define MAX_TRIES 20
//Number of characters in the alphabet + 1
#define ALPHA_SIZE 27

//If set, the word and a few stats are printed before the game starts
//#define DEBUG

//Very cheap error termination script
#define err(x) fprintf(stderr, EOL "[%s:%i] Fatal error: %s" EOL, __FILE__, __LINE__, x);abort();

//Note: CRLF is also used for telnet.
//If you want to make it available in a BBS you may want to force a CRLF
#ifdef WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif

//Cheap boolean
#define bool int
#define false (0)
#define true (!false)

//Files for lists that contain all words and solutions
FILE * fpAll,  * fpSol, fpSolCopy;
//Number of words in the solution list
long wordCount = 0;
//Selected word from solution list
char word[WORD_LENGTH + 1] = {0};
//Possible characters (used to show unused characters)
//The size specifier is necessary or its value will be readonly
char alpha[ALPHA_SIZE] = "abcdefghijklmnopqrstuvwxyz";
//Memory cache:
//0-25 File position in the complete list with words that start with the given letter a-z
//26: Number of words in the solution list
long memcache[ALPHA_SIZE];

//turns characters into its corresponding number in the alphabet starting a=0, b=1, etc.
int toNum(char arg);
//an algorithm to find the solution. 
//Inputs are: string holding the next guess. current # of guesses, the last guess,
//the result of the guess (something like __#o_), and the list of possible solutions
//Has no output, copies what the next guess should be into "nextGuess"
void bruteForce(char nextGuess[WORD_LENGTH + 1], int guessCount, char* guess, char * result, char solutionList[wordCount][WORD_LENGTH + 1]);

//Inputs: letter that can't be in solution, and the solution list
//Will change list of possible solutions so words with "letter" are excluded
//updates the impossibleLet array
void countImpLetter(char letter, char solutionList[wordCount][WORD_LENGTH + 1]);
//Inputs: letter that must be in solution, and the solution list
//Will change list of possible solutions so words that do NOT have "letter" are excluded
//updates the confirmedLet array
void countConfirmedLetter(char letter, char solutionList[wordCount][WORD_LENGTH + 1]);

//puts the first word that is still in "solutionList" into "possibleGuess".
void getPossibleGuess(char possibleGuess[WORD_LENGTH + 1], char solutionList[wordCount][WORD_LENGTH + 1]);
//removes guess from the solution list
void removeWord(char guess[WORD_LENGTH + 1], char solutionList[wordCount][WORD_LENGTH + 1]);
//Runs the main game loop automatically from a starting word to an ending word
void autoGameLoop(char solutionList[wordCount][WORD_LENGTH + 1]);

//Reads solution list and picks a random word
long setup(void);
//Pick the given word
int pickWord(char * word, int index);
//Checks if the supplied word is in the list
bool hasWord(const char * word);
//Convert to lowercase
int toLower(char * str);
//Checks the entered word against the solution. The result is copied onto output.
//for example, by the end output could be "o___#"
void checkWord(const char * guess, char* output);
//Checks if the entered string is a potentially valid word
bool isWord(const char* word);
//Gets the first position of the given char in the given string
int strpos(const char * str, char search);
//Removes characters in the supplied argument from the alphabet
void removeAlpha(const char * guess);
//Runs the main game loop
void gameLoop(char solutionList[wordCount][WORD_LENGTH + 1]);
//Runs the menu
int menu(void);
//Shows the help text
void help(void);

//array of letters that are not in the solution
char impossibleLet[ALPHA_SIZE] = {0};
//array of letters that must be in the solution
char confirmedLet[ALPHA_SIZE] = {0};
//bruteList is the array of words that the "brute-force" algorithm will use to find the solution
const char BRUTELIST[10][WORD_LENGTH + 1] = {"irate", "epoxy", "zebra", "embed", "guava", "chafe", "jewel", "karma", "ninth", "quest"};
//string of what the next guess should be
char nextGuess[WORD_LENGTH + 1] = {0};


//Main function
int main() {
	int gameId;
	setbuf(stdout, NULL);
	//Note: This will search for the file in the current directory
	fpAll = fopen("lists/ALL.TXT", "r");
	fpSol = fopen("lists/SOLUTION.TXT", "r");
	if (fpAll == NULL || fpSol == NULL) {
		err("error opening wordle lists");
	}


	
	#ifdef DEBUG
	printf("Word count: %i" EOL, setup());
	#else
	setup();
	//solutionList is a list containing a copy of the solutions.
	//solutionListCopy is the same thing except for the fact that it never changes.
	//solutionListCopy will later be used to copy its content into solutionList multiple times
	//for the game loop to function.
	char solutionListCopy[wordCount][WORD_LENGTH + 1];
	char solutionList[wordCount][WORD_LENGTH + 1];
	for (int i = 0; i < wordCount; i++){
		fscanf(fpSol, "%6s", solutionListCopy[i]);
		strcpy(solutionList[i], solutionListCopy[i]);
	}

	#endif/*
	*/
	//char * input;
	char input[20];
	printf("Enter \"auto\" if you want to the program to run itself ");
	scanf("%20s", input);
	toLower(input);
	//if the user enters "auto" then the algorithm will run itself multiple times
	//it will start at the "start" word and end at the "end" word.
	//this uses autoGameLoop instead of gameLoop
	if (strcmp(input, "auto") == 0) {
		int start = 0, end = -1;
		//the while loop serves to make sure that "start" and "end" are valid inputs
		while (end < start || end > wordCount) {
			printf("Enter the word number you want to start with. (Must be within 1-%li)" EOL, wordCount);
			scanf("%d", &start);
			printf("Enter the word number you want to end with. (Must be within 1-%li)" EOL, wordCount);
			scanf("%d", &end);
		}
		
		start--;
		for (gameId = start; gameId < end; gameId++) {
			pickWord(word, gameId);
			printf("Running game #%i" EOL, gameId + 1);
			autoGameLoop(solutionList);

			//resets the solutionList for each new game of Wordle
			for (int i = 0; i < wordCount; i++){
				strcpy(solutionList[i], solutionListCopy[i]);
			}
			//resets confirmedLet and impossibleLet so game can run again
			for (int i = 0; i < ALPHA_SIZE; i++) {
				impossibleLet[i] = '\0';
				confirmedLet[i] = '\0';
			}
		}
	}
	//if the user does not want the program to run automatically then the game will play normally
	else {
		gameId = menu();
		if (gameId >= 0) {
			pickWord(word, gameId);
			printf("Running game #%i" EOL, gameId + 1);
			gameLoop(solutionList);
		} else {
			return EXIT_SUCCESS;
		}
	}
	
	
	fclose(fpAll);
	fclose(fpSol);
	return EXIT_SUCCESS;
}

int menu() {
	char buffer[21] = {0};
	int gameId = 0;
	int scan = 0;

	puts(
		"Main Menu" EOL
		"=========" EOL
		"NEW: start a new game." EOL
		"LOAD <num>: load a specific game" EOL
		"HELP: More information" EOL
		"EXIT: End game");
	printf("The game number must be in range of 1-%li" EOL, wordCount);
	while (true) {
		printf("Input: ");
		while ((scan = scanf("%20s", buffer)) != 1) {
			if (scan == EOF) {
				return -1;
			}
		}

		toLower(buffer);
		if (strcmp(buffer, "exit") == 0) {
			return -1;
		} else if (strcmp(buffer, "help") == 0) {
			help();
		} else if (strcmp(buffer, "new") == 0) {
			return rand() % wordCount;
		} else if (strcmp(buffer, "load") == 0) {
			if (scanf("%i",  & gameId) == 1) {
				if (gameId > 0 && gameId <= wordCount) {
					return gameId - 1;
				}
			}
			puts("Invalid number");
		} else {
			puts("Invalid input");
		}
	}
}

void help() {
	printf("Wordle is a simple game:" EOL "Guess the %i letter word within %i tries" EOL, WORD_LENGTH, MAX_TRIES);
	puts(
		"After every guess, hints are shown for each character." EOL
		"They look like this:" EOL
		"  _ = Character not found at all" EOL
		"  # = Character found and position correct" EOL
		"  o = Character found but position wrong" EOL
		"Unused letters of the alphabet are shown next to the hint" EOL
		EOL
		"The game prefers valid positions over invalid positions," EOL
		"And it handles double letters properly." EOL
		"Guessing \"RATES\" when the word is \"TESTS\" shows \"__oo#\"");
}

void gameLoop(char solutionList[wordCount][WORD_LENGTH + 1]) {
	char guess[WORD_LENGTH + 1] = {0};
	int guesses = 0;
	int scan = 0;
	char result[6] = {0};
	char nextGuess[6] = {0};
	
	//gameMode will be 1 if the user wants to use the brute-force algorithm
	//or 0 if the user does not want to use the algorithm
	int gameMode;
	do {
		printf("Enter 1 for algorithm, 0 for no algorithm: ");
		scanf("%d", &gameMode);
	} while (!(gameMode == 0 || gameMode == 1));
	
	puts(
		"word\tunused alphabet" EOL
		"====\t===============");
	
	
	if (gameMode == 1) {
		printf("You should guess \"%s\" for your first guess\n", BRUTELIST[0]);
	}

	while (guesses < MAX_TRIES && strcmp(guess, word)) {
		printf("Guess %i: ", guesses + 1);
		if ((scan = scanf("%5s", guess)) == 1 && strlen(guess) == WORD_LENGTH) {
			toLower(guess);
			//Do not bother doing all the test logic if we've found the word.
			if (strcmp(guess, word)) {
				if (isWord(guess) && hasWord(guess)) {
					++guesses;
				
					//checkWord assigns a value to result (such as "o___#")
					checkWord(guess, result);
					printf("%s", result);
					removeAlpha(guess);
					printf("\t%s\n", alpha);
					if(gameMode == 1){
						//Since "guess" has already been tried, remove it from the possible solutions list
						removeWord(guess, solutionList);
						//algorithm will copy what the next guess should be into nextGuess
						bruteForce(nextGuess, guesses, guess, result, solutionList);
						printf("Your next guess should be \"%s\"\n", nextGuess);
					}
					
				} else {
					puts("Word not in list");
				}
			}
		} else {
			if (scan == EOF) {
				exit(EXIT_FAILURE);
			}
			printf("Invalid word. Must be %i characters\n", WORD_LENGTH);
		}
	}
	if (strcmp(guess, word)) {
		printf("You lose. The word was %s\n", word);
	} else {
		puts("You win");
	}
}

void autoGameLoop(char solutionList[wordCount][WORD_LENGTH + 1]) {
	char guess[WORD_LENGTH + 1] = {0};
	int guesses = 0;
	int scan = 0;
	char result[6] = {0};
	char nextGuess[6] = {0};
	
	puts(
		"word\tunused alphabet" EOL
		"====\t===============");
	
	
	printf("You should guess \"%s\" for your first guess\n", BRUTELIST[0]);
	
	while (guesses < MAX_TRIES && strcmp(guess, word)) {
		if (guesses == 0) strcpy(guess, BRUTELIST[0]);
		else {
			strcpy(guess, nextGuess);
		};
		printf("Guess %i: %s\n", guesses + 1, guess);
		//Since "guess" has already been tried, remove it from the possible solutions list
		removeWord(guess, solutionList);
		
		//Do not bother doing all the test logic if we've found the word.
		if (strcmp(guess, word)) {
			if (isWord(guess) && hasWord(guess)) {
				++guesses;
					
				//checkWord assigns a value to result (such as "o___#")
				checkWord(guess, result);
				printf("%s", result);
				removeAlpha(guess);
				printf("\t%s\n", alpha);
				//copy what the next guess should be into nextGuess
				bruteForce(nextGuess, guesses, guess, result, solutionList);
				printf("Your next guess should be \"%s\"\n", nextGuess);
					
			} else {
				puts("Word not in list");
			}
		}
		
	}

	if (strcmp(guess, word)) {
		printf("You lose. The word was %s\n", word);
	} else {
		puts("You win");
	}
}

void removeAlpha(const char * guess) {
	int i = 0;
	int pos = 0;
	if (guess != NULL) {
		while (guess[i]) {
			pos = strpos(alpha, guess[i]);
			if (pos >= 0) {
				alpha[pos] = '_';
			}
			++i;
		}
	}
}

int strpos(const char * str, char search) {
	int i = 0;
	if (str != NULL) {
		while (str[i]) {
			if (str[i] == search) {
				return i;
			}
			++i;
		}
	}
	return -1;
}

void checkWord(const char * guess, char* output) {
	if(strlen(guess) != strlen(word)) {
		return;
	}
	int i = 0;
	int pos = -1;
	//Copy is used to blank found characters
	//This avoids wrong reports for double letters, for example "l" in "balls"
	char copy[WORD_LENGTH + 1];
	char result[WORD_LENGTH + 1];
	result[WORD_LENGTH] = 0;
	strcpy(copy, word);
	//Do all correct positions first
	while (copy[i]) {
		if (copy[i] == guess[i]) {
			//Character found and position correct
			result[i] = '#';
			copy[i] = '_';
		} else {
			//Fills remaining slots with blanks
			//We could do this before the loop as well
			result[i] = '_';
		}
		++i;
	}
	i = 0;
	while (copy[i]) {
		pos = strpos(copy, guess[i]);
		//Char must exist but do not overwrite a good guess
		if (pos >= 0 && result[i] != '#') {
			//Character found but position wrong
			result[i] = 'o';
			copy[pos] = '_';
		}
		++i;
	}
	//result is copied into output which is used outside of the function
	strcpy(output, result);
}

int toLower(char * str) {
	int i = 0;
	while (str[i]) {
		if (str[i] >= 'A' && str[i] <= 'Z') {
			str[i] |= 0x20; //Make lowercase
		}
		++i;
	}
	return i;
}

int hasWord(const char * word) {
	//A bit longer to also contain the line terminator
	char buffer[WORD_LENGTH + 4];
	//Don't bother if the argument is invalid
	if (word == NULL || !isWord(word)) {
		return false;
	}
	fseek(fpAll, memcache[word[0]-'a'], SEEK_SET);
	//Stop comparing once we are beyond the current letter
	while (fgets(buffer, WORD_LENGTH + 4, fpAll) != NULL && buffer[0]==word[0]) {
		buffer[WORD_LENGTH]=0;
		if (strcmp(word, buffer) == 0) {
			return true;
		}
	}
	return false;
}

bool isWord(const char* word){
	int i=-1;
	if(strlen(word) == WORD_LENGTH){
		while(word[++i]){
			if(word[i]<'a' || word[i]>'z'){
				return false;
			}
		}
		return true;
	}
	return false;
}

long setup() {
	FILE* cache;
	char currentChar;
	char currentWord[WORD_LENGTH + 1];
	bool success = false;
	
	//Don't bother if setup was already performed
	if (wordCount > 0) {
		return wordCount;
	}
	srand((int)time(0));
	
	if ((cache = fopen("LISTS/CACHE.BIN","rb")) != NULL) {
		printf("Reading cache... ");
	    success = fread(memcache, sizeof(long), ALPHA_SIZE, cache) == ALPHA_SIZE;
	    fclose(cache);
		if(success){
			puts(" [OK]");
			return wordCount = memcache[ALPHA_SIZE - 1];
		}
		else{
			puts(" [FAIL]");
		}
	}
	
	printf("Loading word list...");
	fseek(fpSol, 0, SEEK_SET);
	while (fgets(currentWord, WORD_LENGTH + 1, fpSol) != NULL) {
		//Only increment if word length is correct
		if (strlen(currentWord) == WORD_LENGTH) {
			++wordCount;
		}
	}
	puts(" [OK]");
	memcache[ALPHA_SIZE-1] = wordCount;

	if (!success) {
	    printf("Building cache...");
		currentChar = 'a';
		memcache[0] = 0;
		fseek(fpAll, 0, SEEK_SET);
		while (fgets(currentWord, WORD_LENGTH + 1, fpAll) != NULL) {
			//Only proceed if word length is correct
			if (strlen(currentWord) == WORD_LENGTH) {
			    if (currentChar != currentWord[0]) {
			        currentChar = currentWord[0];
			        memcache[currentChar - 'a'] = ftell(fpAll) - 5;
			    }
			}
		}
		cache = fopen("LISTS/CACHE.BIN", "wb");
		if (cache == NULL) {
			puts(" [FAIL]");
		}
		else{
			fwrite(memcache, sizeof(long), ALPHA_SIZE, cache);
			fclose(cache);
			puts(" [OK]");
		}
	}

	return wordCount;
}

int pickWord(char * word, int index) {
	int i = 0;
	fseek(fpSol, 0, SEEK_SET);
	while (i <= index && fgets(word, WORD_LENGTH + 1, fpSol) != NULL) {
		if (strlen(word) == WORD_LENGTH) {
			++i;
		}
	}
	toLower(word);
	return index;
}

//this function assumes that the input character will always be a letter in the alphabet
//otherwise does not behave as expected
int toNum(char arg){
	return toupper(arg) - 65;
}

void bruteForce(char nextGuess[WORD_LENGTH + 1], int guessCount, char* guess, char* result, char solutionList[wordCount][WORD_LENGTH + 1]) {
	//Gets rid of words with the impossible letters (letters that are '_')
	//updates confirmedLet and impossibleLet
	for (int i = 0; i < WORD_LENGTH+1; i++) {
		if (result[i] == '#' || result[i] == 'o') {
			countConfirmedLetter(guess[i], solutionList);
		}
		if(result[i] == '_') {
			char letter = guess[i];
			bool shouldNotCount = false;
			//check if there is a duplicate letter that is correct
			for (int j = i + 1; j < WORD_LENGTH; j++) {
				if (guess[i] == guess[j] && (result[j] == '#' || result[j] == 'o')){
					shouldNotCount = true;
					break;
				}
			}
			//does not count a letter as "impossible" if it is a duplicate letter that 
			//is somewhere else in the solution
			if(!shouldNotCount) countImpLetter(guess[i], solutionList);
		}
	}
	if(guessCount<10) {
		strcpy(nextGuess, BRUTELIST[guessCount]);
		return;
	}
	//gets the first word in solutionLis that could be the solution
	char possibleGuess[WORD_LENGTH + 1] = {0};
	getPossibleGuess(possibleGuess, solutionList);
	//the next guess will be the first possible solution we can find
	strcpy(nextGuess, possibleGuess);
}

//this function assumes the input letter truly is not in the solution
void countImpLetter(char letter, char solutionList[wordCount][WORD_LENGTH + 1]) {
	//leave function if letter has already been noted as not being in the solution
	//or if the letter is in the solution but just in a different spot (in case of duplicate letters)
	if(impossibleLet[toNum(letter)] == letter || confirmedLet[toNum(letter)] == letter)  return;
	//note that the letter can no longer be in the solution
	impossibleLet[toNum(letter)] = letter;

	//replaces the word at index i with an empty string if the word contains the letter
	//we want to get rid of
	//strcmp(solutionList[i], "") is there so we dont bother if word was already removed
	for (int i =0; i < wordCount; i++) {
		if(strcmp(solutionList[i], "") != 0 && strpos(solutionList[i], letter) != -1) {
			strcpy(solutionList[i], "");
		}
	}
}

//this function assumes the input letter truly is in the solution
void countConfirmedLetter(char letter, char solutionList[wordCount][WORD_LENGTH + 1]) {
	//leave function if letter has already been noted as being in the solution
	if(confirmedLet[toNum(letter)] == letter) return;
	//note that the letter must be in the solution
	confirmedLet[toNum(letter)] = letter;

	//replaces the word at index i with an empty string if the word DOES NOT contain the letter
	//we want to have
	//strcmp(solutionList[i], "") is there so we dont bother if word was already removed
	for (int i =0; i < wordCount; i++) {
		if(strcmp(solutionList[i], "") != 0 && strpos(solutionList[i], letter) == -1) {
			strcpy(solutionList[i], "");
		}
	}
}

//this function assumes that the word is not in the list more than once
void removeWord(char guess[WORD_LENGTH + 1], char solutionList[wordCount][WORD_LENGTH + 1]) {
	for (int i = 0; i < wordCount; i++) {
		if (strcmp(solutionList[i], guess) == 0) {
			strcpy(solutionList[i], "");
			return;
		}
	}
}

void getPossibleGuess(char possibleGuess[WORD_LENGTH + 1], char solutionList[wordCount][WORD_LENGTH + 1]){
	for (int i = 0; i < wordCount; i++) {
		if (isWord(solutionList[i])) {
			strcpy(possibleGuess, solutionList[i]);
			return;
		}
	}
} 