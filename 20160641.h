#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "checks.h"
#include <stdbool.h>

typedef struct _HistoryNode{    // command history node for linked list
    char command[10];
    struct _HistoryNode* next;
} HistoryNode;
HistoryNode* historyHead = NULL, *historyTail = NULL;

typedef struct _OpcodeNode{ // OpcodeNode for hashtable
    int opcode;
    char mnemonic[10];
    char format[5];
    struct _OpcodeNode* next;
} OpcodeNode;
OpcodeNode* hashtable[20];  // hashtable for opcode

typedef struct _SymbolNode{
    char symbol[30];
    int locCtr;
    struct _SymbolNode* next;
}SymbolNode;    // SymbolNode for symboltable
SymbolNode* symboltable[26];

typedef struct _ExternNode{
    char progName[7];
    char symName[7];
    int address;
    int length;
    struct _ExternNode* next;
}ExternNode;    // ExternNode for externtable
ExternNode* externtable[26];

int last_address;       // for dump function
unsigned short int memory[1048576];    // 1MB(16 * 65536) memory
unsigned int progAddr; // program load address
bool bp[4600];    // break point array
short int bpSize = 0;   // break point size
int reg[10] = {0, };    // register A:0, X:1, L:2, PC:8, SW:9 B:3, S:4,T:5, F:6
int lastExecAddr;   // last executed address
bool bpFlag = false;
int fileLen;
/* function : getCommand() */
/* purpose : get command from user */
/* return : parameter from input string */
char* getCommand(char input[], int* command){
    
    char* check;
    char tmp;

    while( !(*input == EOF || *input == '\0') && (*input == ' ' || *input == '\t' )){
        input++;
    }   // now, input points first character that is not space or tab

    check = input;  // 'check' is the command string to be checked which is valid or not

    while(!(*input == EOF || *input == '\0') && !(*input == ' ' || *input == '\t')){
        input++;
    }   
    tmp = *input;
    *input = '\0';

    switch(*check){
        // check 'check' is valid command or not
        case 'h' : 
            if(strcmp(check, "help") == 0 || strcmp(check, "h") ==0){
                *command = 1;
            }else if(strcmp(check, "hi")==0 || strcmp(check, "history") == 0){
                *command = 4;
            }else *command = -1;
            break;

        case 'd' :
            if(strcmp(check, "d")==0 || strcmp(check, "dir") == 0){
                *command = 2;
            }else if(strcmp(check, "du")==0 || strcmp(check, "dump") ==0 ){
                *command = 5;
            }else *command = -1;
            break;

        case 'q' :
            if(strcmp(check, "q")==0 || strcmp(check, "quit") == 0 ){
                *command = 3;
            }else *command = -1;
            break;

        case 'e' :
            if(strcmp(check, "e")==0 || strcmp(check, "edit") ==0){
                *command = 6;
            }else *command = -1;
            break;

        case 'f' :
            if(strcmp(check, "f") ==0 || strcmp(check, "fill") ==0 ){
                *command = 7;
            }else *command = -1;
            break;
        
        case 'r' :
            if(strcmp(check, "reset") ==0){
                *command = 8;
            }else if(strcmp(check, "run") == 0){
                *command = 17;
            }else *command = -1;
            break;
    
        case 'o' :
            if(strcmp(check, "opcode")==0 ){
                *command = 9;
            }else if(strcmp(check, "opcodelist") == 0){
                *command = 10;
            }else *command = -1;
            break;
        case 't' :
            if(strcmp(check, "type")==0){
                *command = 11;
            }else *command = -1;
            break;
        case 'a' :
            if(strcmp(check, "assemble") == 0){
                *command = 12;
            }else *command = -1;
            break;
        case 's' :
            if(strcmp(check, "symbol") == 0){
                *command = 13;
            }else *command = -1;
            break;
        case 'p' :
            if(strcmp(check, "progaddr") == 0){
                *command = 14;
            }else *command = -1;
            break;
        case 'l' :
            if(strcmp(check, "loader") == 0){
                *command = 15;
            }else *command = -1;
            break;
        case 'b' :
            if(strcmp(check, "bp") == 0){
                *command = 16;
            }else *command = -1;
            break;
        default :   // invalid command
            *command = -1;   
            break; 
    }

    *input = tmp; 

    while(!(*input == EOF || *input == '\0')&& (*input == ' ' || *input == '\t')){
        input++;
    } // now, input points parameter string(if there is) or EOF or '\0'

    return input;   // return parameter string
}

/* function : recordHistory() */
/* purpose : record command history to the HistoryNode and link the list*/
/* return : nothing */
void recordHistory(char command[]){
    char tmp[100];
    int i,j=0, paramflag = 0;
    HistoryNode* newNode = (HistoryNode*)malloc(sizeof(HistoryNode));   // allocate new HistoryNode
    newNode->next = NULL;
    
    for(i=0; i<strlen(command); i++)    // refine the string to the appropriate format
    {
        if(i!=0  && paramflag==0 && command[i-1]!=' ' && (command[i]==' ' || command[i]=='\t'))
        {
            paramflag=1;
            tmp[j++]=' ';
        }
        if(command[i]==' ' || command[i]=='\t'){
            continue;
        }else{
            tmp[j++]=command[i];
        }
        
        if(command[i]==','){
            tmp[j] = ',';
            tmp[j++] = ' ';
        }
           
    }
    tmp[j] = '\0';
    strcpy(newNode->command, tmp);

    if(historyHead == NULL){    // if HistoryList is empty
        historyHead = newNode;
        historyTail = newNode;
    }else{      // if there is any HistoryNode, add new historyNode to historyTail
        historyTail->next = newNode;
        historyTail = newNode;
    }
}

/* function : help() */
/* purpose : print help message */
/* return : nothing */
void help(char* param, char command[]){
    // print help message
    if(*param != '\0' && *param != EOF){ // invalid command
        printf("Error : invalid command!\n");
    }else{  
        printf("help\nd[ir]\nq[uit]\nhi[story]\ndum[mp] [start, end]\n");
        printf("e[dit] address, value\nf[ill] start, end, value\n");
        printf("reset\nopcode mnemonic\nopcodelist\n");
        printf("assemble filename\ntype filename\nsymbol\n");
        recordHistory(command); 
    }
}

/* function : showHistory() */
/* purpose : print command history */
/* return : nothing */
void showHistory(char* param, char command[]){
    // print command history
    if(*param != '\0' && *param != EOF){
        printf("Error : invalid command!\n");
    }else{
        int num = 1;
        recordHistory(command); 
        HistoryNode* ptr;
        for(ptr = historyHead; ptr != NULL; ptr = ptr->next){
            printf("%d\t%s\n", num++, ptr->command);
        }
    }
}

/* function : directory() */
/* purpose : print file list in current directory */
/* return : nothing */
void directory(char* param, char command[]){
    // print file list in current directory
    if(*param != '\0' && *param != EOF){
        printf("Error : invalid command!\n");
    }else{
        DIR* dir_ptr = NULL;
        struct dirent* file = NULL;
        struct stat buf;
        dir_ptr = opendir("./"); // get information in current directory 
        if(dir_ptr != NULL){
            while((file = readdir(dir_ptr))!= NULL){
                stat(file->d_name, &buf);
                if(S_ISDIR(buf.st_mode)){   // directory
                    printf("%s/\n", file->d_name);
                }else if(S_IXUSR & buf.st_mode || S_IXGRP & buf.st_mode || S_IXOTH & buf.st_mode){ // execution file
                    printf("%s*\n", file->d_name);
                }else{  // etc
                    printf("%s\n", file->d_name);
                }
            }
            closedir(dir_ptr);
        }
        recordHistory(command); 
    }
}

