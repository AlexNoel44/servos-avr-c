/*
* @file servo.c
* @author Alexis Noel
* @version 2.2
* @date 11 mars 2024
* @brief Definition des fonctions servant � int�ragir avec des servomoteurs.
*/

#include "servo.h"

//(angle * 4000) / 180) + 1000; //Le 22 vient de la divison de 4000/180, c'ets pour simplifier le calcul, 1000 est le OCR_MIN ce qui veut onc dire que si l'angle est �gal � 0 un 1000 de OCR1A est garanti pour donner 500us de Ton.
#define OCR_VALUE	9999
#define OCR_MIN		1000
#define OCR_POS(a) ((a * 22) + OCR_MIN)

volatile uint8_t ind=0;

volatile uint8_t flag=0;

typedef struct
{
	uint8_t pinx;
	uint8_t pos;
	volatile uint8_t* ddrx;
	volatile uint8_t* portx;
}Servo;

Servo servos[SERVO_MAX];

////////////////////////////////////////////////////////////////////////////////"FONCTIONS PRIV�ES"//////////////////////////////////////////////////////////////////////////////////

/*
* @brief fonction qui attribue un pin � une instance de servo qui se trouve dans le tbaleau servos
* @param uint8_t servo : index du servomoteur cible, uint8_t pin : pin � attribuer � l'instance de servomoteur cible
* @return void
*/
void _servoSetPin(uint8_t servo, uint8_t pin)
{
	servos[servo].pinx=pin;
}

/*
* @brief fonction qui retorune le num�ro de la pin du servomoteur cible.
* @param uint8_t servo : index du servomoteur cible.
* @return uint8_t servos[servo].pinx : num�ro de la pin de l'instance du servomoteur cible.
*/
uint8_t _servoGetPin(uint8_t servo)
{
	return servos[servo].pinx;	
}

/***************************************************************************************************************************************************************************************************************/

/*
* @brief fonction qui attribue le Data Direction Register � une instance de servomoteur qui se trouve dans le tableau servos
* @param uint8_t servo : index du servomoteur cible, volatile uint8_t* ddr : adresse du Data Direction Register attribu�e � la pin de signal de l'instance du servomoteur cible.
* @return void
*/
void _servoSetDDR(uint8_t servo, volatile uint8_t* ddr)
{
	servos[servo].ddrx=ddr;
}

/*
* @brief fonction qui retourne l'addresse DDR de l'instance du servomoteur cible.
* @param uint8_t servo : index du servomoteur cible.
* @return volatile uint8_t servos[servo].ddrx : adresse DDR de l'instance du servomoteur cible.
*/
volatile uint8_t* _servoGetDDR(uint8_t servo)
{
	return servos[servo].ddrx;
}

/***************************************************************************************************************************************************************************************************************/

/*
* @brief fonction qui attribue un PORT � l'instance du servomoteur cible gr�ce � l'adresse du DDR, qui permet donc d'envoyer un signal 1 ou 0 sur la pin de signal du servomoteur cible.
* @param uint8_t servo : index du servomoteur cible, volatile uint8_t* ddr : adresse du Data Direction Register attribu�e � la pin de signal de l'instance du servo cible.
* @return void
*/
void _servoSetPORT(uint8_t servo, volatile uint8_t* ddr)
{
	switch ((uintptr_t)ddr)
	{
		case DDRF_ADDR:
			servos[servo].portx=&PORTF;
			break;
		case DDRE_ADDR:
			servos[servo].portx=&PORTE;
			break;
		case DDRD_ADDR:
			servos[servo].portx=&PORTD;
			break;
		case DDRC_ADDR:
			servos[servo].portx=&PORTC;
			break;
		case DDRB_ADDR:
			servos[servo].portx=&PORTB;
			break;
	}
}

/*
* @brief fonction qui retourne l'addresse PORT de l'instance du servomoteur cible.
* @param uint8_t servo : index du servomoteur cible.
* @return volatile uint8_t servos[servo].portx : adresse PORT de l'instance du servomoteur cible.
*/
volatile uint8_t* _servoGetPORT(uint8_t servo)
{
	return servos[servo].portx;
}

/***************************************************************************************************************************************************************************************************************/

void _servoSetPos(uint8_t servo, uint8_t pos)
{
	if(pos<=180 && pos>=0)
		servos[servo].pos=pos;
}

/***************************************************************************************************************************************************************************************************************/

uint8_t _servoGetPos(uint8_t servo)
{
	return servos[servo].pos;
}

////////////////////////////////////////////////////////////////////////////////"FONCTIONS PUBLIQUES"/////////////////////////////////////////////////////////////////////////////////

void servoInit(uint8_t servoIndex, uint8_t pin, volatile uint8_t* ddr)
{	
	_servoSetPin(servoIndex, pin);									
	_servoSetDDR(servoIndex, ddr);	
	*(_servoGetDDR(servoIndex)) |= (1<<_servoGetPin(servoIndex));
	
	TCCR1B |= (1<<CS11) | (1<<WGM12);	//Prescaler de 8 (CS11)
	TCNT1=0;
	OCR1A = 9999;						//200Hz
	TIMSK1 |= (1<<OCIE1A);				//D�masquement de l'interruption.
	sei();								//Activation des interruptions.

	_servoSetPORT(servoIndex, _servoGetDDR(servoIndex));
}

/***************************************************************************************************************************************************************************************************************/

void servoSetAngle(uint8_t servoIndex, uint8_t angle)
{
	_servoSetPos(servoIndex, angle);
}

///////////////////////////////////////////////////////////////////////////VECTEUR D'INTERRUPTION DU TIMER 1//////////////////////////////////////////////////////////////////////////

/*
* @brief Vecteur d'interruption du TIMER 1. Qaund le flag=1, La pin de l'instance du servomoteur cible est mise � 1 et la dur�e de temps y est aussi determin�e.
* Donc cette dur�e sera determin� dans ce if, ensuite comme le flag=0 nous passerons dans le else. Dans le esle il est determin� la dur�e pour laquelle la pin sera mise � 0 .
* Par exemple, si l'angle est de 180 degr�s, un servomoteur normal aura besoin d'une pulsation durant au moins 2,5ms. Donc, quand le flag est � 1 et que l'on rentre dans l'interruption,
* Le if sera execut� et fixera une la dur�e avant une nouvelle interruption � 2,5ms. Apres, le 2,5ms, comme le flag est � 0, le else sera �x�cut�. Dans ce eles, il ser afix� la dur�e
* pour laquelle la pin doit rester � 0. Comme la plupart des servomoteurs marchent � avec une fr�quence de 200Hz (5ms) la dur�e pour cet exemple sera de 5ms-2,5ms donc de 2,5ms.
* Donc la pin sera allum�e 2,5ms et �teinte 2,5ms ce qui fait donc un signal PWM qui contr�le les servomoteurs. Cette op�ration est r�p�t�e pour chaque servmoteurs les uns � la suite
* des autres.  
* @param TIMER_COMPA_vect : vecteur d'interruption du timer 1.
* @return ISR.
*/
ISR(TIMER1_COMPA_vect)
{
	if (flag)
	{
		flag=0;
		*_servoGetPORT(ind) |= (1<<_servoGetPin(ind));
		OCR1A = OCR_POS(_servoGetPos(ind));
	} 
	else
	{
		flag=1;
		*_servoGetPORT(ind) &= ~(1<<_servoGetPin(ind));
		OCR1A = OCR_POS(_servoGetPos(ind));
		ind >= SERVO_MAX-1 ? ind=0 : ind++;
	}	
}