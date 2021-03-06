/*******************************************************************
** File        : Initialisation.h                                 **
********************************************************************
**                                                                **
** Version     :                                                  **
**                                                                **
** Written by   :                                                  **
**                                                                **
** Date        : XX-XX-XXXX                                       **
**                                                                **
** Project     : -                                                **
**                                                                **
********************************************************************
** Changes     :                                                  **
********************************************************************
** Description : Initialization settings for the uController      **
*******************************************************************/
#ifndef __INITIALISATION__
#define __INITIALISATION__
/*******************************************************************
** Include files                                                  **
*******************************************************************/
#include "Globals.h"

/*******************************************************************
** Global definitions                                             **
*******************************************************************/
#define UART_SELXTAL          0x80
#define UART_RCSEL_2          0x20
#define UART_RCSEL_1          0x10
#define UART_RCSEL_0          0x08
#define UART_PM               0x04
#define UART_PE               0x02
#define UART_WL               0x01
#define UART_ECHO             0x80
#define UART_ENRX             0x40
#define UART_ENTX             0x20
#define UART_XRX              0x10
#define UART_XTX              0x08
#define UART_BR_2             0x04
#define UART_BR_1             0x02
#define UART_BR_0             0x01

/*******************************************************************
** InitMicro : Initializes the MicroController peripherals        **
********************************************************************
** In        : -                                                  **
** Out       : -                                                  **  
*******************************************************************/
void InitMicro(void);

/*******************************************************************
** Common Peripherals to all the XE8000 family	                  **
********************************************************************/

/*******************************************************************
**  PA7  *  PA6  *  PA5  *  PA4  *  PA3  *  PA2  *  PA1  *  PA0   **
**   -   *   -   *   -   *   -   *   -   *   -   *   -   *   -    **
*******************************************************************/
/*******************************************************************
** InitPortA : Initializes the Port A                             **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitPortA(void);

/*******************************************************************
**  PB7  *  PB6  *  PB5  *  PB4  *  PB3  *  PB2  *  PB1  *  PB0   **
**   -   *   -   *   -   *   -   *   -   *   -   *   -   *   -    **
*******************************************************************/
/*******************************************************************
** InitPortB : Initializes the Port B                             **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitPortB(void);

/*******************************************************************
** InitCounters : Initializes the Counters                        **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitCounters(void);                      ;

/*******************************************************************
** InitRCOscillator : Initializes the RC oscillator (DFLL)        **
********************************************************************
** In  : Frequency (in Hz)                                        **
** Out : -                                                        **
*******************************************************************/
void InitRCOscillator(unsigned long Frequency);

/*******************************************************************
** Parameters :                                                   **
** Baudrate : 115200, 57600, 38400, 19200, 9600, 4800, 2400,      **
**             1200,600, 300                                bauds **
** Data Len : 1 = 8, 0 = 7                                  bits  **
** Parity   : 0 = ODD, 1 = EVEN                                   **
** ParityEn : Enables or disables the parity                      **
** rcFactor : 1, 2, 4, 8, 16 times the default rc frequency       **
*******************************************************************/

/*******************************************************************
** InitUART : Initializes the UART                                **
********************************************************************
** In  : baudrate, dataLen, parity, parityEn, rcFactor            **
** Out : -                                                        **
*******************************************************************/
void InitUART(_U32 baudrate, _U8 dataLen, _U8 parity, _U8 parityEn,
              _U8 rcFactor);

/*******************************************************************
** Specific Peripherals 								                  **
********************************************************************/

#ifdef _XE88LC02_ 
/*******************************************************************
** PD1_7 * PD1_6 * PD1_5 * PD1_4 * PD1_3 * PD1_2 * PD1_1 * PD1_0  **
**   -   *   -   *   -   *   -   *   -   *   -   *   -   *   -    **
*******************************************************************/
/*******************************************************************
** InitPortD1 : Initializes the Port D1                           **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitPortD1(void);

/*******************************************************************
** PD2_7 * PD2_6 * PD2_5 * PD2_4 * PD2_3 * PD2_2 * PD2_1 * PD2_0  **
**   -   *   -   *   -   *   -   *   -   *   -   *   -   *   -    **
*******************************************************************/
/*******************************************************************
** InitPortD2 : Initializes the Port D2                           **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitPortD2(void);

/*******************************************************************
** InitLCD : Initializes the LCD                                  **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitLCD(void);
#endif //Target LC02

#if defined(_XE88LC01_) || defined(_XE88LC03_) || defined(_XE88LC05_)
/*******************************************************************
**  PC7  *  PC6  *  PC5  *  PC4  * PC3   * PC2   * PC1   * PC0    **
**   -   *   -   *   -   *   -   *   -   *   -   *   -   *   -    **
*******************************************************************/
/*******************************************************************
** InitPortC : Initializes the Port C                             **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitPortC(void);
#endif //Target LC01, LC03, LC05

#ifdef _XE88LC05_
/*******************************************************************
** ------------------------ Parameters ---------------------------**
** NsOrder  : 0x00 - 0x02             [0 (PWM), 1, 2            ] **
** CodeImax : 0x00 - 0x07             [4, 5, 6, 7, 8, 9, 10, 11 ] **
** Enable   : 0x00 - 0x03             [DAC, AMP                 ] **
** Fin      : 0x00 - 0x01             [RC clock, RC clock div 2 ] **
** BW       : 0x00 - 0x01             [CL0, CL1                 ] **
** Inv      : 0x00 - 0x01             [HIGH, LOW                ] **
*******************************************************************/
/*******************************************************************
** InitDAS : Initializes the D/A Signal                           **
********************************************************************
** In  : NsOrder, CodeImax, Enable, Fin, BW, Inv                  **
** Out : -                                                        **
*******************************************************************/
void InitDAS(_U8 NsOrder, _U8 CodeImax, _U8 Enable, _U8 Fin,
	         _U8 BW, _U8 Inv);

/*******************************************************************
** ------------------------ Parameters ---------------------------**
** 0x00                                 [DAC OFF, AMP OFF       ] **
** 0x01                                 [DAC ON , AMP OFF       ] **
** 0x02                                 [DAC OFF, AMP ON        ] **
** 0x03                                 [DAC ON , AMP ON        ] **
*******************************************************************/
/*******************************************************************
** InitDAB : Initializes the D/A Bias                             **
********************************************************************
** In  : Enable                                                   **
** Out : -                                                        **
*******************************************************************/
void InitDAB(_U8 Enable);

#endif //Target LC05

#ifdef _XE88LC06A_ 
/*******************************************************************
**  PD7  *  PD6  *  PD5  *  PD4  * PD3   * PD2   * PD1   * PD0    **
**   -   *   -   *   -   *   -   *   -   *   -   *   -   *   -    **
*******************************************************************/
/*******************************************************************
** InitPortD : Initializes the Port D                             **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitPortD(void);
#endif //Target LC06

#endif /* __INITIALISATION__ */