/* function : getHex() */
/* purpose : get hexadecimal from user input string */
/* return : error - negative int, success - hexadecimal */
int getHex(char* str, int mode){
    // get Hex from string
    int hex = 0, len =0;
    
    if(*str =='-'){
        return -1;  // negative
    }

   while(!(*str == EOF || *str == '\0') && !(*str == ' ' || *str=='\t') && *str != ',' && len <6){
        hex *=16; // int to hex
        if(*str >'f' || *str < '0' || (*str > '9' && *str < 'A') || (*str >'F' && *str < 'a') ){
            return -2;  // not hex
        }
        
        if(*str <= '9'){    // '0' ~ '9'
            hex += *str - '0'; 
        }else if(*str <='F'){   // 'A' ~ 'F'
            hex += *str -'A' + 10;
        }else{  // 'a' ~ 'f'
            hex += *str - 'a' + 10;
        }
        str++; len++;   
    }

    if(len == 0){   // space
        return -5;
    }

    if(mode == 1){  // address
        if(len > 5){  // overflow
            return -3;
        }
    }else if(mode == 2){    // value
        if(len > 2){ // overflow
            return -3;
        }
    }
    else {
        if (len > 8) {
            return -3;
        }
    }
    
    return hex;
}

/* function : paramErrorMessage() */
/* purpose : print parameter error message */
/* return : nothing */
void paramErrorMessage(int errorNum){
    if(errorNum == -1){
        printf("Error : negative integer is not allowed!\n");
    }else if(errorNum == -2){
        printf("Error : it`s not hexadecimal!\n");
    }else if(errorNum == -3){
        printf("Error : overflow!\n");
    }else if(errorNum == -4){
        printf("Error : start must be less than end!\n");
    }else if(errorNum == -5){
        printf("Error : there must be more parameter!\n");
    }else{
        printf("Error : there must be ',' between parameters!\n");
    }
}

/* function : dump() */
/* purpose : show memory */
/* return : nothing */
void dump(char* param, char command[]){
    // dump function
    int start = last_address, end = 0;
    
    if(*param != '\0' && *param != EOF){    // there are parameters
        start = getHex(param, 1);  // get hexadecimal from first parameter

        if(start <0){   // error occured
            paramErrorMessage(start);
            return;
        }

        while(!(*param == EOF || *param == '\0') && !(*param == ' ' || *param=='\t') && *param != ','){
            param++;
        }   // now, param points after first parameter
        while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
            param++;
        }   // trim space(if there is)

        if(*param == ','){  // there might be second parameter
            param++;
            
            while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
                param++;
            }   // trim space(if there is)
            end = getHex(param , 1);    // get hexadecimal from seconde parameter

            if(end < 0){    // error occured
                paramErrorMessage(end);
                return;
            }

            if(start > end){    // start must be less than end
                paramErrorMessage(-4);
                return;
            }
        }else if(*param == EOF || *param == '\0'){   // there is only start parameter
            last_address = start;
        }else{  // there is no ',' between start and end parameter
            paramErrorMessage(-6);
            return;
        }
    }

    if(end == 0){   // if there is no end parameter, end = start + 159
        end = start + 159 <= 0xFFFFF ?  start + 159 : 0xFFFFF;  // if start + 159 > 0xFFFFF, end = 0xFFFFF
    }

    for(int i=start/16*16; i <= end / 16*16; i+= 16){   
        printf("%05X ", i); // print address
        for(int j = i; j < i + 16; j++){
            if (j <= end && j>=start){
                printf("%02X ", memory[j]); // if the value is within range, print value
            }else printf("   ");    // if the value is not within range, print space
        }
        printf("; ");
        for(int j=i ; j < i+16; j++){
            if(j <= end && j>= start && memory[j] >= 0x20 && memory[j] <= 0x7E){
                printf("%c", memory[j]);    // if the value is within range, print ASCII CODE
            }else printf(".");  // if the value is not within range, print '.'
        }
        printf("\n");
    }

    last_address = end + 1;  // renew last_address

    if(last_address > 0xfffff){ // if last_address over 0xfffff, reset last_address
        last_address = 0;
    }
    recordHistory(command);
}

/* function : edit() */
/* purpose : edit memory */
/* return : nothing */
void edit(char* param, char command[]){
    int address, value;

    if(*param == EOF || *param == '\0'){
        printf("Error : enter parameters!\n");
        return;
    }

    address = getHex(param, 1);

    if(address < 0 ){
        paramErrorMessage(address);
        return;
    }
    
    while(!(*param == EOF || *param == '\0') && !(*param == ' ' || *param=='\t') && *param != ','){
        param++;
    }   // now, param points after first parameter
    while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
        param++;
    }   // trim space(if there is)

    if(*param == ','){  //there might be second parameter
        param++;
    
        while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
            param++;
        }   // trim space(if there is)
        if(*param == EOF || *param == '\0'){    // there is no second parameter, value
            paramErrorMessage(-5);
            return;
        }
        value = getHex(param, 2);    // get hexadecimal from second parameter
        
        if(value< 0){   // error occured
            paramErrorMessage(value);
            return;
        }

    }else if(*param == EOF || *param == '\0'){  // there is no second parameter, value
        paramErrorMessage(-5);
        return;
    }else{
        paramErrorMessage(-2);  // invalid parameter
        return;
    }

    memory[address] = value;    // put value in the memory that address points to
    recordHistory(command);    
}

/* function : fill() */
/* purpose : fill memory */
/* return : nothing */
void fill(char* param, char command[]){
    int start, end, value;

    if(*param == EOF || *param == '\0'){
        printf("Error : enter parameters!\n");
        return;
    }

    start = getHex(param, 1);

    if(start < 0 ){
        paramErrorMessage(start);
        return;
    }
    
    while(!(*param == EOF || *param == '\0') && !(*param == ' ' || *param=='\t') && *param != ','){
        param++;
    }   // now, param points after first parameter
    while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
        param++;
    }   // trim space(if there is)

    if(*param == ','){  //there might be second parameter
        param++;
    
        while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
            param++;
        }   // trim space(if there is)
        if(*param == EOF || *param == '\0'){    // there is no second parameter, value
            paramErrorMessage(-5);
            return;
        }
        end = getHex(param, 1);    // get hexadecimal from second parameter
        
        if(end< 0){   // error occured
            paramErrorMessage(end);
            return;
        }
        if(start > end){  // start is bigger than end
            paramErrorMessage(-4);
            return;
        }

        while(!(*param == EOF || *param == '\0') && !(*param == ' ' || *param=='\t') && *param != ','){
           param++;
        }   // now, param points after first parameter
        while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
            param++;
        }   // trim space(if there is)

        if(*param == ','){
            param++;

            while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
                param++;
            }   // trim space(if there is)

            if(*param == EOF || *param == '\0'){    // there is no second parameter, value
                paramErrorMessage(-5);
                return;
            }

            value = getHex(param, 2);   // get hexadecimal from third parameter

            if(value < 0){  // error occured
                paramErrorMessage(value);
                return;
            }

        }else if(*param == EOF || *param == '\0'){
            paramErrorMessage(-5);  // there is no third parameter, value
            return;
        }else{
            paramErrorMessage(-2);  // invalid parameter
            return;
        }

    }else if(*param == EOF || *param == '\0'){  // there is no second parameter, end
        paramErrorMessage(-5);
        return;
    }else{
        paramErrorMessage(-2);  // invalid parameter
        return;
    }

    for(int i= start; i<= end ; i++){
        memory[i] = value;  // fill memory 
    }

    recordHistory(command);    

}

/* function : reset() */
/* purpose : reset memory */
/* return : nothing */
void reset(char* param, char command[]){
    
    if(*param == EOF || *param == '\0'){
        for(int i=0; i< 1048576 ; i++){
            memory[i] = 0;  // reset memory
        }
        recordHistory(command);
    }else{
        printf("Error : invalid command!\n");
    }
    
}

/* function : hash() */
/* purpose : hash fucntion for hashtable */
/* return : int key - hashtable key */
int hash(char* str){
    // hash function
    // hash algorithm : translate str to int.  
    int key = 0;
    while(*str != EOF && *str != '\0'){
        key += *str - 'A';
        str++;
    }
    return key % 20;
}

/* function : makeHashTable() */
/* purpose : make opcode hashTable */
/* return : error - -1 , success - 1 */
int makeHashTable(){
    // make opcode hashtable using hash function
    FILE* fp;
    char mnemonic[10], format[5];
    int opcode, key;

    if( (fp = fopen("opcode.txt", "r")) == NULL){
        printf("Error : can not open file!\n"); // file open error
        return -1;
    }

    while(fscanf(fp, "%x %s %s", &opcode, mnemonic, format) != EOF){    // it continue until EOF
        OpcodeNode* newNode = (OpcodeNode*)malloc(sizeof(OpcodeNode));  // allocate new OpcodeNode
        newNode->opcode = opcode;
        newNode->next = NULL;
        strcpy(newNode->mnemonic, mnemonic);
        strcpy(newNode->format, format);

        key = hash(mnemonic);   // get key from hash function
        if(hashtable[key] != NULL){ //  not empty hashtable list
            OpcodeNode* ptr = hashtable[key];
            while(ptr->next){
                ptr = ptr->next;
            }
            ptr->next = newNode;
        }else{  // empty hashtable list
            hashtable[key] = newNode;
        }
    }

    fclose(fp);
    return 1;
}

