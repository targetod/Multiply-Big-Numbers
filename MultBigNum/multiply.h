/*
TECHNICAL TASK was not accurate determinate. 
Time, memory for calculation, type of MCU, RTOS are not determinate.

If you would write code for microcontrollers
you should to know how many memory is existing and 
what is the MCU width to efficiently writing code.

Also we can use malloc/calloc and free functions if we write for RTOS.
I'm not using malloc() in this prog because it is harmful in embedded systems.

That's why
This code works only with files without malloc.
Need files with only digit (without any symbols '\n' , ' ', ...).  

Code was writting in Visual Studio 2015 in C-style.
*/

#pragma once
#ifndef _MULT_H_
#define _MULT_H_

// 5 for 32bit
// 3 for 16bit
// 2 for 8bit
#define MAX_BUFF_SIZE 5 

// for check MCU width
enum { WIDTH32 = 32, WIDTH16 = 16, WIDTH8 = 8 };

// multiply big unsigned numbers
// return 0 if all right
uint16_t unsigned_multiply(const char* fname_num1,
							const char* fname_num2,
							const char* fname_result,
							uint16_t MCU_width);


// internal function for multiply big unsigned numbers
// return 0 if all right
static uint16_t multiply(FILE* fNum1, FILE* fNum2, FILE* fResult,
	uint16_t file_size1, uint16_t file_size2, uint16_t calc_byte);


// this function return number which is N bytes from file
// its like matrix in the stack
// 2222 3333 | 3333 4444 | 4444 1111,  i =1 j=0   
// portions + 1 is num of packets in one line
static uint32_t get_number(uint16_t i, uint16_t j, uint16_t bytes,
	uint16_t portions, FILE* file);


// return bytes for read biggest number from file for calculate 
// for 32-bit MCU  biggest number is 9999 
// 9999 * 9999 = 99980001  => near 10^8.  uint32_t => near 4*10^9
// we can mult without overflow
// return 4 for 32bit, 2 for 16bit, else 1
static uint16_t get_bytes_for_calc(uint16_t MCU_width);

// we have result of multiply reverse to original data
// thats why we should to reverse data
// 1111 2222   ->  2222 1111
static uint16_t reverse_data (FILE* file, uint16_t bytes, uint16_t portions);


// internal function for add first zeros in file
// it's need for algorithm
// 1 2222 ->   00001 2222
// return 0 if all right
static uint16_t add_zero_to_file(const char* file_name, uint16_t num_zero);


// internal function for delete first zeros in file
// wich  apear after algorithm
// 0000 0001 2222 ->   1 2222
// return 0 if all right
static uint16_t delete_zero_in_file(const char* file_name);



#endif // !_MULT_H_
