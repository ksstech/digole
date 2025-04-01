// digole.h

#pragma once

#include "hal_i2c_common.h"
#include "report.h"

#ifdef __cplusplus
extern "C" {
#endif

// ########################################### Macros ##############################################

// ######################################## Enumerations ###########################################

typedef enum {
	digoleCFG,						// Configure
	digoleCLR,						// Clear Screen
	digoleBL,						// BackLighht <0/1>
	digoleCS,						// Cursor <0/1>
	digoleSF,						// Set Font <number>
	digoleTP,						// Text Position <row,col>
	digoleTT,						// Text Type <message>
	digoleTPTT,						// Text Position & Text <row, col, message>
	digolePF,						// Print Formatted <fmt, ...>
	digoleTPPF,						// Text Position & Print Formatted <row, col, fmt, ...>
	digoleNUM,
} digole_e;

// ######################################### Structures ############################################

typedef struct __attribute__((packed)) digole_t {
	i2c_di_t *	psI2C;									// size = 4
} digole_t;
//DUMB_STATIC_ASSERT(sizeof(digole_t) == 13);

// ###################################### Public variables #########################################

// ###################################### Public functions #########################################

int digoleIdentify(i2c_di_t * psI2C);

int digoleConfig(i2c_di_t * psI2C);

int digoleFunction(digole_e Func, ...);

/**
 * @brief
 * @return
 */
#define digoleSetup()							digoleFunction(digoleCFG)

/**
 * @brief
 * @return
 */
#define digoleClear()							digoleFunction(digoleCLR)

/**
 * @brief
 * @param[in]
 * @return
 */
#define digoleCursor(OffOn)						digoleFunction(digoleCS, OffOn)

/**
 * @brief
 * @param[in]
 * @return
 */
#define digoleSetFont(Font)						digoleFunction(digoleSF, Font)

/**
 * @brief
 * @param[in]
 * @param[in]
 * @return
 */
#define digoleLocate(Row, Col)					digoleFunction(digoleTP, Row, Col)	

/**
 * @brief
 * @param[in]
 * @return
 */
#define digoleText(pccStr)						digoleFunction(digoleTT, pccStr)

/**
 * @brief
 * @param[in]
 * @param[in]
 * @param[in]
 * @return
 */
#define digoleTextAt(Row, Col, pccStr)			digoleFunction(digoleTPTT, Row, Col, pccStr)	

/**
 * @brief
 * @param[in]
 * @return
 */
#define digolePrint(Fmt, ...)					digoleFunction(digolePF, Fmt, ##__VA_ARGS__)

/**
 * @brief
 * @param[in]
 * @param[in]
 * @param[in]
 * @return
 */
#define digolePrintAt(Row, Col, Fmt, ...)		digoleFunction(digoleTPPF, Row, Col, Fmt, ##__VA_ARGS__)	

/**
 * @brief
 * @param[in]
 * @return
 */
int digoleReport(struct report_t * psR);

#ifdef __cplusplus
}
#endif