/* function : opcode() */
/* purpose : translate mnemonic to opcode and print it */
/* return : nothing */
void opcode(char* param , char command[]){
    int key = 0, opcodeFlag = 0;
    char* tmp;
    OpcodeNode* ptr;

    if(*param == EOF || *param == '\0'){    // if there is no parameter
        paramErrorMessage(-5);
        return;
    }

    while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
        param++;
    }   // trim space(if there is)

    tmp = param;
    while(!(*tmp == EOF || *tmp == '\0') && !(*tmp == ' ' || *tmp=='\t')){
        tmp++;
    }   // trim space(if there is)
    *tmp = '\0'; // make param 'non-space word'
    
    key = hash(param);
    for(ptr = hashtable[key]; ptr; ptr = ptr->next){    // print opcode
        if( strcmp(ptr->mnemonic, param) == 0){
            opcodeFlag = 1;
            printf("opcode is %X\n", ptr->opcode);
            recordHistory(command);
            break;
        }
    }
    if(!opcodeFlag){    // error
        printf("Error : there is no such mnemonic!\n");
    }
}

/* function : opcodelist() */
/* purpose : print opcode hashtable to appropriate format */
/* return : nothing */
void opcodelist(char* param, char command[]){
    // printf opcode list
    if(*param == EOF || *param == '\0'){    // if there is no parameter
        for(int i=0; i<20 ; i++){
            OpcodeNode* ptr;
            printf("%d : ", i);
            for(ptr = hashtable[i]; ptr ; ptr = ptr->next){
                printf("[%s, %X]", ptr->mnemonic, ptr->opcode);
                if(ptr->next){
                    printf(" -> ");
                }
            }
            printf("\n");
        }

        recordHistory(command);
    }else{  // error
        printf("Error : invalid command!\n");
        return;
    }

}

/* function : symtabHash() */
/* purpose : symbol table hash function */
/* return : int key - symbol tabel key */
int symtabHash(char* str){
    // hash function
    // hash algorithm : translate str to int.  
    return *str - 'A';
}

/* function : clearSymtab() */
/* purpose : clear symbol table */
/* return : nothing */
void clearSymtab(){
    SymbolNode* ptr, *tmp;
    
    for(int i=0; i<26; i++){
        ptr = symboltable[i];
        while(ptr){
            tmp = ptr->next;
            free(ptr);
            ptr = tmp;
        }
    }
}

/* function : type() */
/* purpose : print contents of selected file */
/* return : nothing */
void type(char* param, char command[]){
    FILE* fp;
    char* tmp;
    char str[255];
    if(*param == EOF || *param == '\0'){    // if there is no parameter
        paramErrorMessage(-5);
        return;
    }

    while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
        param++;
    }   // trim space(if there is)

    tmp = param;
    while(!(*tmp == EOF || *tmp == '\0') && !(*tmp == ' ' || *tmp=='\t')){
        tmp++;
    }   // trim space(if there is)
    *tmp = '\0'; // make param 'non-space word', now param is filename
    
    if( (fp = fopen(param, "r")) ){ // if file exists
        while( fgets(str, sizeof(str), fp) !=NULL ){
            printf("%s", str);
        }
        fclose(fp);
        recordHistory(command);
    }else{
        printf("Error : there is no such file!\n");
        return;
    }

}

