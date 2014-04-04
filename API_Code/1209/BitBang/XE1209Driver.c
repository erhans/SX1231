/*******************************************************************
** File        : XE1209driver.c                                   **
********************************************************************
**                                                                **
** Version     : V 1.0                                            **
**                                                                **
** Written by   : Miguel Luis & Gr�goire Guye                     **
**                                                                **
** Date        : 06-02-2003                                       **
**                                                                **
** Project     : API-1209                                         **
**                                                                **
********************************************************************
** Changes     : V 2.0 / MiL - 09-12-2003                         **
**                                                                **
** Changes     : V 2.1 / MiL - 24-04-2004                         **
**               - No changes                                     **
**                                                                **
** Changes     : V 2.3 / CRo - 06-06-2006                         **
**               - No Changes                                     **
**                                                                **
** Changes     : V 2.4 / CRo - 09-01-2007                         **
**               - No change                                      **
**                                                                **
**                                                                **
********************************************************************
** Description : XE1209 transceiver drivers implementation for the**
**               XE8000 family products (BitBang)                 **
*******************************************************************/

/*******************************************************************
** Include files                                                  **
*******************************************************************/
#include "XE1209Driver.h"

/*******************************************************************
** Global variables                                               **
*******************************************************************/
/*******************************************************************
** Global variables                                               **
*******************************************************************/
static   _U8 RFState = RF_STOP;      // RF state machine
static   _U8 *pRFFrame;              // Pointer to the RF frame
static   _U8 RFFramePos;             // RF frame current position
static   _U8 RFFrameSize;            // RF frame size
static  _U16 ByteCounter = 0;        // RF frame byte counter
static   _U8 PreMode = RF_SLEEP;     // Previous chip operating mode
volatile _U8 EnableSyncByte = true;  // Enables/disables the synchronization byte reception/transmission
static   _U8 SyncByte;               // RF synchronization byte counter
static   _U8 PatternSize = 4;        // Size of pattern detection
static   _U8 StartByte[4] = {0x69, 0x81, 0x7E, 0x96}; // Pattern detection values
static  _U32 RFFrameTimeOut = RF_FRAME_TIMEOUT(1820); // Reception counter value (full frame timeout generation)
static  _U32 RFBaudrate = TX_BAUDRATE_GEN_1820;       // Transmission counter value (baudrate generation)

_U16 RegistersCfg[] = { // XE1209 configuration registers values
    DEF_REG_A | RF_A_OSC_INT | RF_A_TEST_NORMAL | RF_A_SENS_200 | RF_A_PWR_1_8 | RF_A_TR_0 | RF_A_FC_36_86
};

/*******************************************************************
** Configuration functions                                        **
*******************************************************************/

