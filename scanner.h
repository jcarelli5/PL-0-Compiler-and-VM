/*  AUTHOR: Jon Carelli
    DECEMBER 2014
    SIMPLE PL/0 Compiler
    SEE WORD DOCUMENT FOR GRAMMAR
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDENTIFIER_LENGTH_MAX 11
#define NUMBER_LENGTH_MAX 5

typedef enum {
    nulsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5, multsym = 6,
    slashsym = 7, oddsym = 8,  eqlsym = 9, neqsym = 10, lessym = 11, leqsym = 12,
    gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16, commasym = 17,
    semicolonsym = 18, periodsym = 19, becomessym = 20, beginsym = 21, endsym = 22,
    ifsym = 23, thensym = 24, whilesym = 25, dosym = 26, callsym = 27, constsym = 28,
    varsym = 29, procsym = 30, writesym = 31, readsym = 32, elsesym = 33
} token_type;

FILE* input;
FILE* output_cleanInput;
FILE* output_lexEmeList;
FILE* output_lexEmeTable;

char inputFileArr [20000];
int fileLength = 0;
char identifiers[20000][IDENTIFIER_LENGTH_MAX];
int identIndexTracker = 0;
int scannerPrintFlag = 0;

void printClean();
void scanCodeAndPrint();
void printSymbols(char symbol);

int scanner (int flag) {

    int i = 0;
    char temp;

    scannerPrintFlag =  flag;

    // File handling.
    input = fopen("input.txt", "r");
    output_cleanInput = fopen("inputclean.txt", "w");
    output_lexEmeTable = fopen("lexemetable.txt", "w");
    output_lexEmeList = fopen("lexemelist.txt", "w");

    // Scanning in PL/0 and storing it to a character array.
    while(fscanf(input,"%c", &temp) != EOF) {
        inputFileArr[i]= temp;
        i++;
        fileLength++;
    }

    // Removes Comments and prints the clean Input file.
    printClean();

    // Main function, scans the code and prints to the text files.
    scanCodeAndPrint();

    // Closing documents.
    fclose(output_cleanInput);
    fclose(output_lexEmeTable);
    fclose(output_lexEmeList);

    if (scannerPrintFlag == 1) {
        input = fopen("lexemelist.txt", "r");
        char temp[12];

        printf("////////////////////////\n");
        printf("Lexemes / Tokens: \n");
        printf("////////////////////////\n\n");

        while (fscanf(input, "%s ", temp) != EOF) {
            printf("%s ", temp);
        }

        printf("\n\n");
    }
}


// Prints the code without comments to inputclean.txt.
void printClean(){
    int i;

    for (i = 0; i < fileLength; i++) {
        if (inputFileArr[i] == '/' && inputFileArr[i+1] == '*') {
            int j = i + 1;
            // Keep going until we encounter the last '*/'
            while (!(inputFileArr[j] == '*' && inputFileArr[j+1] == '/')) {
                j++;
            }
            // We increment by j + 2 to skip the last '*/' of the comment.
            i = j + 2;

            // Cleans up the line after a comment
            if (inputFileArr[i] == '\n' || inputFileArr[i] == '\r')
                continue;
        }
        fprintf(output_cleanInput, "%c", inputFileArr[i]);
    }
}

