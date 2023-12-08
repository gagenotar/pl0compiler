// Gage Notargiacomo, Fall 2023
// This program was made for Systems and Software.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_SIZE 1000
#define MAX_SYMBOL_TABLE_SIZE 500


typedef struct symbol
{
    int kind; // const = 1, var = 2, proc = 3
    char name[10]; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
    int mark; // to indicate unavailable or deleted
} symbol;

typedef struct symbol_table
{
    symbol table[MAX_SYMBOL_TABLE_SIZE]; // Contains all symbols
    int size; // Table size
    int symIdx; // Symbol index
    int current_level; // Holds the current level
    int procIdx; // Holds the index of the current proc
    int declare; // 1 for declaration, 0 for accessing values
} symbol_table;

typedef struct assembly
{
    int OP; // OP code name
    int L; // Lexicographical level
    int M; // Changes depending on OP
} assembly;

typedef struct code_seg
{
    assembly code[MAX_SIZE]; // Contains all the assembly code
    int size; // Size of code_seg
    int cx; // Code index
} code_seg;

typedef struct token_list
{
    char names[MAX_SIZE][MAX_SIZE]; // Holds a list of all named variables
    int nums[MAX_SIZE]; // Holds a list of all numsym values
    int num_count; // Stores number of numsym values
    int tokens[MAX_SIZE]; // Holds a list of all token values
    int token; // Holds the current token
    int current_index; // Holds index of the current token
    int next_index; // Holds index of the next token
    int size; // Holds the size of the token_list
} token_list;

symbol_table global_sym_table;
code_seg global_code;
token_list global_tkn_list;

int addMultiDigitSymbol (char ogChars[], int index, int numNames);
int addMultiCharSymbol (char ogChars[], int index, int numNames);
int isKeyword (int index);
int isComment (char ogChars[], int index);
int get_next_token();
int symbol_table_check();
void error(int error_num);
void emit(int OP, int L, int M);
void program();
void block();
void const_declaration();
int var_declaration();
void procedure_declaration(int jmpaddr);
void statement();
void condition();
void expression();
void term();
void factor();

