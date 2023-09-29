/**
 * Project 2
 * LC-2K Linker
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE 500
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
    char label[7];
    char location;
    unsigned int offset;
};

struct RelocationTableEntry {
    unsigned int offset;
    char inst[7];
    char label[7];
    unsigned int file;
};

struct FileData {
    unsigned int textSize;
    unsigned int dataSize;
    unsigned int symbolTableSize;
    unsigned int relocationTableSize;
    unsigned int textStartingLine;  // in final executable
    unsigned int dataStartingLine;  // in final executable
    int text[MAXSIZE];
    int data[MAXSIZE];
    SymbolTableEntry symbolTable[MAXSIZE];
    RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
    unsigned int textSize;
    unsigned int dataSize;
    unsigned int symbolTableSize;
    unsigned int relocationTableSize;
    int text[MAXSIZE * MAXFILES];
    int data[MAXSIZE * MAXFILES];
    SymbolTableEntry symbolTable[MAXSIZE * MAXFILES];
    RelocationTableEntry relocTable[MAXSIZE * MAXFILES];
};

int main(int argc, char* argv[]) {
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    unsigned int i, j;

    if (argc <= 2) {
        printf("error: usage: %s <obj file> ... <output-exe-file>\n",
               argv[0]);
        exit(1);
    }

    outFileString = argv[argc - 1];

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    FileData files[MAXFILES];

    // read in all files and combine into a "master" file
    for (i = 0; i < argc - 2; i++) {
        inFileString = argv[i + 1];

        inFilePtr = fopen(inFileString, "r");
        printf("opening %s\n", inFileString);

        if (inFilePtr == NULL) {
            printf("error in opening %s\n", inFileString);
            exit(1);
        }

        char line[MAXLINELENGTH];
        unsigned int textSize, dataSize, symbolTableSize, relocationTableSize;

        // parse first line of file
        fgets(line, MAXSIZE, inFilePtr);
        sscanf(line, "%d %d %d %d",
               &textSize, &dataSize, &symbolTableSize, &relocationTableSize);

        files[i].textSize = textSize;
        files[i].dataSize = dataSize;
        files[i].symbolTableSize = symbolTableSize;
        files[i].relocationTableSize = relocationTableSize;

        // read in text section
        int instr;
        for (j = 0; j < textSize; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            instr = strtol(line, NULL, 0);
            files[i].text[j] = instr;
        }

        // read in data section
        int data;
        for (j = 0; j < dataSize; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            data = strtol(line, NULL, 0);
            files[i].data[j] = data;
        }

        // read in the symbol table
        char label[7];
        char type;
        unsigned int addr;
        for (j = 0; j < symbolTableSize; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            sscanf(line, "%s %c %d",
                   label, &type, &addr);
            files[i].symbolTable[j].offset = addr;
            strcpy(files[i].symbolTable[j].label, label);
            files[i].symbolTable[j].location = type;
        }

        // read in relocation table
        char opcode[7];
        for (j = 0; j < relocationTableSize; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            sscanf(line, "%d %s %s",
                   &addr, opcode, label);
            files[i].relocTable[j].offset = addr;
            strcpy(files[i].relocTable[j].inst, opcode);
            strcpy(files[i].relocTable[j].label, label);
            files[i].relocTable[j].file = i;
        }
        fclose(inFilePtr);
    }  // end reading files

    // *** INSERT YOUR CODE BELOW ***
    //    Begin the linking process
    //    Happy coding!!!

}  // main
