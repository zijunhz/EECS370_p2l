/**
 * Project 2
 * LC-2K Linker
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE 500
#define MAXLINELENGTH 1010
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

int32_t isLocalLabel(const char* label);
int32_t isGlobalLabel(const char* label);
int32_t hasLabel(const char* label);
uint32_t findLabel(FileData* files, uint32_t fileId, uint32_t relocId, uint32_t inputFileCnt);
int32_t convertNum(int32_t num);

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

    // check duplicate defined global labels, and check Stack defined by obj files
    uint32_t inputFileNum = argc - 2;
    for (uint32_t i = 0; i < inputFileNum; ++i)
        for (uint32_t j = 0; j < files[i].symbolTableSize; ++j) {
            if (files[i].symbolTable[j].location != 'U')
                continue;
            if (!strcmp(files[i].symbolTable[j].label, "Stack"))
                exit(1);
            for (uint32_t ii = 0; ii < i; ii++)
                for (uint32_t jj = 0; jj < files[ii].symbolTableSize; ++jj)
                    if ((files[ii].symbolTable[jj].location != 'U') &&
                        (!strcmp(files[ii].symbolTable[jj].label, files[i].symbolTable[j].label)))
                        exit(1);
            for (uint32_t jj = 0; jj < j; ++jj)
                if ((files[i].symbolTable[jj].location != 'U') &&
                    (!strcmp(files[i].symbolTable[jj].label, files[i].symbolTable[j].label)))
                    exit(1);
        }

    // reloc address

    for (uint32_t i = 0; i < inputFileNum; ++i)
        for (uint32_t j = 0; j < files[i].relocationTableSize; ++j) {
            RelocationTableEntry* relocEntry = &(files[i].relocTable[j]);
            uint32_t addr = findLabel(files, i, j, inputFileNum);
            printf("%s %d\n", relocEntry->label, addr);
            int32_t* targetMachineCode = NULL;
            if (!strcmp(relocEntry->inst, ".fill")) {
                targetMachineCode = &(files[i].data[relocEntry->offset]);
            } else {
                targetMachineCode = &(files[i].text[relocEntry->offset]);
            }
            (*targetMachineCode) = ((*targetMachineCode) & ((1 << 27) - (1 << 16))) | (addr & ((1 << 16) - 1));
        }

    // output

    for (uint32_t i = 0; i < inputFileNum; ++i)
        for (uint32_t j = 0; j < files[i].textSize; ++j)
            fprintf(outFilePtr, "%d\n", files[i].text[j]);
    for (uint32_t i = 0; i < inputFileNum; ++i)
        for (uint32_t j = 0; j < files[i].dataSize; ++j)
            fprintf(outFilePtr, "%d\n", files[i].data[j]);

    return 0;

}  // main

int32_t hasLabel(const char* label) {
    return strcmp(label, "") && strcmp(label, " ");
}

int32_t isLocalLabel(const char* label) {
    return hasLabel(label) && islower(label[0]);
}

int32_t isGlobalLabel(const char* label) {
    return hasLabel(label) && isupper(label[0]);
}

uint32_t findLabel(FileData* files, uint32_t fileId, uint32_t relocId, uint32_t inputFileCnt) {
    RelocationTableEntry* relocEntry = &(files[fileId].relocTable[relocId]);
    char* label = relocEntry->label;
    uint32_t addr = 0;
    if (!strcmp(label, "Stack")) {
        for (uint32_t i = 0; i < inputFileCnt; ++i)
            addr += files[i].dataSize + files[i].textSize;
        return addr & ((1 << 16) - 1);
    }
    if (isLocalLabel(label)) {
        int32_t absLoc = 0;
        int32_t machineCode = 0;
        if (!strcmp(relocEntry->inst, ".fill")) {
            machineCode = files[fileId].data[relocEntry->offset];
        } else {
            machineCode = files[fileId].text[relocEntry->offset];
        }
        absLoc = convertNum(machineCode & ((1 << 16) - 1));

        if (absLoc >= files[fileId].textSize) {
            for (uint32_t i = 0; i < inputFileCnt; ++i)
                addr += files[i].textSize;
            addr -= files[fileId].textSize;
            for (uint32_t i = 0; i < fileId; ++i)
                addr += files[i].dataSize;
        } else {
            for (uint32_t i = 0; i < fileId; ++i)
                addr += files[i].textSize;
        }

        addr += convertNum(machineCode & ((1 << 16) - 1));
        return addr & ((1 << 16) - 1);
    }
    // is global label
    uint32_t targetFileId = 0, targetSymbolId = 0;
    uint8_t isFound = 0;
    for (uint32_t i = 0; i < inputFileCnt && (!isFound); ++i)
        for (uint32_t j = 0; j < files[i].symbolTableSize; ++j)
            if ((files[i].symbolTable[j].location != 'U') &&
                (!strcmp(label, files[i].symbolTable[j].label))) {
                isFound = 1;
                targetFileId = i;
                targetSymbolId = j;
                break;
            }
    if (!isFound)
        exit(1);
    if (files[targetFileId].symbolTable[targetSymbolId].location == 'D') {
        for (uint32_t i = 0; i < inputFileCnt; ++i)
            addr += files[i].textSize;
        for (uint32_t i = 0; i < targetFileId; ++i)
            addr += files[i].dataSize;
    } else {
        for (uint32_t i = 0; i < targetFileId; ++i)
            addr += files[i].textSize;
    }
    addr += files[targetFileId].symbolTable[targetSymbolId].offset;

    return addr & ((1 << 16) - 1);
}

int32_t convertNum(int32_t num) {
    return num - ((num & (1 << 15)) ? 1 << 16 : 0);
}
