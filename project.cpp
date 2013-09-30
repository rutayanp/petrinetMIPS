#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>

#define DEBUG 3

using namespace std;

vector<string> imem;
vector<string> init_reg;
int rf[8][3];//rf[n][0] returns the value of Rn and rf[n][1] returns the validity of Rn and rf[n][7] returns if Rn is written or not

int num_inst = 0;

void imemory();
void regfile();
int atoint(string);

class decode
{
    string instruction;
    struct ib_packet
    {
        string ib_instruction;
        int ib_flag;
    };

public:
    string fetch(int pc)
    {
        instruction = imem[pc];
        return instruction;
    }

    ib_packet decod(string instruction)
    {
        ib_packet pre_buffer;
        stringstream src;
        string ib_instruction;
        string opcode, dest, src1, src2;
        int operands[2], ib_flag = 0;

        opcode = instruction.substr(1,3);
        dest = instruction.substr(5,2);
        operands[0] = (int)instruction.at(9) - 48; //get the int value of operands
        operands[1] = (int)instruction.at(12) - 48;//get the int value of operands

        src << rf[operands[0]][0];
        src1 = src.str();
        src.str(string());//clear the string stream

        src << rf[operands[1]][0];
        src2 = src.str();
        src.str(string());//clear the string stream

        if (rf[operands[0]][1] == 0 && rf[operands[1]][1] == 0){
            pre_buffer.ib_instruction = "<" + opcode + "," + dest + "," + src1 + "," + src2 + ">";
            pre_buffer.ib_flag = 1;
        } else {
            pre_buffer.ib_flag = 0;
            pre_buffer.ib_instruction = "";
        }
        return pre_buffer;
    }

    void decode_to_ib(int pc)
    {
        ib_packet pre_buffer;
        pre_buffer.ib_instruction = fetch(pc);
        pre_buffer = decod(pre_buffer.ib_instruction);
        if(DEBUG > 2){
            cout << "=========" << endl;
            cout << "ib_instruction : " << pre_buffer.ib_instruction << endl;
        }
    }
};

int main()
{
    int i;
    string instruction;
    imemory();
    regfile();
    decode d1;
    
    return 0;
}

void imemory()
{
    string instruction;
    int i;
    fstream ins_file ("instructions.txt");
    if (ins_file.is_open()){
        while(getline (ins_file, instruction)){
            imem.push_back(instruction);
        }
        ins_file.close();
    }

    if(DEBUG > 2){
        cout << "=========" << endl << "Instructions" << endl;
        for(i = 0; i < imem.size(); i++){
            cout << imem[i] << endl;
        }
        cout << "=========" << endl;
    }
}

void regfile()
{
    string reg, str_val;
    int i;
    int index, value;
    fstream reg_file("registers.txt");
    
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 3; j++){
            rf[i][j] = 0;
        }
    }
    
    if(reg_file.is_open()){
        while(getline(reg_file, reg)){
            init_reg.push_back(reg);
        }
        reg_file.close();
    }

    if(DEBUG > 2){
        cout <<"Registers" << endl;
        for(i = 0; i < init_reg.size(); i++){
            cout << init_reg[i] << endl;
        } 
        cout << "=========" << endl;
    }
    
    for(i = 0; i < init_reg.size(); i++){
        index = (int)init_reg[i].at(2) - 48; 
        for (int j = 4; j < init_reg[i].length() - 1; j++){
            str_val.push_back(init_reg[i].at(j)); 
        }
        value = atoint (str_val);
        //if( DEBUG > 2)
          //  cout << "reg R" << index << " = " << value << endl;
        str_val.clear();
    
        rf[index][0] = value;
        rf[index][1] = 0;
        rf[index][2] = 1;

        if(DEBUG > 2){ //The values initially feeded in register file
            cout << "R" << index << "= ";
            cout << "value : "<< rf[index][0]  << ", in_use : " << rf[index][1] << ", modified : " << rf[index][2] << endl;
        }
    }
}

int atoint (string str)
{
    int c;
    int value = 0;
    for(int i = 0 ; i < str.length(); i++){
       c = (int)str.at(i) - 48;
       value = value * 10 + c; 
    } 
    return value;
}
