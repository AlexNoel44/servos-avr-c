/**
* @file servo.h
* @author Alexis Noel
* @version 1.0
* @date 11 mars 2024
* @brief Cette bibliothèque offre des fonctions servant à contrôler des servomoteurs avec un AtMega32u4. Si cette bibliothèque est modifiée, elle pourrait être utilisée avec un autre
* microcontrôleur de la famille AtMega.
*
* Pour le faire il suffit de :
*
* -Consulter la datasheet du MCU en question.
* -Modifier les macros contenant les adresses de port propre au AtMega 32u4 puis les remplacer par les adresses du MCU cible.
* -Ajouter des macros supplémentaires si le MCU cible a plus de registres, puis ajouter ces nouveaux registres dans le switch de la fonction _servoSetPORT.
* -Modifier les registres d'initialisation du TIMER 1 dans l'init si les noms et conditions changent.
* -Modifier le nom du vecteur d'interruption du TIMER 1 s'il n'est pas le même sur le MCU cible.
*
* La gestion des plusieurs servomoteurs se fait en boucle, à chaque appel de la fonction servoInit dans le main, une nouvelle instance de servmoteur est créée et ajoutée à un tableau.
* les fonctions s'assure d'atteindre la bonne instance dans le tableau grâce à l'indice du servomoteur en question. Il est préférable de donner 0 comme indice au premier servomoteur 
* et de continuer à compter de la sorte jusqu'au dernier servomoteur. L'interruption d'un servomoteur tout seul est de 5ms, comme le traitement se fait en boucle, chaque servomoteurs
* ajoute son 5ms. Donc, pour quatre servomteurs par exemple, chaque servomoteur aura un période réelle de 20ms car 4 servomoteurs * 5 ms de période individuel. Il n'y à pas vraiment
* de maximum, en tout cas je n'ai jamais fait de test pour le trouver. Donc, théoriquement il pourrait en avoir autant que dans la limite du possible, la seule chose à prendre en
* compte est le cumul des périodes individuelles de traitement.
*
* Le PWM matériel n'est pas utilisé dans cette bibliothèque, c'est donc un PWM logiciel basé sur le TIMER 1 qui place des broches du AtMega 32u4 à 1 ou 0 pour simuler un PWM. 
*/

#ifndef SERVO_H_
#define SERVO_H_

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

//Nombre de servomoteurs utilisé.
#define SERVO_MAX	5

//Macros des adresses de Data Direction Register pour les port propres au AtMega 32u4.
#define DDRF_ADDR 0x30
#define DDRE_ADDR 0x2D
#define DDRD_ADDR 0x2A
#define DDRC_ADDR 0x27
#define DDRB_ADDR 0x24

/*
* @brief Initialisation d'un servomoteur, l'instance du servomoteur sera placée dans un tableau avec son numéro de pin et son Data Direction register pour etre traité dans les fonctions.
* @param uint8_t servoIndex : index du servomoteur individuel, commencer à 0. uint8_t pin : pin utilisée pour le signal du servo sur le DDR. volatile uint8_t* ddr : adresse du Data Direction Register.
* @return void
*/
void servoInit(uint8_t servoIndex, uint8_t pin, volatile uint8_t* ddr);

/*
* @brief Permet à l'utilisateur de donner au servomoteur un angle entre 0 et 180 degrés.
* @param void
* @return void
*/
void servoSetAngle(uint8_t servoIndex, uint8_t angle);

#endif