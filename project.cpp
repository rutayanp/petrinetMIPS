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

struct ib_packet
{
    string ib_instruction;
    int ib_flag;
};

class decode
{
    ib_packet pre_buffer;

public:
    void fetch(int pc)
    {
       pre_buffer.ib_instruction = imem[pc];
    }

    void decod()
    {
        //ib_packet pre_buffer;
        stringstream src;
        //string ib_instruction;
        string opcode, dest, src1, src2;
        int operands[2], ib_flag = 0;

        opcode = pre_buffer.ib_instruction.substr(1,3);
        dest = pre_buffer.ib_instruction.substr(5,2);
        operands[0] = (int)pre_buffer.ib_instruction.at(9) - 48; //get the int value of operands
        operands[1] = (int)pre_buffer.ib_instruction.at(12) - 48;//get the int value of operands
        
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
        //return pre_buffer;
    }

    ib_packet decode_to_ib(int pc)
    {
        fetch(pc);
        decod();
        return pre_buffer;
    }

    //WRITE IM DISPLAY
};

class issue
{
private:
    string sb_pre_buf;
    string mb_pre_buf;
    int issue1_flag;
    int issue2_flag;
public:
    
    void issue_route(string instruction)
    {
        issue2_flag = instruction.compare(1,3,"MUL");
        if(issue2_flag != 0)
            issue2_flag = instruction.compare(1,3,"DIV");

        issue1_flag = instruction.compare(1,3,"ADD");
        if(issue1_flag != 0)
            issue1_flag = instruction.compare(1,3,"SUB");
    
        if(issue1_flag == 0)
            issue1_flag = 1;
        if(issue2_flag == 0)
            issue2_flag = 1;
    }
    
    void ib_sb(ib_packet post_ib_buffer)
    {
        if(issue1_flag == 1){
            sb_pre_buf = post_ib_buffer.ib_instruction;
        } else {
            sb_pre_buf = "";
        }
    }

    void ib_mb(ib_packet post_ib_buffer)
    {
        if(issue2_flag == 1){
            mb_pre_buf = post_ib_buffer.ib_instruction;
        } else {
            mb_pre_buf = "";
        }
    }

    vector<string> ib_2_mb_and_sb (ib_packet post_ib_buf)
    {
        vector<string> sb_mb_buf;
        
        issue_route(post_ib_buf.ib_instruction);
        
        ib_sb(post_ib_buf);
        ib_mb(post_ib_buf);
        
        sb_mb_buf.push_back(sb_pre_buf);
        sb_mb_buf.push_back(mb_pre_buf);

        return sb_mb_buf;
    }
};

class ASU
{
private: 
    int result;
    int operation; //(0)no-op & (1)ADD & (2)SUBTRACTION
    int src1, src2;
    string src1_str, src2_str;
    int dest;
    int i;
public:

    void decode(string instruction)
    {
        if(instruction.length() > 3){
            if(instruction.compare(1,3,"ADD") == 0)
                operation = 1;
            else if(instruction.compare(1,3,"SUB") == 0)
                operation = 2;
        
        for (i = 8; instruction.at(i) != ','; i++){
            src1_str.push_back(instruction.at(i));
        }

        for (i = ++i; instruction.at(i) != '>';i++){
            src2_str.push_back(instruction.at(i));
        }    
        src1 = atoint(src1_str);
        src2 = atoint(src2_str);
        dest = atoint(instruction.substr(6,1));
        //cout << "src1 : " << src1 << ", "<< "src2 : " << src2 <<", " << dest <<endl; 
        } else {
            operation = 0;
        }
    }

    vector<int> as_operate(string instruction)
    {
        decode(instruction);
        vector<int> res;
        if (operation == 0){
            result = 0;
            dest = 9;
        } else if(operation == 1){
            result = src1 + src2;
        } else if(operation == 2){
            result = src1 - src2;
        } 
        res.push_back(dest); // 0th index of res has the DESTINATION
        res.push_back(result); // 1th index has the result value
        return res;
    }
};

int main()
{
    int i;
    vector<string> sb_mb_buf;
    string instruction;
    ib_packet ib_pre_buf, ib_post_buf;
    
    imemory();
    regfile();
    
    decode d1;
    issue issue1_2;
    ASU as1;
    //SIMULATION BEGIN
    ib_pre_buf = d1.decode_to_ib(1);
    if(DEBUG > 2){
        cout << "=========" << endl;
        cout << "ib_instruction : " << ib_pre_buf.ib_instruction << endl;
    }

    ib_post_buf = ib_pre_buf; //buffer transition

    sb_mb_buf = issue1_2.ib_2_mb_and_sb(ib_post_buf);
    if(DEBUG > 2){
        cout << "sb: " << sb_mb_buf[0] << endl;
        cout << "mb: " << sb_mb_buf[1] << endl;
    }
    
    as1.as_operate(sb_mb_buf[0]);
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
