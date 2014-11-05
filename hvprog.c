/*
    HVDAC initial program and test utility
*/

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cmddef.h"
#include "hvop.h"

#define NRETRIES	5
#define NPOINTS		5
#define NSAMPLES	5
#define VSETMIN		0.0
#define VSETMAX		10.0
#define LOGFILE		"HVDAC_test.log"

double Calculate(int N, double *X, double *Y, double *A, double *B)
{
	int i;
	double sx, sy, sxy, sx2, sy2;
	double C;

	sx = 0;
	sy = 0;
	sxy = 0;
	sx2 = 0;
	sy2 = 0;
	for (i=0; i<N; i++) {
		sx += X[i];
		sy += Y[i];
		sxy += X[i] * Y[i];
		sx2 += X[i] * X[i];
		sy2 += Y[i] * Y[i];
	}
	sx /= N;
	sy /= N;
	sxy /= N;
	sx2 /= N;
	sy2 /= N;
	*B = (sxy - sx*sy) / (sx2 - sx*sx);
	*A = sy - (*B)*sx;
	C = sy2 + (*A)*(*A) + (*B)*(*B)*sx2 + 2*((*A)*(*B)*sx - (*A)*sy - (*B)*sxy);
	C = sqrt(C * N / (N - 2));
	return C;
}

void Log(int num, char *msg, ...)
{
	FILE *fLog;
	char *tmr;
	time_t tm;
	va_list arg;
	
	fLog = fopen(LOGFILE, "a");
	if (!fLog) {
		printf("Log file %s open error.\n", LOGFILE);
		return;	
	}
	tm = time(NULL);
	tmr = ctime(&tm);
	tmr[strlen(tmr) - 1] = '\0';
	fprintf(fLog, "%s - SN=%5d: - ", tmr, num);
	va_start(arg, msg);
	vfprintf(fLog, msg, arg);
	va_end(arg);
	fprintf(fLog, "\n");
	fclose(fLog);
}

void Sleep(double t)
{
	struct timespec tm;
	tm.tv_sec = t;
	tm.tv_nsec = (t - tm.tv_sec) * 1E9;
	nanosleep(&tm, NULL);
}

int main(int argc, char **argv)
{
	int brd = 255;	// unprogrammed board address
	int snum;
	char str[1024];
	char *dev;
	int irc, ival;
	int i, j, k;
	double dval;
	double vADC[NPOINTS];
	double vFLK[NPOINTS];
	double A, B, C;
	unsigned short EEPROM[32];

//		Starting	
	printf("\t===== HVDAC test and program utility =====\n");
	if (argc < 2) {
	    printf("Usage: hvprog <com-port-device> [<board-address>]\n"
			   "Eaxmples of <com-port-device> are:\n"
		       "\t/dev/ttyS0 --- PC 1st com-port\n"
		       "\t/dev/ttyUSB0 --- USB adapter 1st com-port\n"
		       "Use <board-address> to reporgram already programmed board\n");
	}
	printf("Ensure that everything is connected:\n"
	       "\tPower and communication connector is plugged to the board and power is ON;\n"
	       "\tFluka is connected to HV pins and is in DC Voltage mode.\n"
	       "Input board serial number and press <Enter> to continue. <Enter> to quit\n");
	fgets(str, sizeof(str), stdin);
	if (strlen(str) <= 1 || strcasestr(str, "Q")) return 0;
	snum = strtoul(str, NULL, 0);
//		Open the device
	dev = argv[1];
	if (argc > 2) brd = strtol(argv[2], NULL, 0) & 0xFF;
	irc = DevOpen(dev);
	if (irc < 0) {
		printf("Can not open com-port %s\n"
		       "Verify that the device name is correct and\n"
		       "if you are using USB-COM adapter it is connected.\n"
		       "Exitting...\n", dev);
		return 10;
	}
// 		Set everything OFF and clear unprotected EEPROM area
	for (i=0; i<NRETRIES; i++) {
		irc = CmdClear(brd);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not clear/access the board %d.\n"
		       "Verify that the board is connected, powered\n"
		       "and its address is correct.\n"
		       "New board should have address 255.\n"
		       "Exitting...\n", brd);
		Log(snum, "BAD - no access");
		return 20;
	}
	Sleep(0.5);