int main (int argc, char **argv) 
{
    char *inFile = argv[1];
    
    // Read in the file, giving feedback if the file doesn't exist and print out the contents
    // of that given file
    FILE *inPtr = fopen(inFile, "r");
    if (inPtr == NULL)
    {
        printf("Error opening file\n"); 
        return 0;
    }

    char ch;
    int size = 0;
    char ogChars[MAX_SIZE];

    // Prints contents of the file as "SOURCE PROGRAM"
    printf("Source Program:\n");
    while ((ch = fgetc(inPtr)) != EOF) {
        printf("%c", ch);
        ogChars[size] = ch;
        size++;

        // Checks if the char array can fit the extra characters
        if (size >= MAX_SIZE - 1) {
            break;
        }
    }
    printf("\n");
    fclose(inPtr);

    int tokens[MAX_SIZE];
    int numTokens = 0;
    int numNames = 0;
    char currentChar = ' ';
    global_tkn_list.num_count = 0;
    
    // This for loop is the main loop that iterates over the array of symbols
    for (int i=0; i<size; i++)
    {
        // Accounts for whitespace to be ignored
        currentChar = ogChars[i];
        if (currentChar < 33)
        {
            continue;
        }

        // Checks for numbers and symbols of any type
        else if (currentChar > 32 && currentChar < 65)
        {
            // Check for Numbers
            if (currentChar >= 48 && currentChar <= 57)
            {
                i = addMultiDigitSymbol(ogChars, i, numNames) - 1;
                if (strlen(global_tkn_list.names[numNames]) < 6) {
                    tokens[numTokens] = 3;
                    global_tkn_list.nums[global_tkn_list.num_count] = atoi(global_tkn_list.names[numNames]);
                    global_tkn_list.num_count++;
                }
                else
                    tokens[numTokens] = 36;
                numTokens++;
                numNames++;
                
            }

            // Check for Special Characters, Comments, and Invalid Symbols
            else 
            {
                switch (currentChar)
                {
                case '+':
                    tokens[numTokens] = 4;
                    strcpy(global_tkn_list.names[numNames], "+");
                    numNames++;
                    numTokens++;
                    break;
                case '-':
                    tokens[numTokens] = 5;
                    strcpy(global_tkn_list.names[numNames], "-");
                    numNames++;
                    numTokens++;
                    break;
                case '*':
                    tokens[numTokens] = 6;
                    strcpy(global_tkn_list.names[numNames], "*");
                    numNames++;
                    numTokens++;
                    break;
                case '=':
                    tokens[numTokens] = 9;
                    strcpy(global_tkn_list.names[numNames], "=");
                    numNames++;
                    numTokens++;
                    break;
                case '(':
                    tokens[numTokens] = 15;
                    strcpy(global_tkn_list.names[numNames], "(");
                    numNames++;
                    numTokens++;
                    break;
                case ')':
                    tokens[numTokens] = 16;
                    strcpy(global_tkn_list.names[numNames], ")");
                    numNames++;
                    numTokens++;
                    break;
                case ',':
                    tokens[numTokens] = 17;
                    strcpy(global_tkn_list.names[numNames], ",");
                    numNames++;
                    numTokens++;
                    break;
                case '.':
                    tokens[numTokens] = 19;
                    strcpy(global_tkn_list.names[numNames], ".");
                    numNames++;
                    numTokens++;
                    break;
                case ';':
                    tokens[numTokens] = 18;
                    strcpy(global_tkn_list.names[numNames], ";");
                    numNames++;
                    numTokens++;
                    break;
                    
                case '/':
                    if ((i + 1) <= size && ogChars[i+1] == 42)
                    {
                        i = isComment(ogChars, i + 1);
                        break;
                    }
                    
                    tokens[numTokens] = 7;
                    strcpy(global_tkn_list.names[numNames], "/");
                    numNames++;
                    numTokens++;
                    break;
                case ':':
                    if ((i + 1) <= size && ogChars[i+1] == 61)
                    {
                        tokens[numTokens] = 20;
                        strcpy(global_tkn_list.names[numNames], ":=");
                        numNames++;
                        numTokens++;
                        i++;
                        break;
                    }
                    
                    tokens[numTokens] = 34;
                    strcpy(global_tkn_list.names[numNames], ":");
                    numNames++;
                    numTokens++;
                    break;
                case '>':
                    if ((i + 1) <= size && ogChars[i+1] == 61)
                    {
                        tokens[numTokens] = 14;
                        strcpy(global_tkn_list.names[numNames], ">=");
                        numNames++;
                        numTokens++;
                        i++;
                        break;
                    }
                    
                    tokens[numTokens] = 13;
                    strcpy(global_tkn_list.names[numNames], ">");
                    numNames++;
                    numTokens++;
                    break;
                case '<':
                    if ((i + 1) <= size && ogChars[i+1] == 62)
                    {
                        tokens[numTokens] = 10;
                        strcpy(global_tkn_list.names[numNames], "<>");
                        numNames++;
                        numTokens++;
                        i++;
                        break;
                    }
                    else if ((i + 1) <= size && ogChars[i+1] == 61)
                    {
                        tokens[numTokens] = 12;
                        strcpy(global_tkn_list.names[numNames], "<=");
                        numNames++;
                        numTokens++;
                        i++;
                        break;
                    }
                    
                    tokens[numTokens] = 11;
                    strcpy(global_tkn_list.names[numNames], "<");
                    numNames++;
                    numTokens++;
                    break;
                
                default:
                    tokens[numTokens] = 34;
                    char tempStr[2];
                    tempStr[0] = currentChar;
                    tempStr[1] = '\0';
                    strcpy(global_tkn_list.names[numNames], tempStr);
                    numNames++;
                    numTokens++;
                }
            }
        }

        // Checks for words of any kind
        else if (currentChar > 64 && currentChar < 123)
        {
                i = addMultiCharSymbol(ogChars, i, numNames) - 1;
                if (strlen(global_tkn_list.names[numNames]) < 12)
                    tokens[numTokens] = isKeyword(numNames);
                else
                    tokens[numTokens] = 35;
                numTokens++;
                numNames++;
        }

        // Otherwise, the symbol is invalid and will throw a corresponding error in the Lexeme Table
        else
        {
            tokens[numTokens] = 34;
            char tempStr[2];
            tempStr[0] = currentChar;
            tempStr[1] = '\0';
            strcpy(global_tkn_list.names[numNames], tempStr);
            numNames++;
            numTokens++;
        }
    }    

    // Fills the global token list
    for (int i=0; i<numTokens; i++){
        global_tkn_list.tokens[i] = tokens[i];
    }
    global_tkn_list.size = numTokens;

    // Prints out the lexeme table
    printf("\nLexeme Table:\n\nlexeme\t\ttoken type\n");
    for (int i=0; i<numNames; i++)
    {
        if (global_tkn_list.tokens[i] == 34)
        {
            printf("%-12s\tError: Symbol is invalid\n", global_tkn_list.names[i]);
            exit(0);
        }
        else if (global_tkn_list.tokens[i] == 35)
        {
            printf("%-12s\tError: Name is too long\n", global_tkn_list.names[i]);
            exit(0);
        }
        else if (global_tkn_list.tokens[i] == 36)
        {
            printf("%-12s\tError: Too many digits\n", global_tkn_list.names[i]);
            exit(0);
        }
        else
            printf("%-12s\t%d\n", global_tkn_list.names[i], global_tkn_list.tokens[i]);
    }

    // Prints out the token list
    printf("\nToken List:\n");
    for (int i=0; i<numTokens; i++)
    {
        if (global_tkn_list.tokens[i] == 2 || global_tkn_list.tokens[i] == 3)
        {
            printf("%d %s ", global_tkn_list.tokens[i], global_tkn_list.names[i]);
        }
        else if (global_tkn_list.tokens[i] > 33)
        {
            continue;
        }
        else
            printf("%d ", global_tkn_list.tokens[i]);
    }

    // Calls the compiler
    global_tkn_list.num_count = 0;
    program();
    printf("\n\nThis program is syntactically correct! Good job\n");

    // Prints out Assembly Instructions to screen
    printf("\nLine\tOP\tL\tM\n");
    printf("0\tJMP\t0\t3\n");
    for (int i=0; i<global_code.size + 1; i++){
        switch(global_code.code[i].OP) {
            case 1:
                printf("%d\tLIT\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 2:
                printf("%d\tOPR\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 3:
                printf("%d\tLOD\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 4:
                printf("%d\tSTO\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 5:
                printf("%d\tCAL\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 6:
                printf("%d\tINC\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 7:
                printf("%d\tJMP\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 8:
                printf("%d\tJPC\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
            case 9:
                printf("%d\tSYS\t%d\t%d\n", i, global_code.code[i].L, global_code.code[i].M);
                break;
        }
    }

    // Prints out Assembly Instructions to screen
    FILE *code_out;  
    code_out = fopen("elf.txt", "w");
    for (int i=0; i<global_code.size + 1; i++){
        switch(global_code.code[i].OP) {
            case 1:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 2:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 3:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 4:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 5:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 6:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 7:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 8:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
            case 9:
                fprintf(code_out, "%d\t%d\t%d\n", global_code.code[i].OP, global_code.code[i].L, global_code.code[i].M);
                break;
        }
    }
    fclose(code_out);

    printf("\nSymbol Table\n");
    printf("Kind \t|Name \t|Value \t|Level \t|Address \t|Mark \n");
    printf("-------------------------------------------------------\n");
    for (int i=0; i<global_sym_table.size; i++){
        if (global_sym_table.table[i].kind == 1) {
        printf("%d \t|%s \t|%d \t|- \t|- \t\t|%d\n", global_sym_table.table[i].kind, 
                    global_sym_table.table[i].name, 
                    global_sym_table.table[i].val, 
                    global_sym_table.table[i].mark);
        }
        else {
        printf("%d \t|%s \t|%d \t|%d \t|%d \t\t|%d\n", global_sym_table.table[i].kind, 
                    global_sym_table.table[i].name, 
                    global_sym_table.table[i].val, 
                    global_sym_table.table[i].level, 
                    global_sym_table.table[i].addr, 
                    global_sym_table.table[i].mark);
        }
    }

    return 0;
}

// This function will add a multi-digit symbol to the name table and returns the new index so that
// main won't reiterate over previous symbols
int addMultiDigitSymbol (char ogChars[], int index, int numNames)
{
    if (isdigit(ogChars[index]))
    {
        char tempStr[2];
        tempStr[0] = ogChars[index];
        tempStr[1] = '\0';
        strcpy(global_tkn_list.names[numNames], strcat(global_tkn_list.names[numNames], tempStr));
        index = addMultiDigitSymbol (ogChars, index + 1, numNames);
    }
    return index;
}

// This function will add a multi-character symbol to the name table and returns the new index so that
// main won't reiterate over previous symbols
int addMultiCharSymbol (char ogChars[], int index, int numNames)
{
    if (isalnum(ogChars[index]))
    {
        char tempStr[2];
        tempStr[0] = ogChars[index];
        tempStr[1] = '\0';
        strcpy(global_tkn_list.names[numNames], strcat(global_tkn_list.names[numNames], tempStr));
        index = addMultiCharSymbol (ogChars, index + 1, numNames);
    }
    return index;
}

// This function determines if a given string is a keyword and will return the proper token value 
// accordingly
int isKeyword (int index)
{
    if (strcmp(global_tkn_list.names[index], "odd") == 0)
    {
        return 1;
    }
    else if (strcmp(global_tkn_list.names[index], "if") == 0)
    {
        return 23;
    }
    else if (strcmp(global_tkn_list.names[index], "become") == 0)
    {
        return 20;
    }
    else if (strcmp(global_tkn_list.names[index], "begin") == 0)
    {
        return 21;
    }
    else if (strcmp(global_tkn_list.names[index], "end") == 0)
    {
        return 22;
    }
    else if (strcmp(global_tkn_list.names[index], "then") == 0)
    {
        return 24;
    }
    else if (strcmp(global_tkn_list.names[index], "while") == 0)
    {
        return 25;
    }
    else if (strcmp(global_tkn_list.names[index], "do") == 0)
    {
        return 26;
    }
    else if (strcmp(global_tkn_list.names[index], "call") == 0)
    {
        return 27;
    }
    else if (strcmp(global_tkn_list.names[index], "const") == 0)
    {
        return 28;
    }
    else if (strcmp(global_tkn_list.names[index], "var") == 0)
    {
        return 29;
    }
    else if (strcmp(global_tkn_list.names[index], "procedure") == 0)
    {
        return 30;
    }
    else if (strcmp(global_tkn_list.names[index], "write") == 0)
    {
        return 31;
    }
    else if (strcmp(global_tkn_list.names[index], "read") == 0)
    {
        return 32;
    }
    else
        return 2;
}

// This function will be called if a comment is recognized and will return the proper index
// for main to iterate over as to ignore anything within a comment.
int isComment (char ogChars[], int index)
{
    if (ogChars[index] != 47)
    {
        index = isComment(ogChars, index + 1);
    }
    return index;
}

// Returns the index of the next token so long as the accessed index is valid
int get_next_token(){
    if (global_tkn_list.next_index < 0 || global_tkn_list.next_index >= global_tkn_list.size)
        return -1;
    return global_tkn_list.next_index;
}

// Updates the token list so that it shifts to the next index 
void update_tokens(int index){
    global_tkn_list.current_index = index;
    global_tkn_list.next_index = global_tkn_list.current_index + 1;
    global_tkn_list.token = global_tkn_list.tokens[global_tkn_list.current_index];
}

// Checks if the symbol table contains the current token index name .
// Returns the index if it is in the symbol table, returns -1 otherwise.
int symbol_table_check(){
    if (global_sym_table.declare == 1) {
        for (int i=global_sym_table.size - 1; i>=0; i--){  
            if (global_sym_table.table[i].kind == 1) {
                if (strcmp(global_sym_table.table[i].name, global_tkn_list.names[global_tkn_list.current_index]) == 0)
                    return i;
            }
            else {
                if ((strcmp(global_sym_table.table[i].name, global_tkn_list.names[global_tkn_list.current_index]) == 0)) {
                    if (global_sym_table.table[i].level == global_sym_table.current_level) {
                        if (global_sym_table.table[i].mark == 0) {
                            return i;
                        }
                    }
                }
            }
        }
    }
    else {
        for (int i=global_sym_table.size - 1; i>=0; i--){  
            if (global_sym_table.table[i].kind == 1) {
                if (strcmp(global_sym_table.table[i].name, global_tkn_list.names[global_tkn_list.current_index]) == 0)
                    return i;
            }
            else {
                if ((strcmp(global_sym_table.table[i].name, global_tkn_list.names[global_tkn_list.current_index]) == 0)) {
                    if (global_sym_table.table[i].mark == 0) {
                        return i;
                    }                    
                    if (global_sym_table.table[i].mark == 1) {
                        error(20);
                    }
                }
            }
        }
    }
    return -1;
}

// Based on a given error number, the corresponding error will be outputted to a text file.
// Afterwards, the program will exit
void error(int error_num){
    FILE *fptr;
    switch (error_num) {
        case 1:
            printf("Error: program must end with period\n"); 
            fptr = fopen("errorout1.txt", "w");
            fprintf(fptr, "Error: program must end with period\n");
            fclose(fptr);
            exit(0);
        case 2:
            printf("Error: const, var, procedure, and read keywords must be followed by identifier\n"); 
            fptr = fopen("errorout2.txt", "w");
            fprintf(fptr, "Error: const, var, procedure, and read keywords must be followed by identifier\n");
            fclose(fptr);
            exit(0);
        case 3:
            printf("Error: symbol name has already been declared\n"); 
            fptr = fopen("errorout3.txt", "w");
            fprintf(fptr, "Error: symbol name has already been declared\n");
            fclose(fptr);
            exit(0);
        case 4:
            printf("Error: constants must be assigned with =\n"); 
            fptr = fopen("errorout4.txt", "w");
            fprintf(fptr, "Error: constants must be assigned with =\n");
            fclose(fptr);
            exit(0);
        case 5:
            printf("Error: constants must be assigned an integer value\n"); 
            fptr = fopen("errorout5.txt", "w");
            fprintf(fptr, "Error: constants must be assigned an integer value\n");
            fclose(fptr);
            exit(0);
        case 6:
            printf("Error: constant, variable, and procedure declarations must be followed by a semicolon\n"); 
            fptr = fopen("errorout6.txt", "w");
            fprintf(fptr, "Error: constant, variable, and procedure declarations must be followed by a semicolon\n");
            fclose(fptr);
            exit(0);
        case 7:
            printf("Error: undeclared identifier %s\n", global_tkn_list.names[global_tkn_list.current_index]); 
            fptr = fopen("errorout7.txt", "w");
            fprintf(fptr, "Error: Error: undeclared identifier\n");
            fclose(fptr);
            exit(0);
        case 8:
            printf("Error: only variable values may be altered\n"); 
            fptr = fopen("errorout8.txt", "w");
            fprintf(fptr, "Error: only variable values may be altered\n");
            fclose(fptr);
            exit(0);
        case 9:
            printf("Error: assignment statements must use :=\n"); 
            fptr = fopen("errorout9.txt", "w");
            fprintf(fptr, "Error: assignment statements must use :=\n");
            fclose(fptr);
            exit(0);
        case 10:
            printf("Error: begin must be followed by end\n"); 
            fptr = fopen("errorout10.txt", "w");
            fprintf(fptr, "Error: begin must be followed by end\n");
            fclose(fptr);
            exit(0);
        case 11:
            printf("Error: if must be followed by then\n"); 
            fptr = fopen("errorout11.txt", "w");
            fprintf(fptr, "Error: if must be followed by then\n");
            fclose(fptr);
            exit(0);
        case 12:
            printf("Error: while must be followed by do\n"); 
            fptr = fopen("errorout12.txt", "w");
            fprintf(fptr, "Error: while must be followed by do\n");
            fclose(fptr);
            exit(0);
        case 13:
            printf("Error: condition must contain comparison operator\n"); 
            fptr = fopen("errorout13.txt", "w");
            fprintf(fptr, "Error: condition must contain comparison operator\n");
            fclose(fptr);
            exit(0);
        case 14:
            printf("Error: right parenthesis must follow left parenthesis\n"); 
            fptr = fopen("errorout14.txt", "w");
            fprintf(fptr, "Error: right parenthesis must follow left parenthesis\n");
            fclose(fptr);
            exit(0);
        case 15:
            printf("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n"); 
            fptr = fopen("errorout15.txt", "w");
            fprintf(fptr, "Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
            fclose(fptr);
            exit(0);
        case 16:
            printf("Error: call must be followed by an identifier\n"); 
            fptr = fopen("errorout16.txt", "w");
            fprintf(fptr, "Error: call must be followed by an identifier\n");
            fclose(fptr);
            exit(0);
        case 17:
            printf("Error: variables and constants cannot be accessed using call\n"); 
            fptr = fopen("errorout17.txt", "w");
            fprintf(fptr, "Error: variables and constants cannot be accessed using call\n");
            fclose(fptr);
            exit(0);
        case 18:
            printf("Error: incorrect symbol during procedure declaration\n"); 
            fptr = fopen("errorout18.txt", "w");
            fprintf(fptr, "Error: incorrect symbol during procedure declaration\n");
            fclose(fptr);
            exit(0);
        case 19:
            printf("Error: procedure and const cannot be reassigned\n"); 
            fptr = fopen("errorout19.txt", "w");
            fprintf(fptr, "Error: procedure and const cannot be reassigned\n");
            fclose(fptr);
            exit(0);
        case 20:
            printf("Error: identifier is out of scope\n"); 
            fptr = fopen("errorout20.txt", "w");
            fprintf(fptr, "Error: identifier is out of scope\n");
            fclose(fptr);
            exit(0);
        default:
            printf("Error: unkown error type ???");
            exit(0);
    }
}

// Stores a new instruction to the assembly code array
void emit(int OP, int L, int M) {
    
    if(global_code.cx > MAX_SIZE)
        error(25);
    else
    {
        global_code.code[global_code.cx].OP = OP; //opcode
        global_code.code[global_code.cx].L = L; // lexicographical level
        global_code.code[global_code.cx].M = M; // modifier
        global_code.cx++;
        global_code.size++;
    }
}

void program(){
    // BLOCK
    // if token != periodsym
    //      error
    // emit HALT

    update_tokens(0);
    global_sym_table.size = 0;
    global_code.cx = 1;
    block();
    if (global_tkn_list.token != 19)
         error(1);
    emit(9, 0, 3);
}

void block(){
    // CONST-DECLARATION
    // numVars = VAR-DECLARATION
    // emit INC (M = 3 + numVars)
    // STATEMENT
        global_sym_table.current_level++;
    global_sym_table.declare = 1;
    int jmpaddr = global_code.cx;
    emit (7, 0, jmpaddr);
    const_declaration();
    int num_vars = var_declaration();
    procedure_declaration(jmpaddr);
    global_code.code[jmpaddr].M = (global_code.cx - 1) * 3;
    emit(6, 0, 3 + num_vars);
    global_sym_table.declare = 0;
    statement();
        for (int i=0; i<global_sym_table.size; i++) {
            if (global_sym_table.table[i].level == global_sym_table.current_level)
                global_sym_table.table[i].mark = 1;
        }
    global_sym_table.current_level--;
}

void const_declaration(){
    // if (token == 28)
    // {
    //     // do {
    //     //     get next token
    //     //     if token != identsym
    //             // error
    //     //     if symbol_table_check(token) != -1
    //             // error
    //     //     save ident name
    //     //     get next token
    //     //     if token != eqlsym
    //             // error
    //     //     get next token
    //     //     if token != numbersym
    //             // error
    //     //     add to symbol table (kind 1, saved name, number, 0, 0)
    //     //     get next token
    //     // } while (token == 17)
    //     // if ((get_next_token(tokens, size, next_index) == 18))
    //     //     error();
    //     // get next token
    // }
    
    if (global_tkn_list.token == 28)
    {
        do {
            update_tokens(get_next_token());
            if (global_tkn_list.token != 2)
                error(2);
            if (symbol_table_check() != -1)
                error(19);
            // save ident name
            strcpy(global_sym_table.table[global_sym_table.size].name, global_tkn_list.names[global_tkn_list.current_index]);
            update_tokens(get_next_token());
            if (global_tkn_list.token != 9)
                error(4);
            update_tokens(get_next_token());
            if (global_tkn_list.token!= 3)
                error(5);
            // add to symbol table (kind 1, value, L, M, mark)
            global_sym_table.table[global_sym_table.size].kind = 1;
            global_sym_table.table[global_sym_table.size].val = global_tkn_list.nums[global_tkn_list.num_count];
            global_sym_table.table[global_sym_table.size].level = 0; 
            global_sym_table.table[global_sym_table.size].addr = 0;
            global_sym_table.table[global_sym_table.size].mark = 0;
            global_sym_table.size++;
            global_tkn_list.num_count++;
            update_tokens(get_next_token());
        } while (global_tkn_list.token == 17);
        if (global_tkn_list.token!= 18)
            error(6);
        update_tokens(get_next_token());
    }
}

int var_declaration(){
    // if (token == varsym) {
    //     do {
    //     num_vars++
    //     get next token
    //     if token != identsym
    //         error
    //     if symbol_table_check (token) != -1
    //         error
    //     add to symbol table (kind 2, ident, 0, 0, var# + 2)
    //     get next token
    //     } while (token == commasym)
    //     if (token != semicolonsym)
    //         error
    //     get next token
    // }
    
    int num_vars = 0;
    if (global_tkn_list.token == 29) {
        int space = 3;
        do {
        num_vars++;
        update_tokens(get_next_token());
        if (global_tkn_list.token != 2)
            error(2);
        if (symbol_table_check() != -1)
            error(3);
        // add to symbol table (kind 2, name, 0, L, M, mark)
        global_sym_table.table[global_sym_table.size].kind = 2;
        strcpy(global_sym_table.table[global_sym_table.size].name, global_tkn_list.names[global_tkn_list.current_index]);
        global_sym_table.table[global_sym_table.size].val = 0;
        global_sym_table.table[global_sym_table.size].level = global_sym_table.current_level;
        global_sym_table.table[global_sym_table.size].addr = space;
        global_sym_table.table[global_sym_table.size].mark = 0;
        global_sym_table.size++;
        global_sym_table.symIdx++;
        space++;
        update_tokens(get_next_token());
        } while (global_tkn_list.token == 17);
        if (global_tkn_list.token != 18)
            error(6);
        update_tokens(get_next_token());
    }
    return num_vars;
}

void procedure_declaration(int jmpaddr){
    //  {"procedure" ident ";" block ";"}
    while (global_tkn_list.token == 30) {       // "procedure"
        update_tokens(get_next_token());  
        if (global_tkn_list.token != 2)         // ident
            error(2);
        if (symbol_table_check() != -1)         // Check if procedure has been declared already
            error(19);
        // add to symbol table (kind 3, ident, 0, 0, var# + 2)
        global_sym_table.table[global_sym_table.size].kind = 3;
        strcpy(global_sym_table.table[global_sym_table.size].name, global_tkn_list.names[global_tkn_list.current_index]);
        global_sym_table.table[global_sym_table.size].val = 0;
        global_sym_table.table[global_sym_table.size].level = global_sym_table.current_level;
        global_sym_table.table[global_sym_table.size].addr = jmpaddr * 3;
        global_sym_table.table[global_sym_table.size].mark = 0;
        global_sym_table.procIdx = global_sym_table.size;
        global_sym_table.size++;
        global_sym_table.symIdx++; 
        update_tokens(get_next_token());  
        if (global_tkn_list.token != 18)        // ";"
            error(18);                           
        update_tokens(get_next_token());
        block(); 
        emit(2, 0, 0);                          // block
        if (global_tkn_list.token != 18)        // ";"
            error(6);                         
        update_tokens(get_next_token()); 
    }
}

void statement(){
    // if (token == identsym) {
    //     symIdx = symbol_table_check (token)
    //     if (symIdx == -1)
    //         error
    //     if (table[symIdx].kind != 2 (not a var))
    //         error
    //     get next token
    //     if (token != becomessym)
    //         error
    //     get next token
    //     expression();
    //     emit STO (M = table[symIdx].addr)
    //     return
    // }
    // if (token == beginsym) {
    //     do {
    //     get next token
    //     statement();
    //     } while token == semicolonsym
    //     if (token != endsym)
    //         error
    //     get next token
    //     return
    // }
    // if (token == ifsym) {
    //     get next token
    //     condition();
    //     jpcIdx = current code index
    //     emit JPC
    //     if (token != thensym)
    //         error
    //     get next token
    //     statement();
    //     code[jpcIdx].M = current code index
    //     return
    // }
    // if (token == whilesym) {
    //     get next token
    //     loopIdx = current code index
    //     condition();
    //     if (token != dosym)
    //         error
    //     get next token
    //     jpcIdx = current code index
    //     emit JPC
    //     statement();
    //     emit JMP (M = loopIdx)
    //     code[jpcIdx].M = current code index
    //     return
    // }
    // if (token == readsym) {
    //     get next token
    //     if (token != identsym)
    //         error
    //     symIdx = symbol_table_check (token)
    //     if (symIdx == -1)
    //         error
    //     if (table[symIdx].kind != 2 (not a var))
    //         error
    //     get next token
    //     emit READ
    //     emit STO (M = table[symIdx].addr)
    //     return
    // }
    // if (token == writesym) {
    //     get next token
    //     expression();
    //     emit WRITE
    //     return
    // }

    if (global_tkn_list.token == 2) {
        global_sym_table.symIdx = symbol_table_check();
        if (global_sym_table.symIdx == -1)
            error(7);
        if (global_sym_table.table[global_sym_table.symIdx].kind != 2)
            error(8);
        update_tokens(get_next_token());
        if (global_tkn_list.token != 20)
            error(9);
        update_tokens(get_next_token());
        expression(); 
        emit(4, global_sym_table.current_level - global_sym_table.table[global_sym_table.symIdx].level, global_sym_table.table[global_sym_table.symIdx].addr);
        return;
    }
    if (global_tkn_list.token == 27) {
        update_tokens(get_next_token());
        if (global_tkn_list.token != 2)
            error(16); 
        global_sym_table.symIdx = symbol_table_check();
        if (global_sym_table.symIdx == -1)
            error(7);
        if (global_sym_table.table[global_sym_table.symIdx].kind != 3)
            error(17); 
        emit(5, global_sym_table.current_level-global_sym_table.table[global_sym_table.symIdx].level, global_sym_table.table[global_sym_table.symIdx].addr);
        update_tokens(get_next_token());
        return;

    }
    if (global_tkn_list.token == 21) {
        do {
            update_tokens(get_next_token());
            statement();
        } while (global_tkn_list.token == 18);
        if (global_tkn_list.token != 22)
            error(10);
        update_tokens(get_next_token());
        return;
    }
    if (global_tkn_list.token == 23) {
        update_tokens(get_next_token());
        condition();
        int jpcIdx = global_code.cx;
        emit(8, 0, jpcIdx);
        if (global_tkn_list.token != 24)
            error(11);
        update_tokens(get_next_token());
        statement();
        global_code.code[jpcIdx].M = 3 * (global_code.cx - 1);
        return;
    }
    if (global_tkn_list.token == 25) {
        update_tokens(get_next_token());
        int loopIdx = 3 * global_code.cx;
        condition();
        if (global_tkn_list.token != 26)
            error(12);
        update_tokens(get_next_token());
        int jpcIdx = global_code.cx;
        emit(8, 0, jpcIdx);
        statement();
        emit(7, 0, loopIdx);
        global_code.code[jpcIdx].M = 3 * (global_code.cx - 1);
        return;
    }
    if (global_tkn_list.token == 32) {
        update_tokens(get_next_token());
        if (global_tkn_list.token != 2)
            error(2);
        global_sym_table.symIdx = symbol_table_check();
        if (global_sym_table.symIdx == -1)
            error(7);
        if (global_sym_table.table[global_sym_table.symIdx].kind != 2)
            error(8);
        update_tokens(get_next_token());
        emit(9, 0, 2); 
        emit(4, global_sym_table.current_level, global_sym_table.table[global_sym_table.symIdx].addr);
        return;
    }
    if (global_tkn_list.token == 31) {
        update_tokens(get_next_token());
        expression();
        emit(9, 0, 1);
        return;
    }
}

void condition(){
    // if (token == oddsym){ 
    //     get next token
    //     expression();
    //     emit ODD
    // }
    // else {
    //     EXPRESSION
    //     if (token == eqlsym) {
    //         get next token
    //         expression();
    //         emit EQL
    //     }
    //     else if (token == neqsym) {
    //         get next token
    //         expression();
    //         emit NEQ
    //     }
    //     else if (token == lessym) {
    //         get next token
    //         expression();
    //         emit LSS
    //     }
    //     else if (token == leqsym) {
    //         get next token
    //         expression();
    //         emit LEQ
    //     }
    //     else if (token == gtrsym) {
    //         get next token
    //         expression();
    //         emit GTR
    //     }
    //     else if (token == geqsym) {
    //         get next token
    //         expression();
    //         emit GEQ
    //     }
    //     else
    //         error
    // 
    // }

    if (global_tkn_list.token == 1){ 
        update_tokens(get_next_token());
        expression();
        emit(2, 0, 11);
    }
    else {
        expression();
        if (global_tkn_list.token == 9) {
            update_tokens(get_next_token());
            expression();
            emit(2, 0, 5);
        }
        else if (global_tkn_list.token == 10) {
            update_tokens(get_next_token());
            expression();
            emit(2, 0, 6);
        }
        else if (global_tkn_list.token == 11) {
            update_tokens(get_next_token());
            expression();
            emit(2, 0, 7);
        }
        else if (global_tkn_list.token == 12) {
            update_tokens(get_next_token());
            expression();
            emit(2, 0, 8);
        }
        else if (global_tkn_list.token == 13) {
            update_tokens(get_next_token());
            expression();
            emit(2, 0, 9);
        }
        else if (global_tkn_list.token == 14) {
            update_tokens(get_next_token());
            expression();
            emit(2, 0, 10);
        }
        else
            error(13);
    
    }
}

void expression(){
    // if (token == minussym) {
    //     get next token
    //     term();
    //     emit NEG
    //     while (token == plussym || token == minussym) {
    //         if (token == plussym) {
    //             get next token
    //             term();
    //             emit ADD
    //         }
    //         else {
    //             get next token
    //             term();
    //             emit SUB
    //         }
    //     }
    // }
    // else {
    //     if (token == plussym)
    //         get next token
    //     term();
    //     while (token == plussym || token == minussym)
    //     {
    //         if (token == plussym) {
    //             get next token
    //             term();
    //             emit ADD
    //         }
    //         else {
    //             get next token
    //             term();
    //             emit SUB
    //         }
    //     }
    // }
    
    if (global_tkn_list.token == 5) {
        update_tokens(get_next_token());
        term();
        while (global_tkn_list.token == 4 || global_tkn_list.token == 5) {
            if (global_tkn_list.token == 4) {
                update_tokens(get_next_token());
                term();
                emit(2, 0, 1);
            }
            else {
                update_tokens(get_next_token());
                term();
                emit(2, 0, 2);
            }
        }
    }
    else {
        if (global_tkn_list.token == 4)
            update_tokens(get_next_token());
        term();
        while (global_tkn_list.token == 4 || global_tkn_list.token == 5)
        {
            if (global_tkn_list.token == 4) {
                update_tokens(get_next_token());
                term();
                emit(2, 0, 1);
            }
            else {
                update_tokens(get_next_token());
                term();
                emit(2, 0, 2);
            }
        }
    }
}

void term(){
    // factor();
    // while (token == multsym || token == slashsym || token == modsym) {
    //     if (token == multsym) {
    //         get next token
    //         factor();
    //         emit MUL
    //     }
    //     else if (token == slashsym) {
    //         get next token
    //         factor();
    //         emit DIV
    //     }
    //     else {
    //         get next token
    //         factor();
    //         emit MOD
    //     }
    // }        
    factor();
    while (global_tkn_list.token == 6 || global_tkn_list.token == 7) {
        if (global_tkn_list.token == 6) {
            update_tokens(get_next_token());
            factor();
            emit(2, 0, 3);
        }
        else if (global_tkn_list.token == 7) {
            update_tokens(get_next_token());
            factor();
            emit(2, 0, 4);
        }
        else {
            update_tokens(get_next_token());
            factor();
            emit(2, 0, 7);
        }
    }
}

void factor(){ 
    // if token == identsym
    //      symIdx = SYMBOLTABLECHECK (token)
    //      if symIdx == -1
    //          error
    //      if table[symIdx].kind == 1 (const)
    //          emit LIT (M = table[symIdx].Value)
    //      else (var)
    //          emit LOD (M = table[symIdx].addr)
    //      get next token
    // else if token == numbersym
    //      emit LIT
    //      get next token
    // else if token == lparentsym
    //      get next token
    //      EXPRESSION
    //      if token != rparentsym
    //          error
    //      get next token
    // else
    //      error

    
    if (global_tkn_list.token == 2) { 
        int temp_idx = symbol_table_check();
        if (temp_idx == -1)
            error(7);
        if (global_sym_table.table[temp_idx].kind == 1){
            emit(1, 0, global_sym_table.table[temp_idx].val);
        }
        else
            emit(3, global_sym_table.current_level - global_sym_table.table[temp_idx].level, global_sym_table.table[temp_idx].addr);
        update_tokens(get_next_token());
    }
    else if (global_tkn_list.token == 3) {
        emit(1, 0, global_tkn_list.nums[global_tkn_list.num_count]);
        global_tkn_list.num_count++;
        update_tokens(get_next_token());
    }
    else if (global_tkn_list.token == 15) {
        update_tokens(get_next_token());
        expression();
        if (global_tkn_list.token != 16)
            error(14);
        update_tokens(get_next_token());
    }
    else
        error(15);
}