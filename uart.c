#include <LPC210X.H>
#include "uart.h"
#include "string.h"

/************ UART ************/
// U0LCR Line Control Register
#define mDIVISOR_LATCH_ACCES_BIT                   0x00000080
#define m8BIT_UART_WORD_LENGTH                     0x00000003

// UxIER Interrupt Enable Register
#define mRX_DATA_AVALIABLE_INTERRUPT_ENABLE        0x00000001
#define mTHRE_INTERRUPT_ENABLE                     0x00000002

// UxIIR Pending Interrupt Identification Register
#define mINTERRUPT_PENDING_IDETIFICATION_BITFIELD  0x0000000F
#define mTHRE_INTERRUPT_PENDING                    0x00000002
#define mRX_DATA_AVALIABLE_INTERRUPT_PENDING       0x00000004

/************ Interrupts **********/
// VIC (Vector Interrupt Controller) channels
#define VIC_UART0_CHANNEL_NR  6
#define VIC_UART1_CHANNEL_NR  7

// VICVectCntlx Vector Control Registers
#define mIRQ_SLOT_ENABLE                           0x00000020

////////////// Zmienne globalne ////////////
char cOdebranyZnak;

struct RecieverBuffer{ 
	char cData[RECEIVER_SIZE];
	unsigned char ucCharCtr;
	enum eRecieverStatus eStatus;
} RXBuffer;

struct TransmiterBuffer{
	char cData[TRANSMITER_SIZE];
	enum eTransmiterStatus eStatus;
	unsigned char fLastCharacter;
	unsigned char ucCharCtr;
};

struct TransmiterBuffer sTransmiterBuffer={"",FREE,0,0}; 

////////////////////////////////////////////RECEIVER////////////////////////////////

void Reciever_PutCharacterToBuffer(char cCharacter){
	
	if(RXBuffer.ucCharCtr==RECEIVER_SIZE){
		RXBuffer.eStatus=OVERFLOW;
	}
	else if(cCharacter=='\r'){
		RXBuffer.cData[RXBuffer.ucCharCtr]='\0';
		RXBuffer.eStatus=READY;
	}
	else{
		RXBuffer.cData[RXBuffer.ucCharCtr]=cCharacter;
		RXBuffer.ucCharCtr++;
	}
	
}

enum eRecieverStatus eReciever_GetStatus(void){
	
		return RXBuffer.eStatus;

}

void Reciever_GetStringCopy(char * ucDestination){
	
	CopyString(RXBuffer.cData,ucDestination);
	RXBuffer.ucCharCtr=0;
	RXBuffer.eStatus=EMPTY;
	
}

///////////////////////////////////TRANSMIT///////////////////////////////////////////

char Transmiter_GetCharacterFromBuffer(){
	
			char cReturnChar;
			
			if(sTransmiterBuffer.cData[sTransmiterBuffer.ucCharCtr]==NULL){
				if(sTransmiterBuffer.fLastCharacter!='\r'){
					sTransmiterBuffer.fLastCharacter='\r';
					sTransmiterBuffer.ucCharCtr++;
				}
				else{
					sTransmiterBuffer.fLastCharacter=NULL;
					sTransmiterBuffer.eStatus=FREE;
					//sTransmiterBuffer.ucCharCtr=0;
				}
				cReturnChar=sTransmiterBuffer.fLastCharacter;
			}
			else{
				cReturnChar=sTransmiterBuffer.cData[sTransmiterBuffer.ucCharCtr];
				sTransmiterBuffer.ucCharCtr++;
				if(sTransmiterBuffer.ucCharCtr>TRANSMITER_SIZE){
					//sTransmiterBuffer.ucCharCtr=0;
					sTransmiterBuffer.eStatus=FREE;
				}
			}
	return cReturnChar;
}

void Transmiter_SendString(char cString[]){
	
	unsigned char ucCharCounter;
	for(ucCharCounter = 0; ucCharCounter < TRANSMITER_SIZE; ucCharCounter++) {
		sTransmiterBuffer.cData[ucCharCounter] = NULL;
	}
	CopyString(cString,sTransmiterBuffer.cData);
	U0THR = sTransmiterBuffer.cData[0];
	sTransmiterBuffer.eStatus=BUSY;
	sTransmiterBuffer.ucCharCtr=1;
	
}

