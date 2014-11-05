#include <time.h>
#include "hvrslib.h"
#include "hvop.h"
#include "cmddef.h"

#define DEV_BAUD    115200
#define DEV_TIMEOUT 100

void CmdSleep(double t)
{
	struct timespec tm;
	tm.tv_sec = t;
	tm.tv_nsec = (t - tm.tv_sec) * 1E9;
	nanosleep(&tm, NULL);
}


int DevFD = -1;                                 // Port file descriptor

// Open port and set port parameters. Return >=0 on success, <0 on error.
int DevOpen(char *dev)
{
    hvrs_CalibrateDelay();
    DevFD = hvrs_Open((dev) ? dev : "/dev/ttyS0", DEV_BAUD, DEV_TIMEOUT);
    return DevFD;
}

// close port
void DevClose(void)
{
    if (DevFD >= 0) hvrs_Close(DevFD);
    DevFD = -1;
}

// get 4-bytes board status. Return 0xFFFFFFFF on error
unsigned int GetStatus(unsigned char addr)
{
    int rc;
    unsigned int res;
    if (DevFD < 0) return 0xFFFFFFFF;
    rc = hvrs_Command(DevFD, addr, CMD_STAT, 0);    // global status & errors
    if (rc < 0) return 0xFFFFFFFF;
    res = rc & 0xFFFF;
    rc = hvrs_Command(DevFD, addr, CMD_STAT, 1);    // channel status
    if (rc < 0) return 0xFFFFFFFF;
    res |= rc << 16;
    return res;
}

// Get ADC value. Return unsigned 16-bit value on success. -1 on error.
int ReadADC(unsigned char addr, int num)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_ADC, num);
    if (rc < 0) return -1;
    return rc;
}

// Get DAC set value. Return unsigned 16-bit value on success. -1 on error.
int ReadDAC(unsigned char addr, int num)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_ADC, num + ADC_SET);
    if (rc < 0) return -1;
    return rc;
}

// Set DAC value. Return 0 on success, -1 on error.
int WriteDAC(unsigned char addr, int num, int val)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_SET, num | (val << 16));
    if (rc < 0) return -1;
    return 0;
}

// switch on channel number num (0..14). Return 0 on success, -1 on error.
int SwitchOn(unsigned char addr, int num)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_ON, 1 << (num + 16));
    if (rc < 0) return -1;
    return 0;
}

// switch off channel number num (0..14). Return 0 on success, -1 on error.
int SwitchOff(unsigned char addr, int num)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_OFF, 1 << (num + 16));
    if (rc < 0) return -1;
    return 0;
}

// switch LV On. Return 0 on success, -1 on error.
int LVOn(unsigned char addr)                   
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_ON, ON_LV);
    if (rc < 0) return -1;
    return 0;
}

// switch LV On. Return 0 on success, -1 on error.
int LVOff(unsigned char addr)                   
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_OFF, ON_LV);
    if (rc < 0) return -1;
    return 0;
}

// switch HV On. Return 0 on success, -1 on error.
int HVOn(unsigned char addr)                   
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_ON, ON_HV);
    if (rc < 0) return -1;
    return 0;
}

// switch HV Off. Return 0 on success, -1 on error.
int HVOff(unsigned char addr)                   
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_OFF, ON_HV);
    if (rc < 0) return -1;
    return 0;
}

// switch everything off and clear unprotected EEPROM
int CmdClear(unsigned char addr)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_CLEAR, 0);
    if (rc < 0) return -1;
    return 0;
}

// Do hard reset via watchdog
int CmdReset(unsigned char addr)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_RESET, 0);
    if (rc < 0) return -1;
    return 0;
}

// Read 2 bytes from EEPROM
int EEPRead(unsigned char addr, unsigned char eaddr)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_EEP, E_READ | (eaddr << 8));
    if (rc < 0) return -1;
    return rc;
}

// Write 2 bytes to EEPROM
int EEPWrite(unsigned char addr, unsigned char eaddr, unsigned short data)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_EEP, E_WRITE | (eaddr << 8) | (data << 16));
    if (rc < 0) return -1;
    return 0;
}

// Read EEPROM status register
int EEPRDSR(unsigned char addr)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_EEP, E_RDSR);
    if (rc < 0) return -1;
    return rc;
}

// Write EEPROM status register
int EEPWRSR(unsigned char addr, unsigned char data)
{
    int rc;
    rc = hvrs_Command(DevFD, addr, CMD_EEP, E_WRSR | (data << 16));
    if (rc < 0) return -1;
    return 0;
}

