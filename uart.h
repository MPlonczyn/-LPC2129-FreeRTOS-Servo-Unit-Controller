#define RECEIVER_SIZE 100
#define TRANSMITER_SIZE 100
#define NULL '\0'

//DECODE
enum eRecieverStatus {EMPTY, READY, OVERFLOW};
enum eTransmiterStatus {FREE, BUSY};

void UART_InitWithInt(unsigned int uiBaudRate);
void Reciever_PutCharacterToBuffer(char cCharacter);
enum eRecieverStatus eReciever_GetStatus(void);
void Reciever_GetStringCopy(char * ucDestination);
enum CompResult eCompareString(char pcStr1[], char pcStr2[]);

//TRANSMIT
char Transmiter_GetCharacterFromBuffer(void);
void Transmiter_SendString(char cString[]);
enum eTransmiterStatus Transmiter_GetStatus();
void DisplayValue(unsigned int uiCounterValue);