/*******************************************************************
** InitRFChip : Initializes the RF Chip registers using           **
**            pre initialized global variable                     **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitRFChip(void){
    // Initializes XE1209
    SrtInit();

    WriteRegister(REG_A, RegistersCfg[REG_A]);

    SetRFMode(RF_SLEEP);
} // void InitRFChip(void)

/*******************************************************************
** SetRFMode : Sets the XE1209 operating mode (sleep,             **
**            carrier_detector, receiver, Transmitter)            **
********************************************************************
** In  : mode                                                     **
** Out : -                                                        **
*******************************************************************/
void SetRFMode(_U8 mode){
    if(mode != PreMode){
        if((mode == RF_TRANSMITTER) && (PreMode == RF_SLEEP)){
            PreMode = RF_TRANSMITTER;
			WriteRegister(REG_A, (ReadRegister(REG_A) | RF_A_TR_1));
			clear_bit(PORTO, RE);
			set_bit(PORTDIR, DATA); // Change DATA pin direction
        }
        else if((mode == RF_TRANSMITTER) && (PreMode == RF_RECEIVER)){
            PreMode = RF_TRANSMITTER;
			WriteRegister(REG_A, (ReadRegister(REG_A) | RF_A_TR_1));
			clear_bit(PORTO, RE);
			set_bit(PORTDIR, DATA); // Change DATA pin direction
        }
        else if((mode == RF_RECEIVER) && (PreMode == RF_SLEEP)){
            PreMode = RF_RECEIVER;
			WriteRegister(REG_A, (ReadRegister(REG_A) | RF_A_TR_1));
			set_bit(PORTO, RE);
			clear_bit(PORTDIR, DATA); // Change DATA pin direction
        }
        else if((mode == RF_RECEIVER) && (PreMode == RF_TRANSMITTER)){
            PreMode = RF_RECEIVER;
			WriteRegister(REG_A, (ReadRegister(REG_A) | RF_A_TR_1));
			set_bit(PORTO, RE);
			clear_bit(PORTDIR, DATA); // Change DATA pin direction
        }
        else if(mode == RF_SLEEP){
            PreMode = RF_SLEEP;
			WriteRegister(REG_A, (ReadRegister(REG_A) & (~RF_A_TR_1)));
            clear_bit(PORTO, RE);
			set_bit(PORTDIR, DATA); // Change DATA pin direction
        }
        else{
            PreMode = RF_SLEEP;
			WriteRegister(REG_A, (ReadRegister(REG_A) & (~RF_A_TR_1)));
            clear_bit(PORTO, RE);
			set_bit(PORTDIR, DATA); // Change DATA pin direction
        }
    }
}

/*******************************************************************
** WriteRegister : Writes the register value on the               **
**                 XE1209                                         **
********************************************************************
** In  : address, value                                           **
** Out : -                                                        **
*******************************************************************/
void WriteRegister(_U8 address, _U16 value){
    _U8 bitCounter;
	
	set_bit(PORTO, DE);
	SrtInit();
	
	for(bitCounter = 0x01; bitCounter != 0x00; bitCounter <<= 1){	       
        if (value & bitCounter){
			SrtSetSO(1);
		}
		else{
            SrtSetSO(0);
        }
        
		if(bitCounter == 0x80){
			SrtSetSCK(1);		
			clear_bit(PORTO, (DE+SC));
		}
		else{
			SrtSetSCK(1);
			SrtSetSCK(0);		
		}
    }
	clear_bit(PORTO, (DE+SD+SC));
}

/*******************************************************************
** ReadRegister : Not possible with XE1209                        **
********************************************************************
** ReadRegister : Reads the pre initialized global variable that  **
**               is the image of RF chip registers value          **
********************************************************************
** In  : address                                                  **
** Out : value                                                    **
*******************************************************************/
_U16 ReadRegister(_U8 address){
    return RegistersCfg[address];
} // _U16 ReadRegister(_U8 address)

/*******************************************************************
** Communication functions                                        **
*******************************************************************/

/*******************************************************************
** SendRfFrame : Sends a RF frame                                 **
********************************************************************
** In  : *buffer, size                                            **
** Out : *pReturnCode                                             **
*******************************************************************/
void SendRfFrame(_U8 *buffer, _U8 size, _U8 *pReturnCode){
    if(size > RF_BUFFER_SIZE_MAX){
        RFState |= RF_STOP;
        *pReturnCode = ERROR;
        return;
    }
    SetRFMode(RF_TRANSMITTER);

    RFState |= RF_BUSY;
    RFState &= ~RF_STOP;
    RFFrameSize = size;
    pRFFrame = buffer;

    TxEventsOn();                                 // Enable Tx events
    clear_bit(PORTO, DATA);

    for(ByteCounter = 0; ByteCounter < 4; ByteCounter++){
        SendByte(0xAA);
    }

    for(ByteCounter = 0; ByteCounter < PatternSize; ByteCounter++){
        SendByte(StartByte[ByteCounter]);
    }

    SendByte(RFFrameSize);

    SyncByte = 0;

    for(ByteCounter = 0, RFFramePos = 0; ByteCounter < RFFrameSize; ByteCounter++){
        if(SyncByte == SYNC_BYTE_FREQ){
            if(EnableSyncByte){
                SyncByte = 0;
                SendByte(0x55);
                ByteCounter--;
            }
        }
        else{
            if(EnableSyncByte){
                SyncByte++;
            }
            SendByte(pRFFrame[RFFramePos++]);
        }
    }

    clear_bit(PORTO, DATA);
    TxEventsOff();                                // Disable Tx events

    RFState |= RF_STOP;
    RFState &= ~RF_TX_DONE;
    *pReturnCode = OK;
} // void SendRfFrame(_U8 *buffer, _U8 size, _U8 *pReturnCode)

