#ifndef CMDDEF_H
#define CMDDEF_H

#define CMD_ON	 	1
#define CMD_OFF	 	2
#define CMD_SET	 	3
#define CMD_ADC	 	4
#define CMD_STAT 	5
#define CMD_EEP   	0x20
#define CMD_CLEAR	0x21	// set unprotected EEPROM to the default (all off) state and reset the device
#define CMD_RESET	0x22	// do a hard reset by WatchDog

#ifdef HWDEBUG
#define CMD_WPORT 0x80
#define CMD_RPORT 0x81
#define CMD_DAC   0x82
#endif

//	ON/OFF switches mask (byte 0) + DAC chanels switches in bytes 2, 3
#define ON_LV		1
#define ON_HV		2

//	Set address map
#define SET_DAC	 	0
#define SET_ILIM 	0x20
#define SET_TDACLIM 0x30
#define SET_TCPULIM 0x31
#define SET_T1WLIM  0x32

//  ADC & DAC readout address map
#define ADC_DAC  0
#define ADC_TDAC 18
#define ADC_HVMV 19
#define ADC_PAMV 20
#define ADC_HVFB 21
#define ADC_HVMI 22
#define ADC_TADC 23
#define ADC_SET  0x40
#define ADC_READ 0x50
#define ADC_T1W	 0x60

//	Error bits mask (byte 1) + bits from ON/OFF
#define ST_OVC		1		// overcurrent
#define ST_OVTDAC	2		// DAC overtemperature
#define ST_OVTCPU	4		// CPU overtemperature
#define ST_OVT1W	8		// 1W overtemperature
#define ST_ECSUM	0x10	// unprotected EEPROM checksum error
#define ST_EPSUM	0x20	// protected EEPROM checksum error
#define ST_BADCMD	0x40	// unknown command

// Commands for CMD_EEP
#define E_READ		0		// read 2 bytes
#define E_WRITE		1		// write 2 bytes
#define E_RDSR		2		// read status
#define E_WRSR		3		// write status

#define CSUM_VAL	0x55AA

// EEPROM memory map
#define EE_FLAGS	0		// device global flags
#define	EE_CHAN		2		// channels ON/OFF
#define EE_OVC		4		// overcurrent limit
#define EE_OVTDAC	6		// DAC overtemperature limit
#define EE_OVTCPU	8		// CPU overtemperature limit
#define EE_OVT1W	0x0A	// 1W-thermometer overtemperature limit
#define EE_DAC		0x20	// 16 DAC settings
#define EE_CSUM		0xBE	// 16-bit sum of low 3/4 of EEPROM should be 0x55AA
#define EE_PBEGIN	0xC0	// begin of the protected part of EEPROM - the high quater
#define EE_HVCA		0xC0	// HV translation coefficient A
#define EE_HVCB		0xC2	// HV translation coefficient B
#define EE_ADDR		0xF0	// device address
#define EE_SERIAL	0xF2	// serial number
#define EE_CFG		0xF4	// device configuration
#define EE_PSUM		0xFE	// protected block control sum - 16-bit sum of the high quater of EEPROM should be 0x55AA
//	EE_FLAGS
#define FLG_AUTO	0x8000	// enable auto ON on power on

#endif
