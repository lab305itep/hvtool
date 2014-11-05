#ifndef HVRSLIB_H
#define HVRSLIB_H

#ifdef __cplusplus
extern "C" {
#endif

// Opens  a connection on serial device <dev> ("/dev/ttyS0" etc) with speed <baud> bps
// Sets <timeout> for read operations in ms, if 0, sets default EPRS_TIMEOUT
// Return: file descriptor on success, -1 otherwise
int hvrs_Open(char *dev, int baud, int timeout);

// Closes an open line
void hvrs_Close(int fd);

// Transmits command <cmd> to the device <addr> on line <fd> with data of length <dlen>
// Returns the device 2-byte reply as an int
// Negative on errors or no reply
int hvrs_Command(int fd, unsigned char addr, unsigned char cmd, unsigned int data);

// Delay in us using program cycling 
void hvrs_uDelay(int us);
// Calibrate the delay. Should be called once
void hvrs_CalibrateDelay(void);

#ifdef __cplusplus
}
#endif

#endif
