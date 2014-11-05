#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include "cmddef.h"
#include "hvrslib.h"

#define HVRS_TIMEOUT	100
#define HVRS_BAUD	115200

void Usage(void) {
    printf("Usage: hvrstool [device]\n");
    printf("All numbers are hex. The tool sends 7 bytes commands and gets 4 bytes responses.\n");
    printf("Default device is --> /dev/ttyS0\n");
    printf("Enter Q to quit. All other lines are interpreted as <addr> <cmd> <data>.\n");
    printf("<addr> and <cmd> - 8 bits. <data> - 32 bits.\n");
    printf("Special <cmd> = U - dUmp flash memory.\n");
    printf("Special <cmd> = P sernum - program protected flash.\n");
    printf("\n");
}

void FlProg(int fd, unsigned char addr, unsigned int snum)
{
	int i, irc;
	unsigned short eeprom[32];
	unsigned short csum;

	printf("Rewriting protected EEPROM @ 0x%2.2X memory with S/N = 0x%X (%d)\n", addr, snum, snum);
//		make block
	for (i=0; i<32; i++) eeprom[i] = 0xFFFF;	// empty
	eeprom[24] = snum & 0xFF;
	eeprom[25] = snum & 0xFFFF;
	eeprom[26] = 0;	// no 1-wire yet
	csum = 0;
	for (i=0; i<31; i++) csum += eeprom[i];
	eeprom[31] = 0x55AA - csum;
//		program
	irc = hvrs_Command(fd, addr, 0x20, 3);	// clear flags - eneble write to protected area
	if (irc < 0) {
		printf("HVRS error!\n");
		return;
	}
	for (i=0; i<32; i++) {
		irc = hvrs_Command(fd, addr, 0x20, (eeprom[i] << 16) | (i << 9) | 0xC001);
		if (irc < 0) printf("EEProm write error\n"); 
	}
	printf("Written... clear and reset.\n");
	hvrs_Command(fd, addr, 0x21, 0);	// clear unprotected part
	sleep(1);
	hvrs_Command(fd, addr, 0x22, 0);	// hardware reset
}

void FlDump(int fd, unsigned char addr)
{
	unsigned short csum;
	int irc;
	int i;

	printf("Unprotected block:\n");
	csum  = 0;
	for (i=0; i<0xC0; i += 2) {
		if ((i & 0x1E) == 0) printf("%2.2X :", i);
		irc = hvrs_Command(fd, addr, 0x20, i << 8);
		if (irc < 0) {
			printf("Device error. - Wrong addr: %X ?\n", (int)addr);
			break;
		}
		csum += irc & 0xFFFF;
		printf(" %4.4X", irc);
		if ((i & 0x1E) == 0x1E) printf("\n");
	}
	printf("Unprotected sum: %4.4X\n", csum);

	printf("Protected block:\n");
	csum  = 0;
	for (i=0xC0; i<0x100; i += 2) {
		if ((i & 0x1E) == 0) printf("%2.2X :", i);
		irc = hvrs_Command(fd, addr, 0x20, i << 8);
		if (irc < 0) {
			printf("Device error. - Wrong addr: %X ?\n", (int)addr);
			break;
		}
		csum += irc & 0xFFFF;
		printf(" %4.4X", irc);
		if ((i & 0x1E) == 0x1E) printf("\n");
	}
	printf("Protected sum: %4.4X\n", csum);
}

int main(int argc, char ** argv) 
{
    int fd;
    unsigned char addr, cmd;
    unsigned int data;
    char *dev = "/dev/ttyS0";
    int irc;
    char *line;
    char *tok;
    const char DELIM[] = " \t";

    fd = -1;
    addr = 0;
    cmd = 0;
    data = 0;
    line = NULL;
    
    if (argc > 1) dev = argv[1];

    fd = hvrs_Open(dev, HVRS_BAUD, HVRS_TIMEOUT);
    if (fd < 0) {
	Usage();
	hvrs_Close(fd);
	return -10;
    }
    
    for(;;) {
	line = readline("Enter <addr> <cmd> <data> or Q(uit)> ");
	if (line && line[0]) add_history(line);
	if (!line || toupper(line[0]) == 'Q') break;
	tok = strtok(line, DELIM);
	if (tok) addr = strtoul(tok, NULL, 16) & 0xFF;
	tok = strtok(NULL, DELIM);
	if (tok && toupper(tok[0]) == 'U') {
		FlDump(fd, addr);
		continue;
	}
	if (tok && toupper(tok[0]) == 'P') {
		data = 0;		
		tok = strtok(NULL, DELIM);
		if (tok) data = strtoul(tok, NULL, 16);
		if (data > 0 && data < 0x10000) {
			FlProg(fd, addr, data);
		} else {
			printf("Serial number is mandatory !\n");
		}
		continue;
	}
	if (tok) cmd = strtoul(tok, NULL, 16) & 0xFF;
	tok = strtok(NULL, DELIM);
	if (tok) data = strtoul(tok, NULL, 16);
	free(line);
	printf("Sending %2.2X %2.2X %8.8X --> ", addr, cmd, data);
	irc = hvrs_Command(fd, addr, cmd, data);
	if (irc < 0) {
	    printf("Error = %d\n", irc);
	} else {
	    printf("%4.4X\n", irc);
	}
    }
    
    if (line) free(line);
    hvrs_Close(fd);
    printf("Good bye.\n");
}