void scanCodeAndPrint() {
        int i, tempZ;

        fprintf(output_lexEmeTable, "lexeme\ttoken type");
        for (i = 0; i < fileLength; i++) {

            // Skipping comments
            if (inputFileArr[i] == '/' && inputFileArr[i+1] == '*') {
                int j = i + 1;
                // Keep going until we encounter the last '*/'
                while (!(inputFileArr[j] == '*' && inputFileArr[j+1] == '/')) {
                    j++;
                }
                // We increment by j + 2 to skip the last '*/' of the comment.
                i = j + 2;
            }

            if (inputFileArr[i] == '\r' || inputFileArr[i] == '\n' || inputFileArr[i] == '\t')
                continue;

            // Function to handle reserved words
            // also handles printing and formatting to respective text documents.
            if (inputFileArr[i] >= 'a' && inputFileArr[i] <= 'z') {
                switch ((int) inputFileArr[i]) {

                    /* const / call */
                    case (int) 'c':

                        // const
                        if ((inputFileArr[i+1] == 'o') && (inputFileArr[i+2] == 'n') &&
                            (inputFileArr[i+3] == 's') && (inputFileArr[i+4] == 't')) {
                            i+=4;
                            fprintf(output_lexEmeTable, "\nconst\t%d", constsym);
                            fprintf(output_lexEmeList, "%d ", constsym);
                            continue;
                        }

                        // call
                        else if ((inputFileArr[i+1] == 'a') && (inputFileArr[i+2] == 'l') &&
                                (inputFileArr[i+3] == 'l')) {
                                i+=3;
                                fprintf(output_lexEmeTable, "\ncall\t%d", callsym);
                                fprintf(output_lexEmeList, "%d ", callsym);
                                continue;
                        }

                    // var
                    case (int) 'v':
                        if ((inputFileArr[i+1] == 'a') && (inputFileArr[i+2] == 'r')){
                            i += 2;
                            fprintf(output_lexEmeTable, "\nvar\t%d", varsym);
                            fprintf(output_lexEmeList, "%d ", varsym);
                            continue;
                        }

                    // procedure
                    case (int) 'p':
                        if ((inputFileArr[i+1] == 'r') && (inputFileArr[i+2] == 'o') &&
                            (inputFileArr[i+3] == 'c') && (inputFileArr[i+4] == 'e') &&
                            (inputFileArr[i+5] == 'd') && (inputFileArr[i+6] == 'u') &&
                            (inputFileArr[i+7] == 'r') && (inputFileArr[i+8] == 'e')) {
                            i += 8;
                            fprintf(output_lexEmeTable, "\nprocedure\t%d", procsym);
                            fprintf(output_lexEmeList, "%d ", procsym);
                            continue;
                        }

                    // begin
                    case (int) 'b':
                        if ((inputFileArr[i+1] == 'e') && (inputFileArr[i+2] == 'g') &&
                            (inputFileArr[i+3] == 'i') && (inputFileArr[i+4] == 'n')){
                            i += 4;
                            fprintf(output_lexEmeTable, "\nbegin\t%d", beginsym);
                            fprintf(output_lexEmeList, "%d ", beginsym);
                            continue;
                        }

                    /* end / else */
                    case (int) 'e':

                        // end
                        if ((inputFileArr[i+1] == 'n') && (inputFileArr[i+2] == 'd')){
                            i += 2;
                            fprintf(output_lexEmeTable, "\nend\t%d", endsym);
                            fprintf(output_lexEmeList, "%d ", endsym);
                            continue;
                        }

                        // else
                        else if ((inputFileArr[i+1] == 'l') && (inputFileArr[i+2] == 's') &&
                                 (inputFileArr[i+3] == 'e')) {
                            i += 3;
                            fprintf(output_lexEmeTable, "\nelse\t%d", elsesym);
                            fprintf(output_lexEmeList, "%d ", elsesym);
                            continue;
                        }

                    // if
                    case (int) 'i':
                        if (inputFileArr[i+1] == 'f') {
                            i += 1;
                            fprintf(output_lexEmeTable, "\nif\t%d", ifsym);
                            fprintf(output_lexEmeList, "%d ", ifsym);
                            continue;
                        }

                    // then
                    case (int) 't':
                        if ((inputFileArr[i+1] == 'h') && (inputFileArr[i+2] == 'e') &&
                            (inputFileArr[i+3] == 'n')) {
                            i += 3;
                            fprintf(output_lexEmeTable, "\nthen\t%d", thensym);
                            fprintf(output_lexEmeList, "%d ", thensym);
                            continue;
                        }

                    /* while / write */
                    case (int) 'w':
                        // while
                        if ((inputFileArr[i+1] == 'h') && (inputFileArr[i+2] == 'i') &&
                            (inputFileArr[i+3] == 'l') && (inputFileArr[i+4] == 'e')) {
                            i += 4;
                            fprintf(output_lexEmeTable, "\nwhile\t%d", whilesym);
                            fprintf(output_lexEmeList, "%d ", whilesym);
                            continue;
                        }
                        // write
                        else if ((inputFileArr[i+1] == 'r') && (inputFileArr[i+2] == 'i') &&
                                (inputFileArr[i+3] == 't') && (inputFileArr[i+4] == 'e')) {
                                i += 4;
                                fprintf(output_lexEmeTable, "\nwrite\t%d", writesym);
                                fprintf(output_lexEmeList, "%d ", writesym);
                                continue;
                        }

                    // do
                    case (int) 'd':
                        if ((inputFileArr[i+1] == 'o') && ((inputFileArr[i+2] == ' ') || (inputFileArr[i+2] == '\n'))) {
                            i += 1;
                            fprintf(output_lexEmeTable, "\ndo\t%d", dosym);
                            fprintf(output_lexEmeList, "%d ", dosym);
                            continue;
                        }

                    // odd
                    case (int) 'o':
                        if ((inputFileArr[i+1] == 'd') && (inputFileArr[i+2] == 'd')) {
                            i+=2;
                            fprintf(output_lexEmeTable, "\nodd\t%d", oddsym);
                            fprintf(output_lexEmeList, "%d ", oddsym);
                            continue;
                        }
                    // read
                    case (int) 'r':
                        if ((inputFileArr[i+1] == 'e') && (inputFileArr[i+2] == 'a') &&
                            (inputFileArr[i+3] == 'd')) {
                            i += 3;
                            fprintf(output_lexEmeTable, "\nread\t%d", readsym);
                            fprintf(output_lexEmeList, "%d ", readsym);
                            continue;
                        }
                    default:
                        break;
                }
            }

            // Symbol Checking
            switch ((int) inputFileArr[i]){
                case (int) ':':
                    if((inputFileArr[i+1] == '=')) {
                        i += 1;
                        fprintf(output_lexEmeTable, "\n:=\t%d", becomessym);
                        fprintf(output_lexEmeList, "%d ", becomessym);
                        continue;
                    }

                case (int) '>':
                    if((inputFileArr[i+1] == '=')){
                        i += 1;
                        fprintf(output_lexEmeTable, "\n>=\t%d", geqsym);
                        fprintf(output_lexEmeList, "%d ", geqsym);
                        continue;
                    }

                case (int) '<':
                    if((inputFileArr[i+1] == '=')){
                        i += 1;
                        fprintf(output_lexEmeTable, "\n<=\t%d", leqsym);
                        fprintf(output_lexEmeList, "%d ", leqsym);
                        continue;
                    }
                    else if((inputFileArr[i+1] == '>')){
                        i += 1;
                        fprintf(output_lexEmeTable, "\n<>\t%d", neqsym);
                        fprintf(output_lexEmeList, "%d ", neqsym);
                        continue;
                    }
            }

        // Handling identifiers
        if ((inputFileArr[i] >= 'A' && inputFileArr[i] <= 'Z') ||
            (inputFileArr[i] >= 'a' && inputFileArr[i] <= 'z')) {
			int v, j, k;
            int flag = 1;
			int Identifier_Length;
            char Identifier[IDENTIFIER_LENGTH_MAX];

            for (k = 0; k < IDENTIFIER_LENGTH_MAX; k++)
                Identifier[k]= '\0';

            Identifier_Length = 0;

            j = i;

            while((inputFileArr[j] >= 'A' && inputFileArr[j] <= 'Z') ||
                  (inputFileArr[j] >= 'a' && inputFileArr[j] <= 'z') ||
                  (inputFileArr[j] >= '0' && inputFileArr[j] <= '9')) {

                // ERROR: Identifier too long.
                if (Identifier_Length+1 > 11) {
                    printf("ERROR: Name too long.");
                    exit(EXIT_FAILURE);
                }

                // Character to string handling.
                Identifier[Identifier_Length] = inputFileArr[j];
                Identifier_Length++;
                j++;
            }

            // Copying string to Identifier array.
            strcpy(identifiers[identIndexTracker],Identifier);

            // Printing Identifiers to respective text files.
            fprintf(output_lexEmeTable, "\n%s\t%d", identifiers[identIndexTracker], identsym);
            fprintf(output_lexEmeList, "%d %s ", identsym, identifiers[identIndexTracker]);

            identIndexTracker++;
            i = j-1;
            continue;
        }

        // Handles numbers
        if (inputFileArr[i] >= '0' && inputFileArr[i] <= '9') {
			int digitCount, k;
            int j = i;

            // Finds number length
            for (j = i; j < NUMBER_LENGTH_MAX + i + 1 ; j++) {

                if ((inputFileArr[j] == ' ') || (inputFileArr[j] == '\n') ||
                    (inputFileArr[j] == '\t'))
                    break;

                // ERROR Check: Variable must not start with a number
                if ((inputFileArr[j] >= 'A' && inputFileArr[j] <= 'Z') ||
                    (inputFileArr[j] >= 'a' && inputFileArr[j] <= 'z')) {
                    printf("ERROR: Variable does not start with a letter.");
                    exit(EXIT_FAILURE);
                }
            }

            j = i;
            digitCount = 1 ;

            // ERROR Check: Number too long.
            while (inputFileArr[j] >= '0' && inputFileArr[j] <= '9') {
                if (digitCount > 5) {
                    printf("ERROR: Number too long.");
                    exit(EXIT_FAILURE);
               }
                digitCount++;
                j++;
            }

            digitCount--;

            // Printing numbers and formatting to respective text documents.
            fprintf(output_lexEmeTable,"\n");
            fprintf(output_lexEmeList, "%d ", numbersym);

            for (k = j - digitCount; k < j; k++) {
                fprintf(output_lexEmeTable,"%d", inputFileArr[k] - '0');
                fprintf(output_lexEmeList, "%d", inputFileArr[k] - '0');
                digitCount--;
            }
            fprintf(output_lexEmeTable,"\t%d", numbersym);
            fprintf(output_lexEmeList, " ");

            i = j - 1;
            continue;
        }

        // Function that handles printing just symbols.

        printSymbols(inputFileArr[i]);

    }
}