enum eTransmiterStatus Transmiter_GetStatus(){
	
	return sTransmiterBuffer.eStatus;
	
}

void DisplayValue(unsigned int uiCounterValue){
	char cDisplay[20]="licznik";
	AppendUIntToString(uiCounterValue, cDisplay);
	Transmiter_SendString(cDisplay);
}
/////////////////////////////////INTERRUPT,INIT//////////

__irq void UART0_Interrupt (void) {
   // jesli przerwanie z odbiornika (Rx)
   
   unsigned int uiCopyOfU0IIR=U0IIR; // odczyt U0IIR powoduje jego kasowanie wiec lepiej pracowac na kopii

   if      ((uiCopyOfU0IIR & mINTERRUPT_PENDING_IDETIFICATION_BITFIELD) == mRX_DATA_AVALIABLE_INTERRUPT_PENDING) // odebrano znak
   {
      Reciever_PutCharacterToBuffer(U0RBR);	
   } 
   
   if ((uiCopyOfU0IIR & mINTERRUPT_PENDING_IDETIFICATION_BITFIELD) == mTHRE_INTERRUPT_PENDING)              // wyslano znak - nadajnik pusty 
   {
     if(Transmiter_GetStatus()==BUSY)  
		 U0THR=Transmiter_GetCharacterFromBuffer();
   }

   VICVectAddr = 0; // Acknowledge Interrupt
}

////////////////////////////////////////////

/*
void UART_InitWithInt(unsigned int uiBaudRate){

   // UART0
   PINSEL0 = PINSEL0 | (1<<2|1<<0);                                     // ustawic pina na odbiornik uart0 i nadajnik 
   U0LCR  |= m8BIT_UART_WORD_LENGTH | mDIVISOR_LATCH_ACCES_BIT; // dlugosc slowa, DLAB = 1
   U0DLL   = ((15000000)/16)/uiBaudRate;                      // predkosc transmisji
   U0LCR  &= (~mDIVISOR_LATCH_ACCES_BIT);                       // DLAB = 0
   U0IER  |= mRX_DATA_AVALIABLE_INTERRUPT_ENABLE|mTHRE_INTERRUPT_ENABLE; // Wlaczam przerwanie Receive Data Available dla UART0 oraz nadajnik

   // INT
   VICVectAddr2  = (unsigned long) UART0_Interrupt;             // set interrupt service routine address
   VICVectCntl2  = mIRQ_SLOT_ENABLE | VIC_UART0_CHANNEL_NR;     // use it for UART 0 Interrupt
   VICIntEnable |= (0x1 << VIC_UART0_CHANNEL_NR);               // Enable UART 0 Interrupt Channel
	 RXBuffer.ucCharCtr=0;
	 sTransmiterBuffer.ucCharCtr=0;
}
*/

void UART_InitWithInt(unsigned int uiBaudRate){
	
	unsigned long ulDivisor, ulWantedClock;
	ulWantedClock=uiBaudRate*16;
	ulDivisor=15000000/ulWantedClock;
	// UART
	PINSEL0 = PINSEL0 | 0x55;                                     // ustawic piny uar0 odbiornik nadajnik
	U0LCR  |= m8BIT_UART_WORD_LENGTH | mDIVISOR_LATCH_ACCES_BIT; // d³ugosc s³owa, DLAB = 1
	U0DLL = ( unsigned char ) ( ulDivisor & ( unsigned long ) 0xff );
	ulDivisor >>= 8;
	U0DLM = ( unsigned char ) ( ulDivisor & ( unsigned long ) 0xff );
	U0LCR  &= (~mDIVISOR_LATCH_ACCES_BIT);                       // DLAB = 0
	U0IER  |= mRX_DATA_AVALIABLE_INTERRUPT_ENABLE | mTHRE_INTERRUPT_ENABLE ;               

	// INT
	VICVectAddr1  = (unsigned long) UART0_Interrupt;             // set interrupt service routine address
	VICVectCntl1  = mIRQ_SLOT_ENABLE | VIC_UART0_CHANNEL_NR;     // use it for UART 0 Interrupt
	VICIntEnable |= (0x1 << VIC_UART0_CHANNEL_NR);               // Enable UART 0 Interrupt Channel

}


