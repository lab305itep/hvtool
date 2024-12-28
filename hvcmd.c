#include <stdio.h>
#include <string.h>
#include "cmddef.h"
#include "hvop.h"

int main(int argc, char **argv)
{
	int fd;
	int addr;
	char strID[20];
	int i, val;
	unsigned int uval;

	if (argc < 4) {
		printf("USAGE: %s dev addr cmd [args]\n", argv[0]);
		printf("Commands:\n");
		printf("getID - returns 64-bits thermometer chip ID\n");
		printf("getStatus - returns board status\n");
		printf("hvOFF - switch HV off\n");
		printf("hvON - switch HV on\n");
		printf("lvOFF - switch LV off\n");
		printf("lvON - switch LV on\n");
		printf("readADC num - read ADC value @ num\n");
		printf("readDAC num - read DAC value @ num\n");
		printf("setDAC num val - set DAC value @ num\n");
		printf("switchOFF num - switch channel num off\n");
		printf("switchON num - switch channel num on\n");
		return 10;
	}

	fd = DevOpen(argv[1]);
	if (fd < 0) {
		printf("ERROR: can not open %s: %m\n", argv[1]);
		return 20;
	}

	addr = strtol(argv[2], NULL, 0);
	if (!strcasecmp(argv[3], "getID")) {
		for (i=0; i<4; i++) {
			val = ReadADC(addr, ADC_T1W + i + 1);
			if (val < 0) {
				goto fail;
			} else {
				sprintf(&strID[4*i], "%2.2X%2.2X", val & 0xFF, (val >> 8) & 0xFF);
			}
		}
		printf("ID=%s\n");
	} else if (!strcasecmp(argv[3], "getStatus")) {
		uval = GetStatus(addr);
		if (uval == 0xFFFFFFFF) goto fail;
		printf("STATUS=%8.8X\n", uval);
	} else if (!strcasecmp(argv[3], "hvOFF")) {
		val = HVOff(addr);
		if (val < 0) goto fail;
	} else if (!strcasecmp(argv[3], "hvON")) {
		val = HVOff(addr);
		if (val < 0) goto fail;
	} else if (!strcasecmp(argv[3], "lvOFF")) {
		val = LVOff(addr);
		if (val < 0) goto fail;
	} else if (!strcasecmp(argv[3], "lvON")) {
		val = LVOn(addr);
		if (val < 0) goto fail;
	} else if (!strcasecmp(argv[3], "readADC")) {
		i = (argc > 4) ? strtol(argv[4], NULL, 0) : 0;
		val = ReadADC(addr, i);
		if (val < 0) goto fail;
		printf("ADC[%d]=%d\n", val);
	} else if (!strcasecmp(argv[3], "readDAC")) {
		i = (argc > 4) ? strtol(argv[4], NULL, 0) : 0;
		val = ReadDAC(addr, i);
		if (val < 0) goto fail;
		if (val & 0x8000) val |= 0xFFFF0000;	// sign extension
		printf("DAC[%d]=%d\n", val);
	} else if (!strcasecmp(argv[3], "setDAC")) {
		i = (argc > 4) ? strtol(argv[4], NULL, 0) : 0;
		val = (argc > 5) ? strtol(argv[5], NULL, 0) : 0;
		val &= 0xFFFF;
		val = WriteDAC(addr, i, val);
		if (val < 0) goto fail;
	} else if (!strcasecmp(argv[3], "switchOFF")) {
		i = (argc > 4) ? strtol(argv[4], NULL, 0) : 0;
		val = SwitchOff(addr, i);
		if (val < 0) goto fail;
	} else if (!strcasecmp(argv[3], "switchON")) {
		i = (argc > 4) ? strtol(argv[4], NULL, 0) : 0;
		val = SwitchOn(addr, i);
		if (val < 0) goto fail;
	}

	DevClose();
	return 0;
fail:
	printf("ERROR: communication failed: %m\n");
	DevClose();
	return 100;
}
