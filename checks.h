
int isEnd(char c){
    if(c == EOF || c == '\0' || c == '\n'){
        return 1; 
    }
    
    return 0;
}

int isAlphabet(char c){
    if( (c >='a'&& c<'z') || ( c>='A' && c<='Z') ){
        return 1;
    }

    return 0;
}

int isIndent(char c){
    if( c== ' ' || c == '\t' ){
        return 1;
    }

    return 0;
}

int isDecimal(char c){
    if( c >= '0' && c <='9'){
        return 1;
    }

    return 0;
}

int isDelimiter(char c){
    if( c ==' ' || c== '\t' || c == '\n' || c== ','){
        return 1;
    }

    return 0;
}

int isDirective(char str[]){
    if(strcmp("START", str)==0 || strcmp("BASE", str) ==0 || strcmp("END", str)==0 || strcmp("BYTE", str)==0 || strcmp("WORD", str)==0 || strcmp("RESB", str)==0 || strcmp("RESW", str)==0){
        return 1;
    }

    return 0;
}

int isPrefix(char c){
    if(c== '@' || c=='#' || c == '+'){
        return 1;
    }

    return 0;
}

int isReg(char c) {
    if (c == 'A' || c == 'X' || c == 'L' || c == 'P' || c == 'S' || c == 'B' || c == 'T' || c == 'F') {
        return 1;
    }
    return 0;
}

int regTable(char str[]) {
    if (strcmp("A", str) == 0) {
        return 0;
    }
    else if (strcmp("X", str) == 0) {
        return 1;
    }
    else if (strcmp("L", str) == 0) {
        return 2;
    }
    else if (strcmp("PC", str) == 0) {
        return 8;
    }
    else if (strcmp("SW", str) == 0) {
        return 0;
    }
    else if (strcmp("B", str) == 0) {
        return 3;
    }
    else if (strcmp("S", str) == 0) {
        return 4;
    }
    else if (strcmp("T", str) == 0) {
        return 5;
    }
    else if (strcmp("F", str) == 0) {
        return 6;
    }
    else {
        return -1;
    }
}

int strToInt(char str[]) {
    int dec = 0;
    for (int i = 0; i < strlen(str); i++) {
        dec *= 10;
        dec += str[i] - '0';
    }

    return dec;
}