//		Turn On low voltage
	for (i=0; i<NRETRIES; i++) {
		irc = LVOn(brd);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not switch low voltage ON. Unstable communication.\n"
		       "Check cables and try again. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
	Sleep(1.0);		// wait for LV to set
//		Verify low voltage
	for (i=0; i<NRETRIES; i++) {
		irc = ReadADC(brd, ADC_PAMV);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not check low voltage. Unstable communication.\n"
		       "Check cables and try again. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
	dval = 5 * (irc / 65536.0 - 1.0);
	if (dval < -4.5 || dval > -3.5) {
		printf("Low voltage %7.2f V is out of range (-4.5 - -3.5) V.\n"
		       "Mark the board as bad. Exitting...\n", dval);
		Log(snum, "BAD - low voltage out of range");
		return 40;
	}
	printf("Low voltage is %7.2f V ... OK.\n", dval);
//		Switch On High voltage
	for (i=0; i<NRETRIES; i++) {
		irc = WriteDAC(brd, 15, 0x7FFF);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not set HV DAC. Unstable communication.\n"
		       "Check cables and try again. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
	for (i=0; i<NRETRIES; i++) {
		irc = HVOn(brd);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not switch high voltage ON. Unstable communication.\n"
		       "Check cables and try again. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
	Sleep(5.0);		// wait for HV to set
//		Verify HV 
	for (i=0; i<NRETRIES; i++) {
		irc = ReadADC(brd, ADC_HVMV);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not check high voltage. Unstable communication.\n"
		       "Check cables and try again. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
	dval = irc * 0.001034; 		// 2.5 * 97.6 / 3.6 / 65536
	if (dval < 62 || dval > 72) {
		printf("Maximum high voltage %7.2f V is out of range (62-72) V.\n"
		       "Mark the board as bad. Exitting...\n", dval);
		Log(snum, "BAD - high voltage is out of range");
		return 40;
	}
	printf("Maximum high voltage is %7.2f V ... OK.\n", dval);
//		Verify HV current
	for (i=0; i<NRETRIES; i++) {
		irc = ReadADC(brd, ADC_HVMI);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not check high voltage current. Unstable communication.\n"
		       "Check cables and try again. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
	dval = irc * 0.000463; 		// 2.5 * 5 / 0.412 / 65536
	if (dval > 10) {
		printf("High voltage current %7.2f uA is too high > 10 uA.\n"
		       "Check Fluka connection and try again. "
		       "Mark the board as bad if the problem persists. Exitting...\n", dval);
		Log(snum, "BAD - HV current too high");
		return 40;
	} else if (dval < 5) {
		printf("High voltage current %7.2f uA is too small < 5 uA.\n"
		       "Check Fluka connection and try again. "
		       "Exitting...\n", dval);
		return 40;
	}
	printf("High voltage current is %7.2f uA ... OK.\n", dval);
//		Getting calibration information
	for (j=0; j<NPOINTS; j++) {
		ival = 3276.8 * (VSETMIN * j + VSETMAX * (NPOINTS - j)) / NPOINTS;
		if (ival > 32767) ival = 32767;
		for (i=0; i<NRETRIES; i++) {
			irc = WriteDAC(brd, 15, ival);
			if (irc >= 0) break;
		}
		if (irc < 0) {
			printf("Can not set HV DAC. Unstable communication.\n"
			       "Check cables and try again. Exitting ...\n");
			Log(snum, "BAD - unstable communication");
			return 30;
		}
		Sleep(1.0);
		printf("Enter Voltage read by Fluka: ");
		fgets(str, sizeof(str), stdin);
		if (strlen(str) == 0 || strcasestr(str, "Q")) return 0;
		vFLK[j] = strtod(str, NULL);
		dval = 0;
		for (k=0; k < NSAMPLES; k++) {
			for (i=0; i<NRETRIES; i++) {
				irc = ReadADC(brd, ADC_HVMV);
				if (irc >= 0) break;
			}
			if (irc < 0) {
				printf("Can not set HV DAC. Unstable communication.\n"
				       "Check cables and try again. Exitting ...\n");
				Log(snum, "BAD - unstable communication");
				return 30;
			}
			dval += irc;
			Sleep(0.2);
		}
		vADC[j] = dval/NSAMPLES;
	}
	C = Calculate(NPOINTS, vADC, vFLK, &A, &B);
	B *= 1000;
	if (C > 0.05) {
		printf("Bad calibration precision RMS = %7.3f V > 0.05 V.\n"
		       "Check connections and try again. Exitting ...\n", C);
		Log(snum, "BAD - unreasonable calibration");
		return 50;
	} else if (fabs(A) > 2) {
		printf("Strange offset = %6.3f V > 2 V.\n"
		       "Check connections and try again. Exitting ...\n", A);
		Log(snum, "BAD - unreasonable calibration");
		return 50;
	} else if (fabs(B - 1) > 0.2) {
		printf("Strange slope %7.4g. Check connections and try again. Exitting ...\n", B);
		Log(snum, "BAD - unreasonable calibration");
		return 50;
	}
	printf("Calibration done: offset = %6.3f V, slope = %7.4f mV/unit RMS = %f V\n", A, B, C);
//		Verify DAC settings
	for (j=0; j<15; j++) {
		ival = 0x8000 + 3276.8 * j;
		ival &= 0xFFFF;
		for (i=0; i<NRETRIES; i++) {
			irc = WriteDAC(brd, j, ival);
			if (irc >= 0) break;
		}
		if (irc < 0) {
			printf("Can not set DAC. Unstable communication.\n"
			       "Check cables and try again. Exitting ...\n");
			Log(snum, "BAD - unstable communication");
			return 30;
		}
		for (i=0; i<NRETRIES; i++) {
			irc = SwitchOn(brd, j);
			if (irc >= 0) break;
		}
		if (irc < 0) {
			printf("Can not switch on a DAC output. Unstable communication.\n"
			       "Check cables and try again. Exitting ...\n");
			Log(snum, "BAD - unstable communication");
			return 30;
		}
	}
	printf("Verify DAC output voltages at the output connector, please.\n"
	       "Channels 1 to 15 should have increasing voltages from -10.0 to 4.0 V\n"
	       "with 1 V step. Values should be within 0.05 V from the nominal.\n"
	       "Check outputs one by one with Fluka. Is everything OK? (yes/NO): ");
	fgets(str, sizeof(str), stdin);
	if (!strcasestr(str, "YES")) {
		printf("DAC outputs verification failed. Mark the board as bad. Exitting...\n");
		Log(snum, "BAD - DAC outputs failed.");
		return 60;
	}
//		Create protected EEPROM block
	memset(EEPROM, -1, sizeof(EEPROM));		// set to 0xFF
	ival =  1000 * A;
	EEPROM[(EE_HVCA - EE_PBEGIN) / sizeof(short)] = ival & 0xFFFF;
	ival = 50000 * B;
	EEPROM[(EE_HVCB - EE_PBEGIN) / sizeof(short)] = ival & 0xFFFF;
	EEPROM[(EE_ADDR - EE_PBEGIN) / sizeof(short)] = snum & 0xFF;
	EEPROM[(EE_SERIAL - EE_PBEGIN) / sizeof(short)] = snum & 0xFFFF;
	EEPROM[(EE_CFG - EE_PBEGIN) / sizeof(short)] = 0;
	ival = 0;
	for (i = 0; i < sizeof(EEPROM) / sizeof(short) - 1; i++) ival += EEPROM[i];
	EEPROM[(EE_PSUM - EE_PBEGIN) / sizeof(short)] = (0x55AA - ival) & 0xFFFF;
//		Clear protection
	for (i=0; i<NRETRIES; i++) {
		irc = EEPWRSR(brd, 0);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not write EEPROM status. Unstable communication.\n"
			   "Check cables and try again. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
//		Write protected area
	for (j=0; j < sizeof(EEPROM) / sizeof(short); j++) {
		for (i=0; i<NRETRIES; i++) {
			irc = EEPWrite(brd, EE_PBEGIN + j * sizeof(short), EEPROM[j]);
			if (irc >= 0) break;
		}
		if (irc < 0) {
			printf("Can not write to EEPROM. Unstable communication.\n"
			       "Check cables and try again. Exitting ...\n");
			Log(snum, "BAD - unstable communication");
			return 30;
		}
	}
//		Do hard reset
	for (i=0; i<NRETRIES; i++) {
		irc = CmdReset(brd);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not reset the board. Unstable communication.\n"
			   "Check cables and try again using old and new addresses. Exitting ...\n");
		Log(snum, "BAD - unstable communication");
		return 30;
	}
	Sleep(1.0);
// 		Set everything OFF and clear unprotected EEPROM area - check answer at the new address
	brd = snum & 0xFF;
	for (i=0; i<NRETRIES; i++) {
		irc = CmdClear(brd);
		if (irc >= 0) break;
	}
	if (irc < 0) {
		printf("Can not get answer at the new address. Something was wrong.\n"
			   "Check cables and try again using old and new addresses. Exitting ...\n");
		Log(snum, "BAD - does not came back after programming");
		return 70;
	}
//		Done...	
	printf("The board %d programmed OK.\n", snum);
	Log(snum, "GOOD - A=%7.3f V B=%7.4f mV/unit", A, B); 
	return 0;
}

