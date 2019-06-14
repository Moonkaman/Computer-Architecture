#include "cpu.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define DATA_LEN 1024

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *fileName)
{
  // char data1[DATA_LEN] = {
  //     // From print8.ls8
  //     0b10000010, // LDI R0,8
  //     0b00000000,
  //     0b00001000,
  //     0b01000111, // PRN R0
  //     0b00000000,
  //     0b00000001 // HLT
  // };

  // for(int i = 0; i < DATA_LEN; i++){
  //   printf("%d\n", data1[i]);
  // }

  // exit(0);

  FILE *fp;
  char data[DATA_LEN];
  int address = 0;

  fp = fopen(fileName, "r");

  if (fp == NULL)
  {
    fprintf(stderr, "File not found\n");
    exit(1);
  }

  while (fgets(data, DATA_LEN, fp) != NULL)
  {
    char *endptr;
    unsigned char v = strtoul(data, &endptr, 2);

    if (endptr == data)
    {
      continue;
    }
    // printf("%d\n", v);
    cpu->ram[address] = v;
    address++;
  }

  fclose(fp);

  // for (int i = 0; i < DATA_LEN; i++)
  // {
  //   cpu->ram[address++] = data[i];
  // }

  // TODO: Replace this with something less hard-coded
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op)
  {
  case ALU_MUL:
    printf("%d\n", cpu->registers[regA] * cpu->registers[regB]);
    break;

  case ALU_ADD:
    cpu->registers[regA] = cpu->registers[regA] + cpu->registers[regB];
    break;

  case ALU_CMP:
    if (cpu->registers[regA] == cpu->registers[regB])
    {
      cpu->FL = 0b00000001;
    }
    else if (cpu->registers[regA] > cpu->registers[regB])
    {
      cpu->FL = 0b00000010;
    }
    else if (cpu->registers[regA] < cpu->registers[regB])
    {
      cpu->FL = 0b00000100;
    }
  }
}

void cpu_ram_write(struct cpu *cpu, int index, int value)
{
  cpu->ram[index] = value;
}

int cpu_ram_read(struct cpu *cpu, int index)
{
  return cpu->ram[index];
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction
  int IR;
  int operandA;
  int operandB;
  int inc;

  while (running)
  {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    IR = cpu_ram_read(cpu, cpu->PC);
    // 2. Figure out how many operands this next instruction requires
    // 3. Get the appropriate value(s) of the operands following this instruction
    operandA = cpu_ram_read(cpu, cpu->PC + 1);
    operandB = cpu_ram_read(cpu, cpu->PC + 2);
    int numOperands = (IR >> 6) + 1;
    inc = 1;
    // cpu->FL = 0b00000000;
    // 4. switch() over it to decide on a course of action.
    // printf("PC: %d IR: %d FL: %d CPU Registers: [%d, %d, %d, %d, %d, %d, %d, %d]\n", cpu->PC + 1, IR, cpu->FL, cpu->registers[0], cpu->registers[1], cpu->registers[2], cpu->registers[3], cpu->registers[4], cpu->registers[5], cpu->registers[6], cpu->registers[7]);
    switch (IR)
    {
    case LDI:
      cpu->registers[operandA] = operandB;
      break;

    case PRN:
      printf("%d\n", cpu->registers[operandA]);
      break;

    case HLT:
      running = 0;
      break;

    case MUL:
      alu(cpu, ALU_MUL, operandA, operandB);
      break;

    case PUSH:
      cpu->registers[SP]--;
      cpu->ram[cpu->registers[SP]] = cpu->registers[operandA];
      break;

    case POP:
      cpu->registers[operandA] = cpu->ram[cpu->registers[SP]];
      cpu->registers[SP]++;
      break;

    case CALL:
      inc = 0;
      cpu->registers[SP]--;
      cpu->ram[cpu->registers[SP]] = cpu->PC + numOperands;
      cpu->PC = cpu->registers[operandA];
      // printf("cpu->PC = %d\n", cpu->PC);
      break;

    case RET:
      cpu->PC = cpu->ram[cpu->registers[SP]];
      cpu->registers[SP]++;
      inc = 0;
      break;

    case ADD:
      alu(cpu, ALU_ADD, operandA, operandB);
      break;

    case CMP:
      alu(cpu, ALU_CMP, operandA, operandB);
      break;

    case JMP:
      inc = 0;
      cpu->PC = cpu->registers[operandA];
      break;

    case JEQ:
      inc = 0;
      if (cpu->FL & 0b00000001)
      {
        cpu->PC = cpu->registers[operandA];
      }
      else
      {
        inc = 1;
      }
      break;

    case JNE:
      inc = 0;
      if (cpu->FL ^ 0b00000001)
      {
        cpu->PC = cpu->registers[operandA];
      }
      else
      {
        inc = 1;
      }
      break;

    default:
      printf("Error on instruction %d\n", cpu->PC);
      exit(0);
      break;
    }
    // 5. Do whatever the instruction should do according to the spec.
    // 6. Move the PC to the next instruction.
    if (inc)
    {
      cpu->PC += numOperands;
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->PC = 0;
  memset(cpu->ram, 0, 256);
  memset(cpu->registers, 0, 8);
  cpu->registers[SP] = 0xf4;
  cpu->FL = 0b00000000;
}
