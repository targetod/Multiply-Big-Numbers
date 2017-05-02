
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "multiply.h"


uint16_t unsigned_multiply(  const char* fname_num1,
							const char* fname_num2,
							const char* fname_result,
							uint16_t MCU_width)
{
	FILE* fNum1=NULL, *fNum2=NULL, *fResult=NULL;
	uint16_t file_size1 = 0;
	uint16_t file_size2 = 0;
	int16_t num_zero = 0;
	int16_t error = 0;
	uint16_t calc_byte = 0; // bytes for read from file for calculate  

	fNum1 = fopen(fname_num1, "rb");
	if (fNum1 == NULL) {
		printf("Error open file\n");
		return 1;
	}
	fNum2 = fopen(fname_num2, "rb");
	if (fNum2 == NULL) {
		printf("Error open file\n");
		return 1;
	}

	fResult = fopen(fname_result, "w+b");
	if (fResult == NULL) {
		printf("Error create file\n");
		return 1;
	}

	// check size of files
	fseek(fNum1, 0, SEEK_END);
	file_size1 = ftell(fNum1);
	fseek(fNum1, 0, SEEK_SET);

	fseek(fNum2, 0, SEEK_END);
	file_size2 = ftell(fNum2);
	fseek(fNum2, 0, SEEK_SET);

	fclose(fNum1);
	fclose(fNum2);

	if (0 == file_size1 || 0 == file_size2)
	{
		printf("Error size file\n");
		return 1;
	}

	calc_byte = get_bytes_for_calc(MCU_width);
	
	// need to align data in the files for my algorithm
	// set additional zero
	// calc num of zero
	num_zero = file_size1 - file_size2;
	if (num_zero > 0) { // need to set zero in file2
		error = add_zero_to_file(fname_num2, num_zero);
	}
	else if (num_zero < 0) {// need to set zero in file1
		error = add_zero_to_file(fname_num1, -num_zero);
	}
	else {
		// all right
	}

	if (error != 0) {
		printf("Error: fail add zeros to file\n");
		return 1;
	}
	//------------------------------------------
	// open for calc
	fNum1 = fopen(fname_num1, "rb");
	if (fNum1 == NULL) {
		printf("Error open file\n");
		return 1;
	}
	fNum2 = fopen(fname_num2, "rb");
	if (fNum2 == NULL) {
		printf("Error open file\n");
		return 1;
	}

	// check size of files again . two file must have the same size
	fseek(fNum1, 0, SEEK_END);
	file_size1 = ftell(fNum1);
	fseek(fNum1, 0, SEEK_SET);

	error = multiply(fNum1, fNum2, fResult, file_size1, file_size1, calc_byte);
	if (error != 0){
		printf("Error:  multiply in internal function \n");
		return 1;
	}


	fclose(fNum1);
	fclose(fNum2);
	fclose(fResult);

	error = delete_zero_in_file(fname_result);
	if (error != 0) {
		printf("Error:  delete zeros in file \n");
		return 1;
	}

	return 0;
}


static uint16_t multiply(FILE* fNum1, FILE* fNum2, FILE* fResult, 
	uint16_t file_size1, uint16_t file_size2, uint16_t calc_byte)
{
	uint16_t error=0;
	uint32_t num1 = 0; // number from file 1
	uint32_t num2 = 0; // number from file 2
	char buff[MAX_BUFF_SIZE] = {};
	// alternative:
	// char * buff = (char*)calloc(calc_byte+1, sizeof(char));
	uint32_t result = 0, rest = 0;
	uint32_t num_to_next_num = 0;
	uint32_t num_to_file = 0;
	uint32_t base = (uint32_t)pow(10, calc_byte);
	uint16_t portions1 = 0;
	uint16_t portions2 = 0;
	uint16_t portions = 0; // parts in the tmp file
	uint32_t line_size = 0;
	int16_t i = 0, j = 0, k = 0;
	//calculate how many portions of calc_byte in the files
	portions1 = file_size1 / calc_byte;
	portions1 = (file_size1 % calc_byte) ? portions1 + 1 : portions1;
	portions2 = file_size2 / calc_byte;
	portions2 = (file_size2 % calc_byte) ? portions2 + 1 : portions2;


	// file for temporary operations
	FILE *ftmp = fopen("tmp.txt", "w+b");
	if (ftmp == NULL) {
		printf("Error create tmp file\n");
		return 1;
	}

	for ( i = portions2-1; i >= 0; --i) {
		num_to_next_num = 0;
		for ( j = portions1-1; j >= 0; j--)
		{
			num1 = get_number(0, j, calc_byte, file_size1, fNum1);
			num2 = get_number(0, i, calc_byte, file_size2, fNum2);

			result = num2 * num1;
			result += num_to_next_num;
			rest = result % base; // % 10^4 for 32bit
			num_to_next_num = result / base; //   / 10^4 for 32bit

			num_to_file = rest;
			sprintf(buff, "%0*d", calc_byte, num_to_file);
			fwrite(buff, sizeof(char), calc_byte, ftmp);
		}
		sprintf(buff, "%0*d", calc_byte, num_to_next_num);
		fwrite(buff, sizeof(char), calc_byte, ftmp);
		
	}
	// get line size of arr in ftmp 
	line_size = (portions1+1)*calc_byte;

	// get num of operations
	portions = (portions1 + portions2)/2;
	

	// start to calc sum in column
	num_to_next_num = 0;
	for ( k = 0; k < portions; k++)
	{
		result = 0;
		for ( i = 0, j = k; i < portions && j >= 0; ++i, --j) {
			result += get_number(i, j, calc_byte, line_size, ftmp); 
		}
		result += num_to_next_num;

		num_to_next_num = result / base;
		result %= base;

		sprintf(buff, "%0*d", calc_byte, result);
		fwrite(buff, sizeof(char), calc_byte, fResult);
	}

	for ( k = 0; k < portions; k++)
	{
		result = 0;
		for ( i = k, j = portions; i < portions && j >= 0; ++i, --j) {
			result += get_number(i, j, calc_byte, line_size, ftmp);
		}
		result += num_to_next_num;

		num_to_next_num = result / base;
		result %= base;

		sprintf(buff, "%0*d", calc_byte, result);
		fwrite(buff, sizeof(char), calc_byte, fResult);
	}

	error = reverse_data(fResult, calc_byte, portions1+portions2);
	if (error != 0) {
		printf("Error in funclion reverse  data\n");
		return 1;
	}

	
	return 0;
}



 static uint32_t get_number( uint16_t i, uint16_t j, uint16_t bytes,
							uint16_t bytes_in_line,  FILE* file )
{
	uint16_t size = 0;
	uint16_t rest_bytes = 0;
	char buff[MAX_BUFF_SIZE] = {};
	
	rest_bytes = bytes_in_line % bytes;
	// check the last byte from file 
	// we can have  11 2222 3333 ->   last num has 2 digits
	
	if (0 == rest_bytes) {
		// move to i and j position in file
		fseek(file, bytes_in_line*i, SEEK_SET);
		fseek(file, j*bytes, SEEK_CUR);

		fread(buff, sizeof(char), bytes, file);
	}
	else {
		fseek(file, bytes_in_line*i, SEEK_SET);
		if (j == 0) {
			fread(buff, sizeof(char), rest_bytes, file);
		}
		else {
			fseek(file, rest_bytes, SEEK_CUR);
			fseek(file, (j - 1)*bytes, SEEK_CUR);
			fread(buff, sizeof(char), bytes, file);
		}
	}

	return atol(buff);
}


