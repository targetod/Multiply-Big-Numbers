/*
It is assumed that MCU width is 32bit
thats why multiply code can be optimised.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "multiply.h"

#define MAX_LEN 20


int main()
{
	uint16_t error = 0;
	char fname_num1[MAX_LEN] = "test1.txt";
	char fname_num2[MAX_LEN] = "test2.txt";
	char fname_result[MAX_LEN] = "result.txt";

	uint16_t MCU_width = WIDTH32;
	
	error = unsigned_multiply(fname_num1, fname_num2, fname_result, MCU_width);
	if (error != 0) {
		printf(" Error: multiply!\n");
		return 1;
	}
	else {
		printf(" Check you result in file!\n");
	}

	return 0;
}