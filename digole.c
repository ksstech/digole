// digole.c - Copyright (c) 2017-25 Andre M. Maree / KSS Technologies (Pty) Ltd.

#include "hal_platform.h"

#if (HAL_DIGOLE > 0)

#include "digole.h"
#include "errors_events.h"

#include <string.h>

// ########################################## MACROS ###############################################

#define	debugFLAG					0xF000
#define	debugTIMING					(debugFLAG_GLOBAL & debugFLAG & 0x1000)
#define	debugTRACK					(debugFLAG_GLOBAL & debugFLAG & 0x2000)
#define	debugPARAM					(debugFLAG_GLOBAL & debugFLAG & 0x4000)
#define	debugRESULT					(debugFLAG_GLOBAL & debugFLAG & 0x8000)

// ######################################## Enumerations ###########################################

// ######################################### Structures ############################################

// ######################################### Local variables #######################################

digole_t sDigole = { 0 };

// ####################################### Local functions #########################################

/**
 * @brief	Write data from buffer to the Digole display
 * @param[in]	pccStr pointer to the data buffer
 * @param[in]	Len size of data in buffer to be sent
 * @return	result from the I2C transaction
 */
static int digoleWriteLen(const char * pccStr, int Len) {
	return halI2C_Queue(sDigole.psI2C, i2cW, (u8_t *)pccStr, Len, (u8_t *) NULL, 0, (i2cq_p1_t) NULL, (i2cq_p2_t) NULL);
}

/**
 * @brief	Write a text string to the Digole display INCLUDING the terminating NUL
 * @param[in]	pccStr pointer to the text string
 * @return	result from the I2C transaction
 */
static int digoleWriteText(const char * pccStr) {
	int iRV = digoleWriteLen("TT", 2);
	if (iRV > erFAILURE)
		iRV = digoleWriteLen(pccStr, strlen(pccStr) + 1);
	return iRV;
}

/**
 * @brief	Encode a value, format as required by Digole display, into a buffer 
 * @param[in]	pcBuf to be used for storage of encoded value
 * @param[in]	Val value to be encoded
 * @return	number of characters stored
 */
static int digoleStoreValue(char * pcBuf, int Val) {
	int iRV = 0;
	if (Val > 255) {
		*pcBuf++ = 255;
		Val -= 255;
		++iRV;
	}
	*pcBuf++ = Val;
	return ++iRV;
}

/**
 * @brief	Encode number of "int" values and Write them following the a text string to the Digole display
 * @param[in]	pccStr pointer to the text string
 * @param[in]	Count number of "int" values to be encoded
 * @param[in]	pvaList pointer to va_list structure with int variables
 * @return	result from the I2C transaction
 */
static int digoleWriteINT(const char * pccStr, int Count, va_list * pvaList) {
	char caBuf[16];
	char * pTmp;
	pTmp = stpcpy(caBuf, pccStr);
	while (Count--) {
		pTmp += digoleStoreValue(pTmp, va_arg(*pvaList, int));	// store parameter
		IF_myASSERT(debugTRACK, (pTmp - caBuf) < sizeof(caBuf));
	}
	return digoleWriteLen(caBuf, pTmp - caBuf);
}

// ###################################### Global functions #########################################

/*
"\e\034\e\030\003\003\e\001Hello World");		ESC code does not work....
"FS\000" "FS\121"								only from v2.8
"DSS\001" "DC\001"								Enable startup and config display
#define	BASIC	"CLCS\000"												// clear, no cursor
#define FONT0	"SF\000TT12345678901234567890\000"						// font 0 W=20
#define FONT6	"SF\006TT1234567890123456789012345678901234567890\000"	// font 6 W=40
#define FONT10	"SF\012TT12345678901234567890123456\000"				// font 10 W=26
#define FONT18	"SF\022TT12345678901234567\000"							// font 18 W=17
#define FONT51	"SF\063TT12345678901\000"								// font 51 W=11
#define FONT120	"SF\170TT1234567890\000"								// font 120 W=10
#define FONT123	"SF\173TT12345\000"										// font 123 W=5
*/

int digoleFunction(digole_e Func, ...) {
	IF_myASSERT(debugPARAM, Func < digoleNUM);
	int iRV = erSUCCESS;
	va_list vaList;
	va_start(vaList, Func);
	if (INRANGE(digoleTP, Func, digoleTPPF)) {			// cursor locate and text display/print (1/2/3/4+ params)
		// step 1: cursor location if requested...
		if (Func == digoleTP || Func == digoleTPTT || Func == digoleTPPF)	// TP or any combination?
			iRV = digoleWriteINT("TP", 2, &vaList);							// handle the TP component
		// Step 2: text message or formatted print ?
		if (iRV > erFAILURE) {
			char * pcTmp = va_arg(vaList, char *);		// TT/Text message or PF/Format string
			if (INRANGE(digolePF, Func, digoleTPPF)) {	// PF & TPPF 
				char caBuf[32];
				vsnprintfx(caBuf, sizeof(caBuf), pcTmp, vaList);
				pcTmp = caBuf;
			}
			iRV = digoleWriteText(pcTmp);
		}
	} else if (INRANGE(digoleBL, Func, digoleSF)) {		// backlight/cursor/font selection (1 param)
		iRV = digoleWriteINT((Func == digoleBL) ? "BL" : (Func == digoleCS) ? "CS" : "SF", 1, &vaList);

	} else if (Func == digoleCLR) {						// clear screen (0 param)
		digoleWriteLen("CL", 2);

	} else if (Func == digoleCFG) {						// fixed multiple commands in 1
		#define digoleCONFIG "CLCS\000SF\022BL\001"
		iRV = digoleWriteLen(digoleCONFIG, sizeof(digoleCONFIG)-1);
	}
	va_end(vaList);
	return iRV;
}

int digoleIdentify(i2c_di_t * psI2C) {
	sDigole.psI2C = psI2C;
	psI2C->Type = i2cDEV_DIGOLE;
	psI2C->Speed = i2cSPEED_400;
	psI2C->TObus = 25;
	// Must add code to properly identify the device
	psI2C->IDok = 1;
	return erSUCCESS;
}

int digoleConfig(i2c_di_t * psI2C) {
	int iRV = erINV_STATE;
	if (psI2C->IDok == 0)
		goto exit;
	psI2C->CFGok = 0;
	halEventUpdateDevice(devMASK_DIGOLE, 0);
	iRV = digoleSetup();
	if (iRV < erSUCCESS)
		goto exit;
	psI2C->CFGok = 1;
	halEventUpdateDevice(devMASK_DIGOLE, 1);
exit:
	return iRV;
}

int digoleReport(report_t * psR) {
	return halI2C_DeviceReport(psR, (void *) sDigole.psI2C);
}

#endif
