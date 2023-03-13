#include "20160641.h"


int main(){
    int quitFlag = 0;
    progAddr = 0x00;    // starting program address
    int hashFlag = makeHashTable();    // make hashtable from "opcode.txt" 
    while(!quitFlag){
        char input[100];
        char* param;
        int command;
       
        printf("sicsim> ");
        fgets(input, sizeof(input), stdin); // get command from user
        input[strlen(input)-1] = '\0';
        param = getCommand(input, &command);    // check command. if command is valid, get command number and parmeter(if there is)

        switch (command)
        {
        case 1 : // help
            help(param, input);
            break;
        case 2 : // dir
            directory(param, input); 
            break;
        case 3 : // quit
            quitFlag = 1;
            break;
        case 4 : // history
            showHistory(param, input);
            break;
        case 5 : // dump
            dump(param, input);
            break;
        case 6 : // edit
            edit(param, input);
            break;
        case 7 : // fill
            fill(param, input);
            break;
        case 8 : //reset
            reset(param, input);
             break;
        case 9 : // opcode
            if(hashFlag == -1){
                printf("you can`t use \"opcode mnemonic\" , \"opcodelist\" \n");
                break;
            }
            opcode(param, input);
            break;
        case 10 : // opcode_list
            opcodelist(param, input);
            break;
        case 11 : // type
            type(param, input);
            break;
        case 12 : // assemble
            assemble(param, input);
            break;
        case 13 :   // symbol
            printSymbol(param, input);
            break;
        case 14 :   // progaddr
            progAddress(param, input);
            break;
        case 15 :   // loader 
            linkAndLoad(param, input);
            break;
        case 16 :    // bp
            setBp(param, input);
            break;
        case 17 : // run
            run(param, input);
            break;
        default:
            printf("Error: invalid command!\n");
            break;
        }
    }

    clearSymtab();
    clearExtab();

    return 0;

}