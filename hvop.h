#ifndef HVOP_H
#define HVOP_H

#ifdef __cplusplus
extern "C" {
#endif

void CmdSleep(double t);			// sleep some time using nanosleep. t - seconds
int DevOpen(char *dev);                         // Open port
void DevClose(void);                            // close port
unsigned int GetStatus(unsigned char addr);     // get 4-bytes board status
int ReadADC(unsigned char addr, int num);       // Get ADC value
int ReadDAC(unsigned char addr, int num);       // Get DAC value
int WriteDAC(unsigned char addr, int num, int val); // Set DAC value
int SwitchOn(unsigned char addr, int num);      // switch on channel number num (0..14)
int SwitchOff(unsigned char addr, int num);     // switch off channel number num (0..14)
int LVOn(unsigned char addr);                   // switch LV On
int LVOff(unsigned char addr);                  // switch LV Off
int HVOn(unsigned char addr);                   // switch HV On
int HVOff(unsigned char addr);                  // switch HV Off
int CmdClear(unsigned char addr);				// switch everything off and clear unprotected EEPROM
int CmdReset(unsigned char addr);				// Do hard reset via watchdog
int EEPRead(unsigned char addr, unsigned char eaddr);	// Read 2 bytes from EEPROM
int EEPWrite(unsigned char addr, unsigned char eaddr, unsigned short data);	// Write 2 bytes to EEPROM
int EEPRDSR(unsigned char addr);				// Read EEPROM status register
int EEPWRSR(unsigned char addr, unsigned char data);	// Write EEPROM status register

#ifdef __cplusplus
}
#endif

#endif
