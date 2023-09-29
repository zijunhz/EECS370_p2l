/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

enum Operator {
    ADD,
    NOR,
    LW,
    SW,
    BEQ,
    JALR,
    HALT,
    NOOP,
    FILL,
    UNKNOWN
};

const char opName[9][9] = {"add", "nor", "lw", "sw", "beq", "jalr", "halt", "noop", ".fill"};

enum Operator c2o(const char* c);

int readAndParse(FILE*, char*, char*, char*, char*, char*);
static inline int isNumber(char*);
static inline int8_t checkReg(char* s);
int32_t everything2dec(const char* op, const char* arg0, const char* arg1, const char* arg2);
int32_t findTargetLine(const char* label, const char labels[MAXLINELENGTH + 10][MAXLINELENGTH + 10], int lineCnt);
int32_t isLocalLabel(const char* label);
int32_t isGlobalLabel(const char* label);
int32_t hasLabel(const char* label);
char* hasSymbolicAddress(enum Operator opc, char* arg0, char* arg2);
char* hasGlobalSymbolicAddress(enum Operator opc, char* arg0, char* arg2);
int32_t checkStrSameStart(const char* str1, const char* str2);

int main(int argc, char** argv) {
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH + 10], opcode[MAXLINELENGTH + 10], arg0[MAXLINELENGTH + 10],
        arg1[MAXLINELENGTH + 10], arg2[MAXLINELENGTH + 10];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    char labels[MAXLINELENGTH + 10][MAXLINELENGTH + 10] = {0}, opcodes[MAXLINELENGTH + 10][MAXLINELENGTH + 10] = {0}, arg0s[MAXLINELENGTH + 10][MAXLINELENGTH + 10] = {0}, arg1s[MAXLINELENGTH + 10][MAXLINELENGTH + 10] = {0}, arg2s[MAXLINELENGTH + 10][MAXLINELENGTH + 10] = {0};

    // read in from file to arrs
    uint32_t lineCnt = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        uint32_t i = lineCnt;
        strcpy(labels[i], label);
        strcpy(opcodes[i], opcode);
        strcpy(arg0s[i], arg0);
        strcpy(arg1s[i], arg1);
        strcpy(arg2s[i], arg2);
        lineCnt++;
    }
    lineCnt--;

    // handle comments, check reg
    for (uint32_t i = 0; i <= lineCnt; i++) {
        enum Operator opc = c2o(opcodes[i]);
        switch (opc) {
            case UNKNOWN:
                exit(1);
            case ADD:
            case NOR:
                if ((!checkReg(arg0s[i])) || (!checkReg(arg1s[i])) || (!checkReg(arg2s[i])))
                    exit(1);
                break;
            case LW:
            case SW:
            case BEQ:
                if ((!checkReg(arg0s[i])) || (!checkReg(arg1s[i])))
                    exit(1);
                break;
            case JALR:
                if ((!checkReg(arg0s[i])) || (!checkReg(arg1s[i])))
                    exit(1);
                strcpy(arg2s[i], "0");
                break;
            case NOOP:
            case HALT:
                strcpy(arg0s[i], "0");
                strcpy(arg1s[i], "0");
                strcpy(arg2s[i], "0");
                break;
            case FILL:
                strcpy(arg1s[i], "0");
                strcpy(arg2s[i], "0");
                break;
            default:
                break;
        }
    }

    char reloc[MAXLINELENGTH + 10][MAXLINELENGTH + 10] = {0};
    char symbolic[MAXLINELENGTH + 10][MAXLINELENGTH + 10] = {0};

    int32_t tCnt = 0, dCnt = 0, sCnt = 0, rCnt = 0;
    for (uint32_t i = 0; i <= lineCnt; i++) {
        enum Operator opc = c2o(opcodes[i]);
        if (opc == UNKNOWN)
            exit(1);
        // find duplicate label
        if (hasLabel(labels[i]))
            for (uint32_t j = 0; j < i; j++)
                if (!strcmp(labels[i], labels[j]))
                    exit(1);
        if (opc == FILL)
            dCnt++;
        else
            tCnt++;
        // deal with symbolic table append due to defined label
        if (isGlobalLabel(labels[i])) {
            sCnt++;
            sprintf(symbolic[sCnt], "%s %c %d", labels[i], opc == FILL ? 'D' : 'T', opc == FILL ? i - tCnt : i);
        }
        // deal with lines that has symbolic address
        if (hasSymbolicAddress(opc, arg0s[i], arg2s[i])) {
            char* theAddress = hasSymbolicAddress(opc, arg0s[i], arg2s[i]);
            if (opc != BEQ) {  // opc == lw/sw/fill, then add to relocation table
                rCnt++;
                sprintf(reloc[rCnt], "%d %s %s", opc == FILL ? i - tCnt : i, opName[opc], theAddress);
            }
            int32_t targetLine = findTargetLine(theAddress, labels, lineCnt);
            if (targetLine == -1) {
                if (isLocalLabel(theAddress))
                    exit(1);
                if (opc == BEQ)
                    exit(1);
                int8_t alreadyExist = 0;
                for (int32_t j = 1; j <= sCnt; j++) {
                    if (checkStrSameStart(theAddress, symbolic[j])) {
                        alreadyExist = 1;
                        break;
                    }
                }
                if (!alreadyExist) {
                    sCnt++;
                    sprintf(symbolic[sCnt], "%s U 0", theAddress);
                }
                strcpy(theAddress, "0");
            } else {
                switch (opc) {
                    case LW:
                    case SW:
                        sprintf(arg2s[i], "%d", targetLine);
                        break;
                    case BEQ:
                        sprintf(arg2s[i], "%d", targetLine - i - 1);
                        break;
                    case FILL:
                        sprintf(arg0s[i], "%d", targetLine);
                        break;
                    default:
                        break;
                }
            }
        }
        if ((opc == LW || opc == SW || opc == BEQ) && (atoi(arg2s[i]) < -32768 || atoi(arg2s[i]) > 32767)) {
            printf("Error: unsupported offsets %s\n", arg2s[i]);
            exit(1);
        }
    }

    // print output

    fprintf(outFilePtr, "%d %d %d %d\n", tCnt, dCnt, sCnt, rCnt);

    for (uint32_t i = 0; i <= lineCnt; i++)
        fprintf(outFilePtr, "%d\n", everything2dec(opcodes[i], arg0s[i], arg1s[i], arg2s[i]));

    // print symbolic table

    for (int32_t i = 1; i <= sCnt; i++) {
        fprintf(outFilePtr, "%s\n", symbolic[i]);
    }

    // print reloc

    for (int32_t i = 1; i <= rCnt; i++) {
        fprintf(outFilePtr, "%s\n", reloc[i]);
    }

    // /* here is an example for how to use readAndParse to read a line from
    //     inFilePtr */
    // if (!readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
    //     /* reached end of file */
    // }

    // /* this is how to rewind the file ptr so that you start reading from the
    //     beginning of the file */
    // rewind(inFilePtr);

    // /* after doing a readAndParse, you may want to do the following to test the
    //     opcode */
    // if (!strcmp(opcode, "add")) {
    //     /* do whatever you need to do for opcode "add" */
    // }
    return (0);
}

