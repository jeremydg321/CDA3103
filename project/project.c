/*Akash Samlal
 *Jeremy Galvan 
 *CDA 3103C - MySPIM Simulation Project*/

#include "spimcore.h"
#include <stdio.h>

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {
//ALUControl determines what operationof the instruction set the ALU perform
switch ((int)ALUControl) {
	//ALU Control: 000 (0)
	//Z (ALUresult) = A + B
	case 0:
		*ALUresult = A + B;
		break;
	//ALU Control: 001 (1)
	//Z (ALUresult) = A - B
	case 1:
		*ALUresult = A - B;
		break;
	//ALU Control: 010 (2)
	//Z (ALUresult) = 1 if signed A < signed B unless Z (ALUresult) = 0
	case 2:
		if ((signed)A < (signed)B) 
			*ALUresult = 1;
		else 
			*ALUresult = 0;
		break;
	//ALU Control: 011 (3)
	//Z (ALUresult) = 1 if unsigned A < unsigned B unless Z (ALUresult) = 0
	case 3:
		if ((unsigned)A < (unsigned)B) 
			*ALUresult = 1;
		else 
			*ALUresult = 0;
		break;
	//ALU Control: 100 (4)
	//Z (ALUresult) = A AND B
	case 4:
		*ALUresult = A & B;
		break;
	//ALU Control: 101 (5)
	//Z (ALUresult) = A OR B
	case 5:
		*ALUresult = A | B;
		break;
	//ALU Control: 110 (6)
	//Shift left B by 16 bits 
	case 6:
		*ALUresult = B << 16;
		break;
	//ALU Control: 111 (7) 
	//Z (ALUresult) = ~A
	case 7:
		*ALUresult = ~A;
		break;
	}
	//If the ALUresult is "0", assign "1" to Zero, otherwise assign "0" to Zero
	if (*ALUresult == 0) 
		*Zero = 1;
	else 
		*Zero = 0;
}
/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
	//In order for the datapath to advance continuously, the PC
	//must update by the adder by adding to 4 to fetch the instruction from the memory, written on every clock cycle
	//right shift the Mem index by 2, quicker way than multiply by 4
	if (PC % 4 == 0) {
		*instruction = Mem[PC >> 2];
		return 0;
	}
	else {
	//Simulation is halted, illegal encountered:
	//Jumping to an address that is not word-alligned, not a multiple of 4 
		return 1;
	}
}
/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec) {
	/*Partition the instruction set into several parts 
	 *Mask and Shift to the appropriate amount
	   * op: instruction [31-26]
	   * r1: instruction [25-21]
	   * r2: instruction [20-16]
	   * r3: instruction [15-11]
	   * funct: instruction [5-0]
	   * offset: instruction [15-0]
	   * jsec: instruction [25-0]
	*/
	*op = (instruction >> 26) & 0x0000003f;
	*r1 = (instruction >> 21) & 0x1f;
	*r2 = (instruction >> 16) & 0x1f;
	*r3 = (instruction >> 11) & 0x1f;
	*funct = instruction & 0x0000003f;
	*offset = instruction & 0x0000ffff;
	*jsec = instruction & 0x03ffffff;
}
/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
	//Initalize 0 to all child attributes to control
	//Only change the necessary child attributes for the op code
	controls->RegDst = 0;
	controls->Jump = 0; 
	controls->Branch = 0; 
	controls->MemRead = 0;
	controls->MemtoReg = 0; 
	controls->ALUOp = 0; 
	controls->MemWrite = 0;
	controls->ALUSrc = 0; 
	controls->RegWrite = 0;

   switch (op) {
	//Enabled for all of R-Type Instructions: 
	//R-Type Instructions: Addition, Subtraction, Or, (Signed) Set Less Than, (Unsigned) Set Less Than
	case 0:
		controls->RegDst = 1;
		controls->ALUOp = 7;
		controls->RegWrite = 1; 
		break;
	//Enabled for I-Type Instruction: Add Immediate
	case 8:
		controls->ALUSrc = 1;
		controls->RegWrite = 1;
		break;
	//Enabled for I-Type Instruction: Signed Set Less Than Immediate
	case 10:
		controls->RegDst = 1;
		controls->RegWrite = 1;
		controls->ALUOp = 2;
		break;
    //Enabled for I-Type Instruction: Unsigned Set Less Than Immediate
	case 11:
		controls->RegDst = 1;
		controls->RegWrite = 1;
		controls->ALUOp = 3;
		break;
	//Enabled for I-Type Instruction: Branch Equal To
	case 4:
		controls->RegDst = 2;
		controls->Branch = 1;
		controls->MemtoReg = 2;
		controls->ALUOp = 1;
		break;
    //Enabled for I-Type Instruction: Load Word 
	case 35:
		controls->MemRead = 1;
		controls->MemtoReg = 1;
		controls->ALUSrc = 1;
		controls->RegWrite = 1;
		break;
	//Enabled for I-Type Instruction: Unsigned Load Immediate 
	case 15:
		controls->ALUOp = 6;
		controls->ALUSrc = 1; 
		controls->RegWrite = 1;
		break;
	//Enabled for I-Type Instruction: Stored Word
	case 43:
		controls->RegDst = 2; 
		controls->MemtoReg = 2;
		controls->MemWrite = 1;
		controls->ALUSrc = 1;
		break;
	//Enabled for J-Type Instruction: Jump
	case 2:
		controls->Jump = 1;
		break;
   //Simulation is halted if illegal field is encountered
	default:
		return 1;
	}
   //Gracefully exit, no halt in the simulation
	return 0; 
}
/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2) {
	//Data1 & Data2 assigned the values from the register of index r1 & r2
	*data1 = Reg[r1];
	*data2 = Reg[r2]; 
}
/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value) {
	//Set Offset to negative  
	unsigned negSign = offset >> 15; 
	//Expand the value to 32 bits if negative, with 1's value is true 
	if (negSign == 1) 
		*extended_value = offset | 0xFFFF0000;
	//Retain at 16 bits, if not negative, with 0's
	else 
		*extended_value = offset & 0x0000FFFF;
}
/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero) {
	//Data is using the extended value from ALUSrc  
	if ((int)ALUSrc == 1)
		data2 = extended_value;
	//R-Type Instruction execute based on instruction set
	if ((int)ALUOp == 7) {
	//Determine the instruction by funct
      switch (funct) {
      //Add in R-Type
        case 32:
			ALUOp = 0; 
		    break;
   	  //Subtract in R-Type
		case 34:
			ALUOp = 1;
			break;
      //Signed Less Than in R-Type
		case 42:
			ALUOp = 2;
			break;
      //Unsigned Less Than in R-Type
		case 43:
			ALUOp = 3;
			break; 
 	  //And in R-Type
		case 36:
			ALUOp = 4;
			break; 
	  //Or in R-Type
		case 37:
			ALUOp = 5;
			break; 
	  //Shift in R-Type 
		case 4:
			ALUOp = 6;
			break; 
   	  //Not in R-Type
		case 39:
			ALUOp = 7;
			break; 
		//Simulation is halted if illegal field is encountered
		default:
			return 1;
		}
	  //Data processed by the ALU
		ALU(data1, data2, ALUOp, ALUresult, Zero);
	}
	//Since it's not an R-type instruction, there is no instruction function field
	//Continue on the Simulation by passing through the function
	else
		ALU(data1, data2, ALUOp, ALUresult, Zero);
   //Gracefully exit, no halt in the simulation
	return 0;	   
}
/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {       
	//set index to word alignment 
	unsigned index = ALUresult >> 2;
	//Read from the next word alignment in memory
	if ((int)MemRead == 1) {
		if (ALUresult % 4 == 0) 
			*memdata = Mem[index];
		//Simulation is halted, illegal encountered
		else
			return 1; 
	}
	//Write to the next word alignment in memory
	if ((int)MemWrite == 1) {
		if (ALUresult % 4 == 0) 
			Mem[index] = data2;
		//Simulation is halted, illegal encountered
		else 
			return 1; 	
	}
	//Gracefully exit, no halt in the simulation
	return 0; 
}
/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg) {
	//If writing to the register is true
	//Either write the value of ALUresult or memdata
	//to the register addressed by r3 or r2
	if ((int)RegWrite == 1) {
		switch (MemtoReg) {
		//Data from the register
		case 0 :
			//R-type instruction stored in register
			if (RegDst == 1)
				Reg[r3] = ALUresult;
			//I-type instruction, stored in index r2, RegDst is 0
			else
				Reg[r2] = ALUresult; 
			break; 
		//Data from the memory, stored to the register
		case 1:
			Reg[r2] = memdata; 
			break; 
		}
	}
}
/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
	//In order for the datapath to continously advance
	//The program counter must be updated by the adder by being added by the value of 4 
	*PC += 4; 

	//Add the extended value from the offset if branching 
	//As well Zero is true, multiply extended value by 4 to the PC for word alignment 
	if ((int)Branch == 1) {
		if ((int)Zero == 1) 
			*PC += extended_value << 2;
	}
	//Jump is assured, multiply by 4 and jump to the jump register  
	if ((int)Jump == 1)
		*PC = (*PC & 0xf0000000) | (jsec << 2);
}