/*******************************************************************
** ReceiveRfFrame : Receives a RF frame                           **
********************************************************************
** In  : -                                                        **
** Out : *buffer, size, *pReturnCode                              **
*******************************************************************/
void ReceiveRfFrame(_U8 *buffer, _U8 *size, _U8 *pReturnCode){
    _U8  dummy = 0;
	_U8  i;
    _U16 preamble = 0;
    _U8  bitCounter = 0;
	_U8  start[4] = {0, 0, 0, 0};

    pRFFrame = buffer;
    RFFramePos = 0;
    RFFrameSize = 2;
    SyncByte = 0;

    SetRFMode(RF_RECEIVER);
    RxEventsOn();                               // Enable events (timeout)

    do {
        preamble <<= 1;                         // Shifts left the received data
        do{
            asm("halt");
        }while ((RegEvn & 0x90) == 0x00);       // Waits the event from the PortA or from the counter A
        if ((RegEvn & 0x80) == 0x80){           // Tests if counter A generates an event
            RxEventsOff();                      // Disable events
            *pReturnCode = RX_TIMEOUT;          // Returns the status TimeOut
            return;
        }
        RegEvn |= 0x10;                         // Clears the event from the PortA pin 1 on the event register
        asm("clrb %stat, #0");                  // Clears the event on the CoolRISC status register
        if(PORTI & DATA){                       // Test the DATA pin
            set_bit(preamble, 1);
        }
        else{
            clear_bit(preamble, 1);
        }
        bitCounter++;                           // Increments the bit counter
    } while (preamble != (_U16)PREAMBLE);       // Waits to detect preamble

    do{
        start[0] >>= 1;                         // Shifts right the received data
        do{
            asm("halt");
        }while ((RegEvn & 0x90) == 0x00);       // Waits the event from the PortA or from the counter A

        if ((RegEvn & 0x80) == 0x80){           // Tests if counter A generates an event
            RxEventsOff();                      // Disable events
            *pReturnCode = RX_TIMEOUT;          // Returns the status TimeOut
            return;
        }
        RegEvn = 0x10;                          // Clears the event from the PortA pin 1 on the event register
        asm("clrb %stat, #0");                  // Clears the event on the CoolRISC status register
        if(PORTI & DATA){                       // Test the DATA pin
            set_bit(start[0], 0x80);
        }
        else{
            clear_bit(start[0], 0x80);
        }
    }while (start[0] != StartByte[0]);

    // The following lines performs the same behavior as RxEventsOff then RxEventsOn inlined
    RegCntOn &= 0xFC;                           // Disables counters A&B
    RegEvnEn &= 0x6F;                           // Disables events from PortA bit 1 and Cnt A
    asm("clrb %stat, #0");                      // Clears the event on the CoolRISC status register
    RegCntA       = (_U8)(RFFrameTimeOut);      // LSB of RFFrameTimeOut
    RegCntB       = (_U8)(RFFrameTimeOut >> 8); // MSB of RFFrameTimeOut
    RegEvnEn      |= 0x90;                      // Enables events from PA1 and CntA
    RegEvn        |= 0x90;                      // Clears the event from the CntA and PA1 on the event register
    asm("and %stat, #0xDE");                    // Clears the event on the CoolRISC status register, and disable all interrupts
    RegCntOn      |= 0x03;                      // Enables counter A&B
    // End of RxEventsOff then RxEventsOn inlined

    for(i = 1; i < PatternSize; i++){
        start[i] = ReceiveByte();

        if(start[i] != StartByte[i]){
            RxEventsOff();
            *pReturnCode = ERROR;
            return;
        }
    }

    RFFrameSize = ReceiveByte();
    RFFramePos++;

    while(RFFramePos < RFFrameSize + 1){
        if(SyncByte == SYNC_BYTE_FREQ){
            if(EnableSyncByte){
                SyncByte = 0;
                dummy = ReceiveByte();
                RFFramePos--;
            }
        }
        else{
            pRFFrame[RFFramePos-1] = ReceiveByte();
            if(EnableSyncByte){
                SyncByte++;
            }
        }
        if ((RegEvn & 0x80) == 0x80){       // Tests if counter A generates an event
            RxEventsOff();
            *size = RFFrameSize;
            *pReturnCode = RX_TIMEOUT;
            return ;                        // Returns the status TimeOut
        }
        RFFramePos++;
    }

    if(RFFrameSize >= RF_BUFFER_SIZE_MAX){
        *size = RFFrameSize;
        *pReturnCode = ERROR;
        return;
    }
    *size = RFFrameSize;
    *pReturnCode = OK;
    return;
} // void ReceiveRfFrame(_U8 *buffer, _U8 size, _U8 *pReturnCode)

