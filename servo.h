enum ServoState{
	CALLIB,
	IDLE,
	IN_PROGRESS
};

struct Servo{
	enum ServoState eState; 
	unsigned int uiCurrentPosition; 
	unsigned int uiDesiredPosition;
	unsigned int uiWaitTicks;
	unsigned int uiSpeed;
};

extern struct Servo sServo;

void ServoInit(unsigned int uiServoFrequency);
void ServoCallib(void);
void ServoGoTo(unsigned int uiPosition);
void ServoWait(unsigned int uiTicks);
void ServoSpeed(unsigned int uiDelay);
void Automat(void);
