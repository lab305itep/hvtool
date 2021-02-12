#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "hvrslib.h"

/****************************************************************
 * Command structure (7 bytes ALWAYS!):                         *
 * 1. Address byte with 9th bit set                             *
 * 2. Command byte (this and all following with 9th bit clear)  *
 * 3-6. 4 data bytes                                            *
 * 7. Control sum, so all 7 bytes sum as 0xFF                   *
 * ------------------------------------------------------------ *
 * Reply structure (4 bytes always!):                           *
 * 1. Replier address with 9th bit set                          *
 * 2-3. 2 data bytes (command dependent)                        *
 * 4. Control sum, so all 4 bytes sum as 0xFF                   *
 ****************************************************************/

float usperc = 1.4;

int hvrs_Open(char *dev, int baud, int timeout) {
    int fd, ret;
    struct termios hvrs_attr;
    fd = open(dev, O_RDWR);
    if (fd < 0) {
	printf("HVRS-Open-ERR: Failed to open %s : %s\n", dev, strerror(errno));
	return -1;
    }
    ret = tcgetattr(fd, &hvrs_attr);
    if (ret != 0) {
	printf("HVRS-Open-ERR: Failed to get %s attributes : %s\n", dev, strerror(errno));
	return -1;
    }
    hvrs_attr.c_iflag &= ~(PARMRK|ISTRIP|BRKINT|IGNCR|ICRNL|INLCR|IXOFF|IXON|IXANY|IMAXBEL);
    hvrs_attr.c_iflag |=  (INPCK|IGNPAR|IGNBRK);
    hvrs_attr.c_oflag &= ~(OPOST|ONLCR);
    hvrs_attr.c_lflag &= ~(ICANON|ECHO|ECHONL|ISIG|IEXTEN|TOSTOP);
    hvrs_attr.c_lflag |=  (NOFLSH);
	// 1 STOP bit, 8 bits, parity enabled, parity fixed=SPACE
    hvrs_attr.c_cflag &= ~(HUPCL|CSTOPB|CSIZE|PARODD);
    hvrs_attr.c_cflag |=  (CLOCAL|CREAD|PARENB|CS8|CMSPAR);
    ret = cfsetspeed(&hvrs_attr, baud);
    if (ret != 0) {
	printf("HVRS-Open-ERR: bad baud rate %d\n", baud);
	return -1;
    }
    hvrs_attr.c_cc[VMIN] = 0;
    hvrs_attr.c_cc[VTIME] = (timeout == 0) ? 2 : timeout/100;
    ret = tcsetattr(fd, TCSANOW, &hvrs_attr);
    if (ret != 0) {
	printf("HVRS-Open-ERR: Failed to configure %s : %s\n", dev, strerror(errno));
	return -1;
    }
    return fd;
}

void hvrs_Close(int fd) {
    if (fd >= 0) close(fd);
}

void hvrs_uDelay(int us) {
    int j, jj, tim;
    tim = (float)us/usperc;
    for (j=0; j<tim; j++) for (jj=0; jj<100; jj++);
}

void hvrs_CalibrateDelay(void) {
    clock_t start, end;
    float res, mean;
    int i;
    
    usperc = 1.0;
    for (i=0; i<10; i++) {
	start = clock();
	hvrs_uDelay(100000);
	end = clock();
	res = ((double)(end - start)) / CLOCKS_PER_SEC;
	mean += res;
    }
    mean /= 10.;
    usperc = mean*10.;
}

int hvrs_Command(int fd, unsigned char addr, unsigned char cmd, unsigned int data) 
{
// Returns the reply, negative on error or no reply (timeout)
    int i, irc;
    unsigned char csm;
    struct termios hvrs_attr;
    struct timespec wait;
    unsigned char sbuf[7], rbuf[4];

    if (fd <= 0) return -EINVAL;
//	Make the sbuf
    sbuf[0] = addr;
    sbuf[1] = cmd;
    for (i=0; i<4; i++) {
	sbuf[2+i] = data & 0xFF;
	data >>= 8;
    }
    csm = 0;
    for (i=0; i<6; i++) csm += sbuf[i];
    sbuf[6] = 0xFF - csm;
    
	// Addr byte with 9 bit set
    irc = tcgetattr(fd, &hvrs_attr);
    if (irc) { 
	printf("hvrs-Send-ERR-Cannot access port attributes (irc=%d): %s\n", 
	    irc, strerror(errno)); 
	return -errno;
    }	// 1 STOP bit, 8 bits, parity enabled, parity fixed=MARK
    hvrs_attr.c_cflag |= PARODD;
    irc = tcsetattr(fd, TCSADRAIN, &hvrs_attr);
    if (irc) { 
	printf("hvrs-Send-ERR-Cannot set port attributes (irc=%d): %s\n", 
	    irc, strerror(errno)); 
	return -errno;
    }
    irc = write(fd, &sbuf[0], 1);
    if (irc != 1) {
	printf("hvrs-Send-ERR-Cannot write to RS232: %s\n", strerror(errno));
	return -EIO;
    }
    hvrs_uDelay(100);	// 0.1 ms
	// Clear 9 bit
    hvrs_attr.c_cflag &= ~(PARODD);
    irc = tcsetattr(fd, TCSANOW, &hvrs_attr);
    if (irc) return -ENOTTY;
	// Command
    irc = write(fd, &sbuf[1], 6);
    if (irc != 6) return -EIO;
	// No let's read
    for(i=0; i<4; i += irc) {
	irc = read(fd, &rbuf[i], 4-i);
	if (irc <= 0) break;
    }
    irc = i;
    if (irc != 4) {
	printf("hvrs-Get-ERR-No or bad reply from %2.2X (len=%d", addr, irc);
	if (irc > 0) printf(", rbuf=");
	for (i=0; i<irc; i++) printf(" %2.2X", rbuf[i]);
	printf(")\n");
	return -ETIMEDOUT;
    }
    csm = 0;
    for (i=0; i<4; i++) csm += rbuf[i];
    if (csm != 0xFF) {
	printf("hvrs-Get-ERR-No or bad reply from %2.2X (rbuf= ", addr);
	for (i=0; i<4; i++) printf(" %2.2X", rbuf[i]);
	printf(")\n");
	return -EIO;
    }
    
    return rbuf[1] + (rbuf[2] << 8);
}