/*
 * NOTE: The code defined below is not to be modifed as it is implimented correctly.
 */

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE* inFilePtr, char* label, char* opcode, char* arg0, char* arg1, char* arg2) {
    char line[MAXLINELENGTH + 10];
    char* ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH + 10, inFilePtr) == NULL) {
        /* reached end of file */
        return (0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH + 10 - 1) {
        printf("error: line too long\n");
        exit(1);
    }

    // Treat a blank line as end of file.
    // Arguably, we could just ignore and continue, but that could
    // get messy in terms of label line numbers etc.
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for (size_t line_idx = 0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for (int whitespace_idx = 0; whitespace_idx < 4; ++whitespace_idx) {
            if (line[line_idx] == whitespace[whitespace_idx]) {
                ++line_char_is_whitespace;
                break;
            }
        }
        if (!line_char_is_whitespace) {
            ++nonempty_line;
            break;
        }
    }
    if (nonempty_line == 0) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);

    return (1);
}

static inline int
isNumber(char* string) {
    int num;
    char c;
    return ((sscanf(string, "%d%c", &num, &c)) == 1);
}

enum Operator c2o(const char* c) {
    enum Operator ops[9] = {ADD, NOR, LW, SW, BEQ, JALR, HALT, NOOP, FILL};
    for (uint8_t i = 0; i < 9; i++) {
        if (!strcmp(opName[i], c))
            return ops[i];
    }
    return UNKNOWN;
}

static inline int8_t checkReg(char* s) {
    if (!isNumber(s))
        exit(1);
    int32_t num = atoi(s);
    return num >= 0 && num <= 7;
}

int32_t everything2dec(const char* op, const char* arg0, const char* arg1, const char* arg2) {
    // const uint8_t SHIFT[5] = {0, 22, 19, 16, 0};
    // opcode, arg0, arg1, arg2
    enum Operator opp = c2o(op);
    if (opp == FILL) {
        return atoi(arg0);
    }
    return ((opp << 22) & ((1 << 25) - (1 << 22))) + ((atoi(arg0) << 19) & ((1 << 22) - (1 << 19))) + ((atoi(arg1) << 16) & ((1 << 19) - (1 << 16))) + ((atoi(arg2)) & ((1 << 16) - 1));
}

int32_t findTargetLine(const char* label, const char labels[MAXLINELENGTH + 10][MAXLINELENGTH + 10], int lineCnt) {
    for (int k = 0; k <= lineCnt; k++)
        if (!strcmp(labels[k], label)) {
            return k;
        }
    return -1;
}

int32_t hasLabel(const char* label) {
    return strcmp(label, "") && strcmp(label, " ");
}

int32_t isLocalLabel(const char* label) {
    return hasLabel(label) && islower(label[0]);
}

int32_t isGlobalLabel(const char* label) {
    return hasLabel(label) && isupper(label[0]);
}

char* hasSymbolicAddress(enum Operator opc, char* arg0, char* arg2) {
    char* theAddress = NULL;
    if (opc == LW || opc == SW || opc == BEQ)
        theAddress = arg2;
    else if (opc == FILL)
        theAddress = arg0;
    else
        return NULL;
    return (!isNumber(theAddress)) ? theAddress : NULL;
}

char* hasGlobalSymbolicAddress(enum Operator opc, char* arg0, char* arg2) {
    char* theAddress = hasSymbolicAddress(opc, arg0, arg2);
    return (theAddress && isGlobalLabel(theAddress)) ? theAddress : NULL;
}

int32_t checkStrSameStart(const char* str1, const char* str2) {
    for (int32_t i = 0; str1[i] != '\0' && str2[i] != '\0' && str1[i] != ' ' && str2[i] != ' '; i++)
        if (str1[i] != str2[i])
            return 0;
    return 1;
}