// INPUT: Symbol
// OUTPUT: Print to Lexemetable.txt and Lexemelist.txt
void printSymbols(char symbol) {
    switch ((int) symbol) {
        case (int) '+':
            fprintf(output_lexEmeTable, "\n+\t%d", plussym);
            fprintf(output_lexEmeList, "%d ", plussym);
            break;

        case (int) '-':
            fprintf(output_lexEmeTable, "\n-\t%d", minussym);
            fprintf(output_lexEmeList, "%d ", minussym);
            break;

        case (int) '*':
            fprintf(output_lexEmeTable, "\n*\t%d", multsym);
            fprintf(output_lexEmeList, "%d ", multsym);
            break;

        case (int) '/':
            fprintf(output_lexEmeTable, "\n/\t%d", slashsym);
            fprintf(output_lexEmeList, "%d ", slashsym);
            break;

        case (int) '=':
            fprintf(output_lexEmeTable, "\n=\t%d", eqlsym);
            fprintf(output_lexEmeList, "%d ", eqlsym);
            break;

        case (int) '<':
            fprintf(output_lexEmeTable, "\n<\t%d", lessym);
            fprintf(output_lexEmeList, "%d ", lessym);
            break;

        case (int) '>':
            fprintf(output_lexEmeTable, "\n>\t%d", gtrsym);
            fprintf(output_lexEmeList, "%d ", gtrsym);
            break;

        case (int) '(':
            fprintf(output_lexEmeTable, "\n(\t%d", lparentsym);
            fprintf(output_lexEmeList, "%d ", lparentsym);
            break;

        case (int) ')':
            fprintf(output_lexEmeTable, "\n)\t%d", rparentsym);
            fprintf(output_lexEmeList, "%d ", rparentsym);
            break;

        case (int) ',':
            fprintf(output_lexEmeTable, "\n,\t%d", commasym);
            fprintf(output_lexEmeList, "%d ", commasym);
            break;

        case (int) ';':
            fprintf(output_lexEmeTable, "\n;\t%d", semicolonsym);
            fprintf(output_lexEmeList, "%d ", semicolonsym);
            break;

        case (int) '.':
            fprintf(output_lexEmeTable, "\n.\t%d", periodsym);
            fprintf(output_lexEmeList, "%d ", periodsym);
            break;

        case (int) ' ':
            break;

        default:
            printf("ERROR: Invalid symbol.");
            exit(EXIT_FAILURE);
    }
}