static uint16_t get_bytes_for_calc(uint16_t MCU_width)
{
	uint32_t bytes=0;
	
		switch (MCU_width) {
		case WIDTH32:
			bytes = 4;
			break;
		case WIDTH16:
			bytes = 2;
			break;
		case WIDTH8:
			bytes = 1;
			break;
		default:
			bytes = 1;
			break;
		}

	return bytes;
}

static uint16_t reverse_data(FILE* file, uint16_t bytes, uint16_t portions)
{
	char buff1[MAX_BUFF_SIZE] = {};
	char buff2[MAX_BUFF_SIZE] = {};
	char buff_tmp[MAX_BUFF_SIZE] = {};

	uint16_t i = 0;
	uint16_t num_of_swaps = portions /2;


	for (i = 0; i < num_of_swaps; ++i) {
		// set and get left 
		fseek(file, i * bytes, SEEK_SET);
		fread(buff1, sizeof(char), bytes, file);
		// set and get right 
		fseek(file, -(i+1)*bytes, SEEK_END);
		fread(buff2, sizeof(char), bytes, file);
		
		// reverse
		strcpy(buff_tmp, buff1);
		strcpy(buff1, buff2);
		strcpy(buff2, buff_tmp);
		
		// write
		fseek(file, i * bytes, SEEK_SET);
		fwrite(buff1, sizeof(char), bytes, file);
		fseek(file, -(i + 1)*bytes, SEEK_END);
		fwrite(buff2, sizeof(char), bytes, file);
	}

	return 0;
}

static uint16_t add_zero_to_file(const char* file_name, uint16_t num_zero) {
	FILE* ftmp = NULL, *file = NULL ;
	uint8_t c = 0;
	uint16_t error = 0;
	const char * pftmp_name = "tmp.txt";
	int16_t i = 0;

	ftmp = fopen(pftmp_name, "wb");
	if (ftmp == NULL) {
		printf("Error create tmp file\n");
		return 1;
	}
	file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("Error create  file\n");
		return 1;
	}

	// set zeros
	for ( i = 0; i < num_zero; i++)
	{
		fputc('0', ftmp);
	}
	
	// copy
	while (true) {
		c = fgetc(file);
		if (feof(file)) {
			break;
		}
		fputc(c, ftmp);
	}
	// close file resources
	fclose(ftmp);
	fclose(file);

	error = remove(file_name);
	if (error != 0) {
		printf("Error: unable to delete the file");
		return 1;
	}

	error = rename(pftmp_name, file_name);
	if (error != 0){
		printf("Error: unable to rename the file");
		return 1;
	}
	
	return 0;
}

static uint16_t delete_zero_in_file(const char* file_name) {

	FILE* ftmp = NULL, *file = NULL;
	uint8_t c = 0;
	uint16_t error = 0;
	const char * pftmp_name = "tmp_zeros.txt";

	ftmp = fopen(pftmp_name, "wb");
	if (ftmp == NULL) {
		printf("Error create tmp file\n");
		return 1;
	}
	file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("Error create  file\n");
		return 1;
	}

	// pass all first '0' in file 
	do {
		c = fgetc(file);
	} while (c == '0');

	// copy to tmp
	while (true) {
		fputc(c, ftmp);
		c = fgetc(file);
		if (feof(file)) {
			break;
		}
	}
	// close file resources
	fclose(ftmp);
	fclose(file);

	error = remove(file_name);
	if (error != 0) {
		printf("Error: unable to delete the file");
		return 1;
	}

	error = rename(pftmp_name, file_name);
	if (error != 0) {
		printf("Error: unable to rename the file");
		return 1;
	}

	return 0;

}