/*******************************************************************
** SendByte : Send a data of 8 bits to the transceiver LSB first  **
********************************************************************
** In  : b                                                        **
** Out : -                                                        **
*******************************************************************/
void SendByte(_U8 b){
    _U8 bitCounter;

	for(bitCounter = 0x01; bitCounter != 0x00; bitCounter <<= 1){
        do{
            asm("halt");
        }while ((RegEvn & 0x80) == 0x00);         // Waits the event from the counter A
        if (b & bitCounter){
            set_bit(PORTO, DATA);
        }
        else{
            clear_bit(PORTO, DATA);
        }
        RegEvn = 0x80;                           // Clears the event from the counter A on the event register
        asm("clrb %stat, #0");                   // Clears the event on the CoolRISC status register
    } // for(bitCounter = 0x01; bitCounter != 0x00; bitCounter <<= 1)
} // void Sendbyte(_U8 b)

/*******************************************************************
** ReceiveByte : Receives a data of 8 bits from the transceiver   **
**              LSB first                                         **
********************************************************************
** In  : -                                                        **
** Out : b                                                        **
*******************************************************************/
_U8 ReceiveByte(void){
    _U8 b = 0;
	// Bit 0
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x01;
    }
	// Bit 1
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x02;
    }
	// Bit 2
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x04;
    }
	// Bit 3
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x08;
    }
	// Bit 4
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x10;
    }
	// Bit 5
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x20;
    }
	// Bit 6
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x40;
    }
	// Bit 7
    do{
        asm("halt");
    }while ((RegEvn & 0x90) == 0x00); // Waits the event from the PortA or the counter A
    RegEvn = 0x10;                    // Clears the event from the PortA on the event register
    asm("clrb %stat, #0");            // Clears the event on the CoolRISC status register
    if(PORTI & DATA){
        b |= 0x80;
    }
    return b;
} // _U8 ReceiveByte(void)

/*******************************************************************
** Transceiver specific functions                                 **
*******************************************************************/

/*******************************************************************
** Utility functions                                              **
*******************************************************************/