/* function : pass1() */
/* purpose :  Algorithm for pass1 of assembler*/
/* return : error(-1) not error(1) */
int pass1(char filename[], int* programLen){
    FILE *asmPtr, *itmPtr; // indicate asm file, intermediate file
    int locCtr=0, lineCnt=0, insCtr=0, startAddr=0, existFlag = 0, errorFlag=0, idx=0, negFlag = 0, i,key, value=0, plusFlag=0, tmp;
    char label[30], mnemonic[30], operand[30], line[100], temp[10];
    SymbolNode* symPtr, *newNode;
    OpcodeNode* opPtr;

    if( !(asmPtr = fopen(filename, "r")) ){   // read file 
        printf("Error : there is no such file!\n"); // file open error
        return -1;
    }

    if( !(itmPtr = fopen("itm" , "w")  )){ // write intermediate file
        printf("Error : can not create intermediate file!\n");  // file write error
        fclose(asmPtr);
        return -1;
    }

    while( ((fgets(line, 100, asmPtr)) != NULL) && !errorFlag ){
        line[strlen(line)-1] = '\0';
        lineCnt++; 
        idx = 0; insCtr = 0; plusFlag = 0; value = 0; label[0] = '\0';
        if (line[0] == '\0') {
            break;
        }
        fprintf(itmPtr, "%4d\t", 5*lineCnt);    // print line number
        
        if(line[0] == '.'){   // comment line 
            fprintf(itmPtr, "%s\n", line);
            continue;   // skip comment line
        }

        if( isAlphabet(line[0]) ){ // start with alphabet -> label exists
            i=0;
            while( !isDelimiter(line[idx]) ){   // store label
                label[i] = line[idx];
                i++; idx++;
                if( !isDelimiter(line[idx]) && !isAlphabet(line[idx]) && !isDecimal(line[idx]) ) // it`s not label
                {
                    printf("%s: %d: Error : label cannot have non-alphabet or non-number!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }
            }
            label[i] = '\0';
            
            key = symtabHash(label);    // get key of symbol table
            symPtr = symboltable[key];

            while(symPtr){  // check if label exist in symbol table
                if(strcmp(label, symPtr->symbol) == 0 ){
                    printf("%s: %d: Error: %s is already exist in symbol table!\n", filename, lineCnt, label);
                    errorFlag = 1;
                    break;
                }
                symPtr = symPtr->next;
            }
        }

        if(!isIndent(label[0]) && !isEnd(label[0])){    // label exists
            fprintf(itmPtr, "%s\t", label);
        }else{  // label does not exist -> blank
            fprintf(itmPtr, ">\t");
        }
        
        while(!isEnd(line[idx]) && isDelimiter(line[idx])){
            idx++;  // skip blank
        }

        if(line[idx] == '+'){ // if prefix(+) exists
            fprintf(itmPtr, "%c", line[idx]);
            plusFlag = 1;
            idx++;
        }else if(line[idx] == '@' || line[idx] == '#'){
            printf("%s: %d : Error: invalid prefix!\n", filename, lineCnt);
            errorFlag = 1;
            break;
        }

        i=0; 
        while(isAlphabet(line[idx])){   // get mnemonic
            mnemonic[i++] = line[idx++];
        }
        mnemonic[i] = '\0';
        
        while(!isEnd(line[idx]) && isIndent(line[idx])){    // skip blank
            idx++;
        }

        fprintf(itmPtr, "%s\t", mnemonic);   
        
        if(isDirective(mnemonic)){  // if mnemonic is directive
            if(strcmp("START", mnemonic)==0){
                if(isEnd(line[idx])){   // if there is no operand(starting address)
                    printf("%s: %d: Error: there is no operand(starting address)!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }
                
                i=0;
                while(!isEnd(line[idx]) && !isDelimiter(line[idx])){ // get operand
                    operand[i++] = line[idx++];
                }
                operand[i] = '\0';
                
                startAddr = getHex(operand, 1);    // string to hexadecimal
                if(startAddr < 0){   // invalid address
                    printf("%s: %d: ", filename, lineCnt);
                    paramErrorMessage(startAddr);
                    errorFlag = 1;
                    break;
                }

                locCtr = startAddr; // set locCtr
                fprintf(itmPtr, "%s", operand);

            }else if(strcmp("BASE", mnemonic)==0){
                if(isEnd(line[idx])){   //  if there is no operand
                    printf("%s: %d: Error: there is no operand!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }
                if(!isAlphabet(line[idx])){  // invalid mnemonic(non alphabet)
                    printf("%s: %d: Error: invalid mnemonic!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }

                while(!isEnd(line[idx]) && !isDelimiter(line[idx])){  // get operand
                    fprintf(itmPtr, "%c", line[idx]);
                    idx++;
                }
               
            }else if( strcmp("BYTE", mnemonic) == 0){
                fprintf(itmPtr, "%c", line[idx]);
                if(line[idx] == 'C'){   // Charater
                    insCtr = 1;
                }else if(line[idx] == 'X'){ // Hexadecimal
                    insCtr = 2;
                }else{
                    printf("%s: %d: Error: invalid format!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }
                idx++;
                if( line[idx] != '\'' ){  // invalid constant  
                    printf("%s: %d: Error: invalid constant!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }

                fprintf(itmPtr, "%c", line[idx]);   // fprintf ' 
                idx++;
                i=0;
                while(!isEnd(line[idx]) && line[idx] != '\''){
                    fprintf(itmPtr, "%c", line[idx]);
                    idx++;
                    i++;    // here, i counts length;    
                }
                if(line[idx] != '\''){ // invalid constant
                    printf("%s: %d: Error: invalid format!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }
                fprintf(itmPtr, "%c", line[idx]); // fprintf '
                if(insCtr == 1){    // charater
                    insCtr = i;
                }else{  // hexadecimal
                    insCtr = (i+1)/2;
                }

            }else if(strcmp("WORD", mnemonic) == 0){
                if(line[idx] == '-'){   // if negative value
                    negFlag = 1;
                }
                idx++;
                while(isDecimal(line[idx]) && !isEnd(line[idx])){   // get value
                    value *= 10;
                    value += line[idx] - '0';
                    idx++;
                }

                if(negFlag){    // if value is negative
                    value *= -1;
                }

                fprintf(itmPtr, "%d", value);

                insCtr = 3; // WORD = 3 byte

            }else if(strcmp("RESB", mnemonic) == 0){
                while(isDecimal(line[idx]) && !isEnd(line[idx]) ){
                    value *= 10;
                    value += line[idx] - '0';
                    idx++;
                }
                fprintf(itmPtr, "%d", value);
                insCtr = value*1;   // value * 1byte
               
                             
            }else if(strcmp("RESW", mnemonic) == 0){
                while(isDecimal(line[idx]) && !isEnd(line[idx]) ){
                    value *= 10;
                    value += line[idx]- '0';
                    idx++;
                }
                fprintf(itmPtr, "%d", value);
                insCtr = value*3;   // WORD = 3Byte
                
            }else{  //  mnemonic == END
                if(isAlphabet(line[idx])){
                    i=0; existFlag = 0;
                    while(!isDelimiter(line[idx])){ // get operand
                        operand[i++] = line[idx++];
                    }
                    operand[i] = '\0';
                    while(!isEnd(line[idx]) && isIndent(line[idx])){    //skip blank    
                        idx++;
                    }
                    
                    key = symtabHash(operand); // get key of symbol table
                    symPtr = symboltable[key];
                    while(symPtr != NULL)
                    {   
                        if(strcmp(operand, symPtr->symbol) == 0){
                            existFlag = 1;
                            break;
                        }
                        symPtr = symPtr->next;
                    }
                    fprintf(itmPtr, "%s", operand);
                }else{
                    printf("%s: %d: Error: invalid operand!\n", filename, lineCnt);
                    errorFlag = 1;
                    break;
                }

                *programLen = locCtr - startAddr;   // store program length
                break;
            }
        }else{  // if mnemonic is not directive 
            existFlag = 0;
            key = hash(mnemonic); // get key of opcode table 
            opPtr = hashtable[key];
            while(opPtr != NULL){
                if( strcmp(opPtr->mnemonic, mnemonic)==0 ){
                    existFlag = 1;
                    break;
                }
                opPtr = opPtr->next;
            }

            if(!existFlag){ // does not exist in optab
                printf("%s: %d: Error: %s does not exist in opcode table!\n", filename, lineCnt, mnemonic);
                errorFlag = 1;
                break;
            }

            insCtr = opPtr->format[0] - '0';    // format3 = 3 format 4 = 4
            if(plusFlag){   // format 4
                insCtr = 4;
            }
            if (strcmp("RSUB", mnemonic) == 0) {
                fprintf(itmPtr, "!\t");
            }
            if(line[idx] == '@' || line[idx] == '#'){   // if there is prefix
                fprintf(itmPtr, "%c", line[idx]);
                idx++;
            }
            i=0;
            while( !isEnd(line[idx]) && !isDelimiter(line[idx]) ){
                operand[i++] = line[idx++];
            }
            operand[i] = '\0';

            fprintf(itmPtr, "%s", operand);

            while(!isEnd(line[idx]) && isIndent(line[idx])){    // skip blanks
                idx++;
            }

            if( line[idx] == ',' ){  // if there is second operand
                fprintf(itmPtr, ",");
                idx++;
                while (!isEnd(line[idx]) && isIndent(line[idx])) {    // skip blanks
                    idx++;
                }
                i=0;
                while(!isEnd(line[idx]) && !isDelimiter(line[idx])){
                    operand[i++] = line[idx++];
                }
                operand[i] = '\0';
                fprintf(itmPtr, "%s", operand);
            }
        }

        while(!isEnd(line[idx])){
            idx++;
        }

        fprintf(itmPtr, "\t%X", locCtr);    // print locCtr

        if( isAlphabet(label[0]) && strcmp("START", mnemonic) != 0){   // store symbol in symbol table if exist
            key = symtabHash(label);

            newNode = (SymbolNode*)malloc(sizeof(SymbolNode));
            newNode->next = NULL;
            strcpy(newNode->symbol, label);
            newNode->locCtr = locCtr;
            
            if(!symboltable[key]){  // empty
                symboltable[key] = newNode;
            }else{  //   insert alphabeticallly
                symPtr = symboltable[key];
                while(symPtr->next){
                    if(strcmp(newNode->symbol, symPtr->symbol) > 0  && strcmp(newNode->symbol, symPtr->next->symbol) < 0){
                        newNode->next = symPtr->next;
                        symPtr->next = newNode;
                        break;
                    }
                    symPtr = symPtr->next;
                }
                symPtr->next = newNode;
                if( strcmp(symPtr->symbol, symPtr->next->symbol) >0 ){  // swap
                    strcpy(temp, symPtr->symbol);
                    strcpy(symPtr->symbol, symPtr->next->symbol);
                    strcpy(symPtr->next->symbol, temp);
                    tmp = symPtr->locCtr;
                    symPtr->locCtr = symPtr->next->locCtr;
                    symPtr->next->locCtr = tmp;                    
                } 
                
            }
        }

        locCtr += insCtr;   // store next locCtr

        fprintf(itmPtr, "\t%X\n", locCtr);  // print PC
    }

   
    if(errorFlag){
        clearSymtab();
        fclose(asmPtr);
        fclose(itmPtr);
        return -1;
    }

    fclose(asmPtr);
    fclose(itmPtr);

    return 1;
}

/* function : twosComplement() */
/* purpose : change negative hex to positive hex */
/* return : int positive hex */
int twosComplement(int x) {
    int a = 0xFFF;
    int i, j;
    a = ~(x * (-1)) + 1;
    char temp[10], res[4];
    sprintf(temp, "%X", a);
    for ( i = 5, j=0; i <8 ; i++, j++) {
        res[j] = temp[i];
    }
    res[3] = '\0';
    
    return getHex(res, 1);

}


/* function : pass2() */
/* purpose : algortim for pass2 of assemble */
/* return : error : -1 not error : 1) */
int pass2(char filename[], int programLen){
    int locCtr = 0, lineCnt = 0, i = 0, idx = 0, oidx=0, errorFlag = 0, PC = 0, B = 0, key = 0, value = 0, opcode = 0, disp =0;
    int existFlag = 0, objCnt = 0, startAddr = 0, tmp = 0, recStart = 0, enterFlag=0, lastCnt =0, modi=0, totalCnt = 0;
    int mod[1000];
    char objName[50], lstName[50], line[100], label[30], mnemonic[30], operand[30], objLine[100], objCode[10], temp[10];
    FILE* itmPtr, *objPtr, *lstPtr;
    SymbolNode* symPtr;
    OpcodeNode* opPtr;
    
    i = 0;
    for (i = 0; filename[i] != '.'; i++) {
        objName[i] = filename[i];
        lstName[i] = filename[i];
    }

    objName[i] = '\0';
    lstName[i] = '\0';

    strcat(objName, ".obj");
    strcat(lstName, ".lst");
    printf("[%s], [%s]\n", lstName, objName);

    if (!(itmPtr = fopen("itm", "r"))){
        printf("Error: can not open intermediate file!\n");
        return -1;
    }
    
    if (!(objPtr = fopen(objName, "w"))) {
        printf("Error: can not create object file!\n");
        fclose(itmPtr);
        return -1;
    }

    if (!(lstPtr = fopen(lstName, "w"))) {
        printf("Error: can not create listing file!\n");
        fclose(itmPtr);
        fclose(objPtr);
        return -1;
    }

    
    while (((fgets(line, 100, itmPtr)) != NULL) && !errorFlag) {
        existFlag = 0; enterFlag = 0;
        sscanf(line, "%d\t%*s", &lineCnt);
        if (strchr(line, '.')) {    // comment line
            for (i = 0; i < strlen(line); i++) {
                if (line[i] == '.')
                    break;
            }
            fprintf(lstPtr, "%-4d\t    \t%s", lineCnt, line+i);
            continue;
        }
        else {  // not comment line
            sscanf(line, "%d %s %s %s %X %X", &lineCnt, label, mnemonic, operand, &locCtr, &PC);
        }     
    
        if (label[0] == '>') {
            strcpy(label, "      ");
        }
        if (mnemonic[0] == '+') {   // format 4
            key = hash(mnemonic+1);   // find opcode in opcode table
            opPtr = hashtable[key];
            while (opPtr != NULL) {
                if (strcmp(opPtr->mnemonic, mnemonic+1) == 0) {
                    existFlag = 1;
                    break;
                }
                opPtr = opPtr->next;
            }
        }
        else {
            key = hash(mnemonic);   // find opcode in opcode table
            opPtr = hashtable[key];
            while (opPtr != NULL) {
                if (strcmp(opPtr->mnemonic, mnemonic) == 0) {
                    existFlag = 1;
                    break;
                }
                opPtr = opPtr->next;
            }
        }        

        if (!existFlag) {   // mnemonic might be directive
            if (strcmp("START", mnemonic) == 0) {   // print H part
                fprintf(objPtr, "H%-6s%06X%06X\n", label, locCtr, programLen);
                fprintf(lstPtr, "%-4d\t%04X\t%-6s\t%-6s\t%-10s\n", lineCnt, locCtr, label, mnemonic, operand);
                startAddr = locCtr;
                recStart = startAddr;
                continue;
            }
            else if (strcmp("BYTE", mnemonic) == 0) {
                i = 0;
                if (operand[0] == 'C') {    // character
                    for (i = 2, tmp=0; operand[i] != '\''; i++, tmp+=2) {
                        sprintf(objCode + tmp, "%02X", operand[i]);
                    }
                }
                else {  // hexadecimal
                    for (i = 2, tmp = 0; operand[i] != '\''; i++, tmp++) {
                        sprintf(objCode + tmp, "%c", operand[i]);
                        
                    }
                }
                fprintf(lstPtr, "%-4d\t%04X\t%-6s\t%-6s\t%-10s", lineCnt, locCtr, label, mnemonic, operand);
            }
            else if (strcmp("RESW", mnemonic) ==0 || strcmp("RESB", mnemonic)==0 ) {
                fprintf(lstPtr, "%-4d\t%04X\t%-6s\t%-6s\t%-10s", lineCnt, locCtr, label, mnemonic, operand);
                enterFlag = 1;
            }
            else if (strcmp("WORD", mnemonic) == 0) {
                value = getHex(operand, 1); // get WORD value
                sprintf(objCode, "%06X", value);
                fprintf(lstPtr, "%-4d\t%04X\t%-6s\t%-6s\t%-10s", lineCnt, locCtr, label, mnemonic, operand);
                
            }
            else if (strcmp("BASE", mnemonic) == 0) {
                fprintf(lstPtr, "%-4d\t    \t%-6s\t%-6s\t%-10s", lineCnt, label, mnemonic, operand);
                key = symtabHash(operand);
                symPtr = symboltable[key];
                while (symPtr != NULL) {    // find the symbol in symbol table
                    if (strcmp(symPtr->symbol, operand) == 0) {
                        break;
                    }
                    symPtr = symPtr->next;
                }
                B = symPtr->locCtr; // store Base address
            }
            else { // END
                fprintf(lstPtr, "%-4d\t    \t      \t%-6s\t%-10s\n", lineCnt, mnemonic, operand);
                break;
            }
        }
        else {  // opcode in opcode table
            if (strcmp("RSUB", mnemonic) == 0) {
                fprintf(lstPtr, "%-4d\t%04X\t      \t%-6s\t          ", lineCnt, locCtr, mnemonic);
            }
            else {
                fprintf(lstPtr, "%-4d\t%04X\t%-6s\t%-6s\t%-10s", lineCnt, locCtr, label, mnemonic, operand);
            }
            opcode = opPtr->opcode;    // get opcode
            idx = 0;
            key = symtabHash(operand);  
            symPtr = symboltable[key];
            while (symPtr != NULL) {    // find the symbol in symbol table
                if (strcmp(symPtr->symbol, operand) == 0) {
                    existFlag = 1;
                    break;
                }
                symPtr = symPtr->next;
            }

            if (operand[0] == '#') {    // imediate addressing
                opcode += 1;   // set i = 1, n=0 -> 1
            }
            else if (operand[0] == '@') {   // indirect addressing
                opcode += 2;   // set i=0, n = 1 -> 2
            }
            else if (opPtr->format[0] == '2' && isReg(operand[0])){
                opcode += 0;   // no memory reference
            }
            else {  // simple addressing
                if (opPtr->format[0] != '1') {  // not format 1
                    opcode += 3;   // set i= 1, n= 1 -> 3
                }
                else {  // format 1
                    opcode += 0;
                }
            }
            sprintf(objCode, "%02X", opcode);
            idx += 2;
            

            if (opPtr->format[0] == '1') {  // format 1
                sprintf(objLine, "%s", objCode);
                oidx += strlen(objCode);
            }
            else if (opPtr->format[0] == '2') {
                if (strlen(operand) == 1) { // one register
                    value = regTable(operand);
                    if (value == -1) {
                        errorFlag = 1;
                        printf("%s: %d: Error: invalid register!\n", filename, lineCnt);
                        break;
                    }
                    else {
                        sprintf(objCode + idx, "%01X0", value);
                        idx += 1;
                    
                    }
                }
                else {  // two register
                    for (i = 0; i < strlen(operand); i++) {
                        if (operand[i] == ',') {
                            operand[i] = '\0';
                            tmp = i+1;
                            value = regTable(operand);
                            sprintf(objCode + idx, "%01X", value);
                            idx += 1;
                        }          
                    }
                    value = regTable(operand + tmp);
                    sprintf(objCode + idx, "%01X", value);
                    idx += 1;
                }
                
            }
            else if (opPtr->format[0] == '3' && mnemonic[0] != '+') {
                if (strcmp("LDB", mnemonic) == 0 || operand[0] != '#') {    // simple or indirect addressing
                    if (operand[0] == '#' || operand[0] == '@' ) {
                        key = symtabHash(operand + 1);
                        symPtr = symboltable[key];
                        while (symPtr != NULL) {    // find the symbol in symbol table
                            if (strcmp(symPtr->symbol, operand+1) == 0) {
                                existFlag = 1;
                                break;
                            }
                            symPtr = symPtr->next;
                        }
                    }
                    if (operand[strlen(operand) - 1] == 'X') {
                        for (i = 0; operand[i] != ','; i++) {
                            temp[i] = operand[i];
                        }
                        temp[i] = '\0';
                        key = symtabHash(temp);
                        symPtr = symboltable[key];
                        while (symPtr != NULL) {    // find the symbol in symbol table
                            if (strcmp(symPtr->symbol, temp) == 0) {
                                existFlag = 1;
                                break;
                            }
                            symPtr = symPtr->next;
                        }
                    }
                    if (operand[0] != '!') {    // not RSUB
                        disp = symPtr->locCtr;
                        if (disp - PC > -2049 && disp - PC < 2048) {  // try PC relative first
                            disp -= PC;
                            sprintf(objCode + idx, "2");    // set p = 1 -> 2
                            idx += 1;
                        }
                        else if (disp - B > -1 && disp - B < 4096) { // try B relative second
                            disp -= B;
                            if (operand[strlen(operand) - 1] == 'X') {
                                sprintf(objCode + idx, "C");    // set b = 1, x= 1 - > C
                                idx += 1;
                            }
                            else {
                                sprintf(objCode + idx, "4");    // set b = 1 - > 4
                                idx += 1;
                            }
                        }
                        else {  // overflow
                            errorFlag = 1;
                            printf("%s: %d : Error : overflow occured!\n", filename, lineCnt);
                            break;
                        }

                        if (disp < 0) {
                            disp = twosComplement(disp);
                        }
                        sprintf(objCode + idx, "%03X", disp);
                        idx += 3;
                        
                    }
                    else {  // RSUB
                        disp = 0;
                        sprintf(objCode + idx, "0");    // set x=b=p=e=0 - > 0
                        idx += 1;
                        sprintf(objCode + idx, "%03X", disp);
                        idx += 4;
                    }
                   
                }
                else {  // direct addressing
                    disp = getHex(operand + 1, 1);
                    sprintf(objCode + idx, "0%03X", disp);
                    idx += 3;
                    
                }
            }
            else if (opPtr->format[0] == '4' || mnemonic[0] == '+') {
                if (operand[0] != '#') {    // simple or indirect addressing
                    disp = symPtr->locCtr;
                    sprintf(objCode + idx, "1%05X", disp);
                    idx += 5;
                    
                }
                else {  //direct addressing
                    disp = strToInt(operand + 1);
                    sprintf(objCode + idx, "1%05X", disp);
                    idx += 5;
                    
                }
            }
            else {  // invalid format
                errorFlag = 1;
                printf("%s: %d: Error : invalide format!\n", filename, lineCnt);
                break;
            }
        }

        objCnt += strlen(objCode) / 2;
        lastCnt = strlen(objCode) / 2;
        totalCnt += strlen(objCode)/2;
        if( strlen(objCode)/2 == 4 && operand[0] != '#'){    // needed modification
            mod[modi++] = totalCnt-3;
        }

        if ( objCnt >= 30 ) { // record one line if objCnt >= 30
            if (objCnt == 30) { // fit in one line
                fprintf(objPtr, "T%06X%02X%s\n", recStart, objCnt, objLine);
            }
            else {  // over one line
                fprintf(objPtr, "T%06X%02X%s\n", recStart, objCnt - lastCnt, objLine);
            }
            objCnt = 0;
            oidx = 0;
            sprintf(objLine, "%s", objCode);
            oidx += strlen(objCode);
            objCnt += strlen(objCode) / 2;
            recStart = locCtr;
        }
        else if (enterFlag) {   // if RESW ,RESB 
            if (objLine[0] != '\0') {
                fprintf(objPtr, "T%06X%02X%s\n",recStart, objCnt-lastCnt, objLine);
                recStart = locCtr;
            }
            recStart = PC;
            objCnt = 0;
            oidx = 0;
            objLine[0] = '\0';
            enterFlag = 0;
        }
        else {  // record
            if (objCode[0] != '\0') {
                sprintf(objLine + oidx, "%s", objCode);
                oidx += strlen(objCode);
            }
        }
        
        if (enterFlag || strcmp("BASE", mnemonic) == 0) {
            fprintf(lstPtr, "\n");
        }
        else {
            fprintf(lstPtr, "%s\n", objCode);
        }
        objCode[0] = '\0';
    }

    if (objLine[0] != '\0') {
        fprintf(objPtr, "T%06X%02X%s\n", recStart, objCnt, objLine);
    }

    for(i=0; i<modi; i++){  // print modification
        fprintf(objPtr,"M%06X05\n", mod[i]);
    }

    fprintf(objPtr, "E%06X\n", startAddr);
    if(errorFlag){
        remove(objName);
        remove(lstName);
        return -1;
    }

    fclose(itmPtr);
    fclose(objPtr);
    fclose(lstPtr);

    return 1;
}



/* function : assemble() */
/* purpose : assemble .asm file */
/* return : nothing */
void assemble(char* param, char command[]){
    char* tmp;
    int programLen;
    if(*param == EOF || *param == '\0'){    // if there is no parameter
        paramErrorMessage(-5);
        return;
    }

    while(!(*param == EOF || *param == '\0') && (*param == ' ' || *param=='\t')){
        param++;
    }   // trim space(if there is)

    tmp = param;
    while(!(*tmp == EOF || *tmp == '\0') && !(*tmp == ' ' || *tmp=='\t')){
        tmp++;
    }   // trim space(if there is)
    *tmp = '\0'; // make filename 'non-space word'
    
    if(pass1(param, &programLen) < 0 ){
        printf("assemble failed!\n");
    }else{
        if(pass2(param, programLen)<0){
            printf("assemble failed!\n");
        }else{  // success!
            recordHistory(command);
        }
    }

}

/* function : printSymbol() */
/* purpose : print symboltable */
/* return : nothing */
void printSymbol(char* param, char command[]){
    SymbolNode* symPtr;
       
    for(int i =0; i<26 ; i++){
        symPtr = symboltable[i];
        while(symPtr){
            printf("\t%-6s\t%04X\n",symPtr->symbol, symPtr->locCtr);
            symPtr = symPtr->next;
        }
    }

    recordHistory(command);
}

/* function : progAddress()*/
/* purpose : set program address */
/* return : nothing */
void progAddress(char* param, char command[]){
    int temp = getHex(param, 1);    // string to hex

    progAddr = temp;    // set progAddr

}

/* function : compare()*/
/* purpose : qsort`s compare function */
/* return : int  */
int compare(const void* a, const void* b) {
    return ((ExternNode*)a)->address - ((ExternNode*)b)->address;
}


/* function : extabHash()*/
/* purpose : externtable hash algorithm */
/* return : int key */
int extabHash(char* str){
    int key = 0;
    for(int i=0; i<strlen(str); i++){
        key += str[i] - 'A';
    }
    return key % 26;
}

/* function : clearExtab()*/
/* purpose : clear externtable */
/* return : nothing */
void clearExtab() {
    ExternNode* ptr, *tmp; 
    for (int i = 0; i < 26; i++) {
        ptr = externtable[i];
        while (ptr) {
            tmp = ptr->next;
            free(ptr);
            ptr = tmp;
        }
        externtable[i] = NULL;
    }
}

/* function : linkPass1()*/
/* purpose : pass1 of linking loader */
/* return : int size(exteren table size) */
int linkLoadPass1(char param[3][10], int fileCnt){
    int i,idx, key, csAddr, csLen, addr,size=0;
    char line[100], progName[7], symName[7];
    FILE* fp;
    ExternNode *newNode, *ptr;

    csAddr = progAddr;  // first csAddr is progAddr

    for(i=0; i<fileCnt; i++){   
        if(!(fp = fopen(param[i], "r"))){
            printf("Error: can not open file!\n");
            return -1;
        }

        fgets(line, 100, fp);
        sscanf(line, "%*c%6s%6X%6X", progName, &addr, &csLen); // get program name and control section length;
        
        key = extabHash(progName);
       
        newNode = (ExternNode*)malloc(sizeof(ExternNode)); // make new ExternNode
        newNode->address = addr + csAddr;
        newNode->length = csLen;
        strcpy(newNode->progName, progName);
        strcpy(newNode->symName, "NULL");
        newNode->next = NULL;
        size++;
        // insert newNode into externtable
        if(!externtable[key]){   // empty
            externtable[key] = newNode;
        }
        else{   // not empty   
            for(ptr = externtable[key]; ptr->next ; ptr= ptr->next){
                continue;
            }
            ptr->next = newNode;
        }

        while(1){
            fgets(line, 100, fp);
            if(line[0] == 'E'){ // End record 
                break;
            }
            else if(line[0] == 'D'){    // Data record
                idx = 1;
                while(1){
                    if(isEnd(line[idx]) || line[idx] == '\n'){
                        break;
                    }
                    sscanf(line+idx, "%6s%6X", symName, &addr); // get symbol name and address
                    
                    key = extabHash(symName);
                    newNode = (ExternNode*)malloc(sizeof(ExternNode));
                    newNode->address = addr + csAddr;   // add csAddr
                    newNode->length = 0;
                    strcpy(newNode->progName, "NULL");
                    strcpy(newNode->symName , symName);
                    newNode->next = NULL;
                    size++;

                    if(!externtable[key]){  // empty
                        externtable[key] = newNode;
                    }
                    else{   // not empty
                        for(ptr=externtable[key]; ptr->next; ptr = ptr->next){
                            continue;   
                        }
                        ptr->next = newNode;
                    }
                    idx+= 12;
                }
            }else{  // record that we don`t consider
                continue;
            }
        }

        csAddr += csLen; // renew control address for next control section
        fclose(fp); // close current obj file

    } 

    return size;

}

/* function : linkLoadPass2()*/
/* purpose : pass2 of linking loader */
/* return : error(-1) or int total length */
int linkLoadPass2(char param[3][10], int fileCnt){
    int i,j, idx, key, csAddr, csLen,refNum , addr, textLen, temp, loc, modAddr, val, total=0;
    int ref[100] = {0,};
    
    char line[100], progName[7], symName[7], value[10], sign; 
    FILE* fp;
    ExternNode* ptr;

    csAddr = progAddr;  // first csAddr is progAddr
    addr = 0;
    for(i=0; i<fileCnt; i++){
        if(!(fp = fopen(param[i], "r"))){
            printf("Error : can not open file!\n");
            return -1;
        }    

        fgets(line, 100, fp);   // get program name and length;
        sscanf(line, "%*c%6s%*6X%6X", progName, &csLen);
        key = extabHash(progName);
        for (ptr = externtable[key]; ptr; ptr = ptr->next) {    // find appropriate member
            if (strcmp(ptr->progName, progName) == 0) {
                ref[1] = ptr->address;
                total += ptr->length;
                break;
            }
        }
        

        while(1){
            fgets(line, 100, fp);
            if(line[0] == 'R'){   // 'R' record
                idx = 1;
                while(1){
                    if(isEnd(line[idx]) || line[idx] == '\n' || idx >= strlen(line)){
                        break;
                    }
                    sscanf(line+idx, "%2X%6s", &refNum, symName);   // get reference number and symbol name
                    key = extabHash(symName);
                    for(ptr= externtable[key]; ptr; ptr= ptr->next){    // find appropriate member
                        if(strcmp(ptr->symName, symName) == 0){
                            break;
                        }
                    }
                    if (!ptr) {
                        printf("Error: there is no such symbol!!\n");
                        return -1;
                    }
                    ref[refNum] = ptr->address; // set reference number
                    idx +=8;
                }
            }else if(line[0] == 'T'){    // T record
                sscanf(line+1, "%6X%2X", &addr, &textLen);
                addr += csAddr; // add csAddr
                for(j=0; j < textLen * 2; j += 2){
                    sscanf(line + 9 + j, "%2X", &temp);
                    memory[addr + j/2] = (unsigned short int)temp;  // set memory value
                }
            }else if(line[0] == 'M'){
                sscanf(line+1, "%6X%2X%c%2X", &addr, &loc, &sign, &refNum);
                addr += csAddr;
                modAddr = ref[refNum];

                if(sign == '-'){    // if negative
                    modAddr *= -1;
                }

                if (loc == 0x05) {  // half byte
                    for (j = 0; j < 3; j++) {
                        sprintf(value+j*2, "%02X", memory[addr + j]);
                    }
                    value[7] = '\0';
                }
                else {
                    for (j = 0; j < 3; j++) {
                        sprintf(value+j*2, "%02X", memory[addr + j]);
                    }
                    value[7] = '\0';
                }

                if (value[0] == '8' || value[0] == '9' || (value[0] >= 'A' && value[0] <= 'F')) {   // negative
                    val = getHex(value, 3);
                    val = val + 0XFF000000;
                    val = val ^ (0XFFFFFFFF);
                    val += 1;
                    val *= -1;
                }
                else {
                    val = getHex(value, 3);
                }
                
                val += modAddr;
                
                sprintf(value, "%06X", val);
                if (val < 0) {
                    for (j = 0; j < 6; j += 2) {
                        sscanf(value + j+2, "%2X", &temp);
                        memory[addr + j / 2] = (unsigned short int)temp;
                    }
                }
                else {
                    for (j = 0; j < 6; j += 2) {
                        sscanf(value + j, "%2X", &temp);
                        memory[addr + j / 2] = (unsigned short int)temp;
                    }
                }
                
            }
            else if (line[0] == 'E') {
                break;
            }
            else{
                continue;
            }
        }

        csAddr += csLen;
        fclose(fp);
    }

    return total;

}


/* function : linkAndLoad()*/
/* purpose : Linking and Loading */
/* return : nothing */
void linkAndLoad(char* param, char command[]){
    char file[10];
    char input[3][10];
    ExternNode* ptr, *temp;
    int i, idx, size, fileCnt=0, totalLen = 0;
   
    idx = 0;
    clearExtab();   // clear extab

    while (!isEnd(*param)) {
        while (!isIndent(*param) && *param != '\0' && *param != '\n') {
            file[idx++] = *param;
            param++;
        }
        file[idx] = '\0';
        idx = 0;
        strcpy(input[fileCnt++], file);
        while (isIndent(*param)) {
            param++;
        }
    }

    size = linkLoadPass1(input, fileCnt);// pass1 
    if (size < 0) {
        return;
    }
    totalLen = linkLoadPass2(input, fileCnt);   // pass2 
    if (totalLen < 0) {
        return;
    }
    temp = (ExternNode*)malloc(sizeof(ExternNode) * size);  // for sort
    idx = 0;
    for (i = 0; i < 26; i++) {
        for (ptr = externtable[i]; ptr; ptr = ptr->next) {  // for sort 
            temp[idx].address = ptr->address;
            temp[idx].length = ptr->length;
            strcpy(temp[idx].progName, ptr->progName);
            strcpy(temp[idx].symName, ptr->symName);
            idx++;
        }
    }
    qsort(temp, size, sizeof(ExternNode), compare); // sort by address

    // print extern table
    printf("control\tsymbol\n");  
    printf("section\tname\taddress\tlength\n");
    printf("--------------------------------------------------------------\n");
    for (i = 0; i < size; i++) { 
         if (strcmp(temp[i].progName, "NULL")==0) {
             printf("      \t%-6s\t%04X\t\n", temp[i].symName, temp[i].address);  }
         else {
            printf("%-6s\t      \t%04X\t%04X\n", temp[i].progName, temp[i].address, temp[i].length);
         }    
    }
    printf("--------------------------------------------------------------\n");
    printf("\ttotal length %04X\n", totalLen);

    for (i = 0; i < 7; i++) {   // reset register
        reg[i] = 0;
    }
    fileLen = reg[2] = totalLen;  // set L register
    lastExecAddr = reg[8] = progAddr;  // set PC register
    
    free(temp);
    
}


/* function : setBp() */
/* purpose : set break point */
/* return : nothing */
void setBp(char* param, char command[]) {
    int i, val;

    if (*param == '\0' || *param == '\n') { // print all break points
        printf("\t\tbreakpoint\n");
        printf("\t\t----------\n");
        for (i = 0; i < 4600; i++) {
            if (bp[i]) {
                printf("\t\t%-4X\n", i);
            }
        }
    }
    else if (*param == 'c') {   // clear all break points
        for (i = 0; i < 4600; i++) {
            bp[i] = 0;
        }
        bpSize = 0;
        printf("\t\t [ok] clear all breakpoints\n");
    }
    else {  // set break point
        val = getHex(param, 1);
        bp[val] = true;
        bpSize++;
        printf("\t\t[ok] create breakpoint %4x\n", val);
    }

}

/* function : printReg() */
/* purpose : print register */
/* return : nothing */
void printReg() {
    // register A:0, X:1, L:2, PC:8, SW:9 B:3, S:4,T:5, F:6
    printf("A : %06X  X : %06X\n", reg[0], reg[1]);
    printf("L : %06X PC : %06X\n", reg[2], reg[8]);
    printf("B : %06X  S : %06X\n", reg[3], reg[4]);
    printf("T : %06X\n", reg[5]);

}

/* function : getTarget() */
/* purpose : get target address */
/* return : int target address */
int getTarget(int loc) {
    char xbpe[3], temp[7];
    int addr = 0, i;

    sprintf(xbpe, "%02X", memory[loc+1]);
    
    if (xbpe[0] == '1') {   // extended mode
        for (i = 1; i < 4; i++) {
            sprintf(temp + i * 2-2, "%02X", memory[loc + i]);
        }
        addr = getHex(temp + 1, 3);
    }
    else if (xbpe[0] == '2') {  // PC relative mode
        for (i = 1; i < 3; i++) {
            sprintf(temp+i*2-2, "%02X", memory[loc + i]);
        }

        if (temp[1] == '8' || temp[1] == '9' || (temp[1] >= 'A' && temp[1] <= 'F')) {   // negative
            addr = getHex(temp+1, 3);
            addr = addr + 0XFF000;
            addr = addr ^ (0XFFFFF);
            addr += 1;
            addr *= -1;
        }
        else {
            addr = getHex(temp + 1, 3);
        }
        

        addr += reg[8] + 3;
    }
    else if (xbpe[0] == 'C') {  // BASE relative mode
        for (i = 1; i < 3; i++) {
            sprintf(temp+i*2-2, "%02X", memory[loc + i]);
        }
        addr = getHex(temp + 1, 3);
        addr += reg[3];
    }
    else if (xbpe[0] == '0') {
        for (i = 1; i < 3; i++) {
            sprintf(temp + i * 2 - 2, "%02X", memory[loc + i]);
        }
        addr = getHex(temp + 1, 3);
    }
    else if (xbpe[0] == '4') {
        for (i = 1; i < 3; i++) {
            sprintf(temp + i * 2 - 2, "%02X", memory[loc + i]);
        }
        addr = getHex(temp + 1, 3);
        addr += reg[3];
    }

    return addr;

}


/* function : getVal() */
/* purpose : get value from memory */
/* return : int value */
int getVal(int addr, int loc) {
    int val = 0, tmp, i;
    char temp[10];

    tmp = memory[loc];
    tmp = tmp & 0X03;

    if (tmp == 0) { 
        // no need for this project
    }
    else if (tmp == 1) {    // imediate addressing
        val = addr;
    }
    else if (tmp == 2) {    // indirect addressing
        for (i = 1; i < 3; i++) {
            sprintf(temp + i * 2-2, "%02X", memory[addr + i]);
        }
        temp[4] = '\0';
        val = getHex(temp, 3);
        
    }
    else if (tmp == 3) {    // simple addressing
        for (i = 0; i < 3; i++) {
            sprintf(temp + i * 2, "%02X", memory[addr + i]);
        }
        temp[6] = '\0';
        val = getHex(temp, 3);
    }

    return val;

}


/* function : run() */
/* purpose : run program */
/* return : nothing */
void run(char* param, char command[]) {
    int i, loc, endAddr, opcode, opFlag, targetAddr, val;
    char xbpe[3], target[7];
    OpcodeNode* ptr;
    
    
    loc = reg[8];
    endAddr = progAddr + fileLen; // progAddr + program length
    
    while (reg[8] != endAddr) { // until PC == end address
        if (bp[loc]) {  // break point
            if (bpFlag) { 
                bpFlag = false;
            }
            else {
                bpFlag = true;
                printReg();
                printf("\t\tStop at checkpoint[%X]\n", loc);
                return;
            }
        }

        opFlag = 0;
        opcode = memory[loc] & 0XFC; // get opcode
        for (i = 0; i < 20; i++) {
            ptr = hashtable[i];     // opcode table
            while (ptr) {
                if (ptr->opcode == opcode) {
                    opFlag = 1;
                    break;
                }
                ptr = ptr->next;
            }
            if (opFlag == 1) {
                break;
            }
        }
        
        if (ptr->format[0] == '1') {  // format 1
            // update later ( no need for project 3 )
        }
        else if (ptr->format[0] == '2') {   // format 2
            if(strcmp(ptr->mnemonic, "CLEAR")==0) {
                sprintf(target, "%02X", memory[loc+1]);
                reg[target[0] - '0'] = 0;   // clear register
            }
            else if (strcmp(ptr->mnemonic, "COMPR") == 0) {
                sprintf(target, "%02X", memory[loc + 1]);
                reg[9] = reg[target[0] - '0'] >= reg[target[1] - '0'] ? reg[target[0] - '0'] == reg[target[1] - '0'] ? 0 : 1 : -1; // set reg SW
            
            }
            else if (strcmp(ptr->mnemonic, "TIXR") == 0) {
                sprintf(target, "%02X", memory[loc + 1]);
                reg[1]++; // inc reg x
                reg[9] = reg[1] >= reg[target[0] - '0'] ? reg[1] == reg[target[0] - '0'] ? 0 : 1 : -1;  // set reg SW

            }
            reg[8] += 2; // PC +=2
        }
        else if (ptr->format[0] == '3') { // format 3
            sprintf(xbpe, "%02X", memory[loc + 1]);
            targetAddr = getTarget(loc);
            
            if (strcmp(ptr->mnemonic, "STA") == 0) {
                sprintf(target, "%06X", reg[0]);    // store A register
                for (i = 0; i < 3; i++) {
                    sscanf(target + i * 2, "%2X", &val);
                    memory[targetAddr + i] = val;
                }
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "STL") == 0) {
                sprintf(target, "%06X", reg[2]);    // store L register
                for (i = 0; i < 3; i++) {
                    sscanf(target + i * 2, "%2X", &val);
                    memory[targetAddr + i] = val;
                }
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "STX") == 0) {
                sprintf(target, "%06X", reg[1]);    // store L register
                for (i = 0; i < 3; i++) {
                    sscanf(target + i * 2, "%2X", &val);
                    memory[targetAddr + i] = val;
                }
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "LDA") == 0 ) {
                val = getVal(targetAddr, loc);
                reg[0] = val;
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "LDB") == 0) {
                val = getVal(targetAddr, loc);
                reg[3] = val;
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "LDT") == 0) {
                val = getVal(targetAddr, loc);
                reg[5] = val;
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "J") == 0) {
                val = getVal(targetAddr, loc);
                reg[8] = val;
            }
            else if (strcmp(ptr->mnemonic, "JSUB") == 0) {
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
                reg[2] = reg[8];    // PC -> L
                reg[8] = targetAddr;    // m -> PC
            }
            else if (strcmp(ptr->mnemonic, "JEQ") == 0) {
                if (reg[9] == 0) {  // if CC is '='
                    reg[8] = targetAddr;    // m -> PC
                }
                else {
                    if (xbpe[0] == '1') {
                        reg[8] += 4;
                    }
                    else {
                        reg[8] += 3;
                    }
                }

            }
            else if (strcmp(ptr->mnemonic, "JLT") == 0) {
                if (reg[9] == -1) {  // if CC is '<'
                    reg[8] = targetAddr;    // m -> PC
                }
                else {
                    if (xbpe[0] == '1') {
                        reg[8] += 4;
                    }
                    else {
                        reg[8] += 3;
                    }
                }
            }
            else if (strcmp(ptr->mnemonic, "RSUB") == 0) {
                reg[8] = reg[2];    // L -> PC
            }
            else if (strcmp(ptr->mnemonic, "COMP") == 0) {
                val = getVal(targetAddr, loc);  // compare A register and value
                reg[9] = val >= reg[0] ? val == reg[0] ? 0 : 1 : -1;
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "TD") == 0) {
                reg[9] = -1;    // set CC '<'
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "RD") == 0) {
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "WD") == 0) {
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "STCH") == 0) {
                memory[targetAddr] = reg[0];
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }
            else if (strcmp(ptr->mnemonic, "LDCH") == 0) {
                reg[0] = memory[targetAddr+2];
                if (xbpe[0] == '1') {
                    reg[8] += 4;
                }
                else {
                    reg[8] += 3;
                }
            }

        }
        else {  // format 4
            // update later ( no need for project 3 )
        }

        loc = reg[8];   // next instruction 
    }

    printReg();
    printf("\t\tEnd program\n");
    
    bpFlag = false;

    return;
}
