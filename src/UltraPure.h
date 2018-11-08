/*
 * UltraPure.h
 *
 *  Created on: Nov 5, 2018
 *      Author: Tanner
 */

#ifndef ULTRAPURE_H_
#define ULTRAPURE_H_


/* PIN DEFINITIONS */
#define PUMP_P_PIN				7
#define PUMP_M_PIN				6

#define SENSOR_1_NC_PIN			5
#define SENSOR_1_COM_PIN		4
#define SENSOR_1_NO_PIN			3

#define SENSOR_2_NC_PIN			2
#define SENSOR_2_COM_PIN		1
#define SENSOR_2_NO_PIN			0

#define LED_OUTPUT_RED_PIN		11
#define LED_OUTPUT_AQUA_PIN		10

/* PORT DEFINITIONS */
#define PUMP_P_PORT				GPIOA
#define PUMP_M_PORT				GPIOA

#define SENSOR_1_NC_PORT		GPIOA
#define SENSOR_1_COM_PORT		GPIOA
#define SENSOR_1_NO_PORT		GPIOA

#define SENSOR_2_NC_PORT		GPIOA
#define SENSOR_2_COM_PORT		GPIOA
#define SENSOR_2_NO_PORT		GPIOA

#define LED_OUTPUT_RED_PORT		GPIOA
#define LED_OUTPUT_AQUA_PORT	GPIOA



#define MEASUREMENT_FREQUENCY	1000
#define PUMP_DELAY				45


typedef enum{
	ACTIVE,
	INACTIVE
}pump_status_t;

typedef enum{
	NORMAL,
	ALARM
}sensor_status_t;

typedef enum{
	RED,
	AQUA
}led_color_t;

typedef enum{
	SENSOR_1,
	SENSOR_2
}sensor_id_t;


typedef struct{
	pump_status_t 	pump;
	sensor_status_t	sensor_1;
	sensor_status_t sensor_2;
}ultra_pure_status_t;


void ultra_pure_init( void );
void sensor_monitor( sensor_id_t sensor_id );
void pump_monitor( void );
void led_control( void );


#endif /* ULTRAPURE_H_ */