/*******************************************************************
** Wait : This routine uses the counter A&B to create a delay     **
**        using the RC ck source                                  **
********************************************************************
** In   : cntVal                                                  **
** Out  : -                                                       **
*******************************************************************/
void Wait(_U16 cntVal){
    RegCntOn &= 0xFC;                              // Disables counter A&B
    RegEvnEn &= 0x7F;                              // Disables events from the counter A&B
    RegEvn = 0x80;                                 // Clears the event from the CntA on the event register
    RegCntCtrlCk =  (RegCntCtrlCk & 0xFC) | 0x01;  // Selects RC frequency as clock source for counter A&B
    RegCntConfig1 |= 0x34;                         // A&B counters count up, counter A&B are in cascade mode
    RegCntA       = (_U8)(cntVal);                 // LSB of cntVal
    RegCntB       = (_U8)(cntVal >> 8);            // MSB of cntVal
    RegEvnEn      |= 0x80;                         // Enables events from CntA
    RegEvn        |= 0x80;                         // Clears the event from the CntA on the event register
    asm("clrb %stat, #0");                         // Clears the event on the CoolRISC status register
    RegCntOn      |= 0x03;                         // Enables counter A&B
    do{
        asm("halt");
    }while ((RegEvn & 0x80) == 0x00);              // Waits the event from counter A
    RegCntOn      &= 0xFE;                         // Disables counter A
    RegEvnEn      &= 0x7F;                         // Disables events from the counter A
    RegEvn        |= 0x80;                         // Clears the event from the CntA on the event register
    asm("clrb %stat, #0");                         // Clears the event on the CoolRISC status register
} // void Wait(_U16 cntVal)

/*******************************************************************
** TxEventsOn : Initializes the timers and the events related to  **
**             the TX routines PA1 CntA&B                         **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void TxEventsOn(void){
    RegCntCtrlCk =  (RegCntCtrlCk & 0xFC) | 0x01; // Selects RC frequency as clock source for counter A&B
    RegCntConfig1 |=  0x34;                       // A&B counters count up, counter A&B  are in cascade mode
    RegCntA       = (char)RFBaudrate;             // LSB of RFBaudrate
    RegCntB       = (char)(RFBaudrate >> 8);      // MSB of RFBaudrate
    RegEvnEn      |= 0x80;                        // Enables events for the counter A&B
    RegEvn        |= 0x80;                        // Clears the event from the CntA on the event register
    asm("and %stat, #0xDE");                      // Clears the event on the CoolRISC status register, and disable all interrupts
    RegCntOn      |= 0x03;                        // Enables counter A&B
} // void TxEventsOn(void)

/*******************************************************************
** TxEventsOff : Initializes the timers and the events related to **
**             the TX routines PA1 CntA&B                         **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void TxEventsOff(void){
    RegCntOn &= 0xFC;                              // Disables counter A&B
    RegEvnEn &= 0x7F;                              // Disables events from the counter A&B
    RegEvn = 0x80;                                 // Clears the event from the CntA on the event register
    asm("clrb %stat, #0");                         // Clears the event on the CoolRISC status register
    asm("setb %stat, #5");                         // Enable all interrupts
} // void TxEventsOff(void)

/*******************************************************************
** RxEventsOn : Initializes the timers and the events related to  **
**             the RX routines PA1 CntA&B                         **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void RxEventsOn(void){
    RegCntCtrlCk =  (RegCntCtrlCk & 0xFC) | 0x03;  // Selects 128 Hz frequency as clock source for counter A&B
    RegCntConfig1 |= 0x34;                         // A&B counters count up, counter A&B are in cascade mode
    RegCntA       = (_U8)RFFrameTimeOut;           // LSB of RFFrameTimeOut
    RegCntB       = (_U8)(RFFrameTimeOut >> 8);    // MSB of RFFrameTimeOut
    RegEvnEn      |= 0x90;                         // Enables events from PA1 and CntA
    RegEvn        |= 0x90;                         // Clears the event from the CntA and PA1 on the event register
    asm("and %stat, #0xDE");                       // Clears the event on the CoolRISC status register, and disable all interrupts
    RegCntOn      |= 0x03;                         // Enables counter A&B
} // void RxEventsOn(void)

/*******************************************************************
** RxEventsOff : Initializes the timers and the events related to **
**             the RX routines PA1 CntA&B                         **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void RxEventsOff(void){
    RegCntOn &= 0xFC;                           // Disables counters A&B
    RegEvnEn &= 0x6F;                           // Disables events from PortA bit 1 and Cnt A
    RegEvn |= 0x90;                             // Clears the event from the CntA and PA1 on the event register
    asm("clrb %stat, #0");                      // Clears the event on the CoolRISC status register
    asm("setb %stat, #5");                      // Enable all interrupts
} // void RxEventsOff(void)
