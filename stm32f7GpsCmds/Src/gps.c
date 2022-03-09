/*
 * gps.c
 *
 *  This file contains functions to read and parse data from GPS
 *
 */

/**
 * --------------------- HEADER FILES ----------------
 */
#include "gps.h"
#include "dbgUart.h"
#include "usart.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include "dbgCmds.h"
#include "minmea.h"
#include "string.h"
#include "stream_buffer.h"
#include "rtc.h"

/**
 * --------------------- MACROS ----------------
 */

#define RX_QUEUE_SIZE  				512		//RX buffer size
#define MAXLEN 						512		//MAX length of message to be handled
#define TRIGGER_SIZE				sizeof(uint8_t)	// Trigger size of uint8_t

#define GPS_GET_STRING_MAX_TRIES 	3000	// Maximum retries to get the string
#define GPS_RX_TIMEOUT_MS      		2000ul	// Maximum timeout
#define MAX_RETRIES					10		// Retries to get GPS data

#define NMEA_PROMPT					'$'		// NMEA prompt string
#define RMC_TAG						"RMC,"	// RMC tag for
#define NULL_CHAR					'\0'	// Null string
#define	NEW_LINE					'\n'	// new line
#define LEN_GPRMC_TAG				6		// Length of RMC tag

#define BASE_YEAR					2000	//Set base year to 2000

/**
 * --------------------- GLOBAL VARIABLES ----------------
 * Note : Make all global variable as static
 */
static UartHandle_t gps_uart_handle;			// Uart handle for gps
static uint8_t puc_rx_stream[RX_QUEUE_SIZE];	// Static buffer for stream
static StaticStreamBuffer_t static_stream;		// Static stream for gps data
static StreamBufferHandle_t rx_stream_handle;	// Handle for stream buffer


/**
 * ----------- FUCNTION DEFINITIONS ------
 */


BaseType_t gps_init(UART_HandleTypeDef *huart)
{
	/* Check for NULL handle*/
	if(huart == NULL)
	{
		DbgPrintf("gpsInit: Failed to initalize the GPS UART\n");
		DbgPrintf("gpsInit: NULL uart handle\n");
		return pdFAIL;
	}

	// Initialize UART for GPS
	gps_uart_handle = uartHandleInit(huart);

	//Initialize Rx stream
	rx_stream_handle = xStreamBufferCreateStatic(RX_QUEUE_SIZE, TRIGGER_SIZE,
													puc_rx_stream, &static_stream);

	DbgPrintf("GPS initialized successfully\n");
	return pdPASS;
}



void conv_gps_to_rtc(struct minmea_sentence_rmc *px_frame,
						RTC_TimeTypeDef *px_time, RTC_DateTypeDef *px_date)
{
	/* Store time */
	px_time->Hours = px_frame->time.hours;
	px_time->Minutes = px_frame->time.minutes;
	px_time->Seconds = px_frame->time.seconds;
	px_time->SubSeconds = 0;
    //these values are vaguely defined in the tech ref
	px_time->TimeFormat = RTC_HOURFORMAT12_PM;       /*!< Specifies the RTC AM/PM Time->
                                 This parameter can be a value of @ref RTC_AM_PM_Definitions */

	px_time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;  /*!< Specifies RTC_DayLightSaveOperation: the value of hour adjustment.
                                 This parameter can be a value of @ref RTC_DayLightSaving_Definitions */
	px_time->StoreOperation = RTC_STOREOPERATION_SET;  /*!< Specifies RTC_StoreOperation value to be written in the BCK bit
                                 in CR register to store the operation.
                                 This parameter can be a value of @ref RTC_StoreOperation_Definitions */


	/* Store date */
	px_date->Date = px_frame->date.day;
	px_date->Month = px_frame->date.month;
	px_date->Year = px_frame->date.year;
	px_date->WeekDay = find_day(px_date);

}


BaseType_t gps_get_uart_string(uint8_t *puc_gps_str)
{

	uint8_t puc_gps_data[RX_QUEUE_SIZE];
	size_t bytes;
	bool rx_complete = false;
	BaseType_t success = pdFAIL;

	/* Start stream */
	if (UART_Rx_Stream_Start(gps_uart_handle, rx_stream_handle) != UART_REQ_BUSY)
	{
		/* Return if strean failed to start */
		DbgPrintf("Failed to start the UART_RX_STREAM\n");
		return pdFAIL;
	}

	/* Run this loop for GPS_GET_STRING_MAX_TRIES */
	for (int a = 0; a < GPS_GET_STRING_MAX_TRIES; a++)
	{
		rx_complete = false;

		// Read blocking forever waiting for data on the stream
		bytes = xStreamBufferReceive(rx_stream_handle, puc_gps_data,
						RX_QUEUE_SIZE, pdMS_TO_TICKS(GPS_RX_TIMEOUT_MS));
		if (bytes > 0)
		{
			/* Check if first character is received */
			if (puc_gps_data[0] == NMEA_PROMPT)
			{

				int j = 0;
				while (j < MAXLEN)
				{
					int k = 0;
					while (k < bytes)
					{
						puc_gps_str[j] = puc_gps_data[k];
						if (puc_gps_data[k] == NEW_LINE)
						{
							puc_gps_str[j + 1] = NULL_CHAR;
							rx_complete = true;
							break;
						}
						k++;
						j++;
					}
					if (rx_complete == false)
					{
						bytes = xStreamBufferReceive(rx_stream_handle, puc_gps_data,
								RX_QUEUE_SIZE, pdMS_TO_TICKS(GPS_RX_TIMEOUT_MS));
					}
					else
					{
						break;
					}
				}
				// Check if the string ended and the string contains $xxRMC, message
				if (rx_complete)
				{
					if (memcmp( RMC_TAG, (const char*) &puc_gps_str[3],
							strlen(RMC_TAG)) == 0)
					{
						success = pdPASS;
						break;
					}
				}
			}
		}
		else
		{
			//TODO: handle timeout?
		}
	}
	// Stop stream
	if(UART_Rx_Stream_Stop(gps_uart_handle) != UART_REQ_READY)
	{
		// ToDo : Handle this failure
		DbgPrintf("gps_get_uart_string: Failed to stop UART_Rx_Stream_Stop()\n");
	}

	return success;
}


BaseType_t get_gps_time(struct minmea_sentence_rmc *px_frame)
{
	uint8_t puc_gps_buffer[RX_QUEUE_SIZE];
	BaseType_t status = pdFAIL;

	// Find RMC message and decode
	// Below loop should run forever

	// ToDo: make changes after checking with max count
	for (int a = 0; a < MAX_RETRIES; a++)
	{
		// Get GPS receive string
		if (gps_get_uart_string(puc_gps_buffer) == true)
		{
			// Decode $GPRMC message
			if (parse_rmc(puc_gps_buffer, px_frame))
			{
				status = pdPASS;
				break;
			}
		}
	}
	return status;
}


BaseType_t parse_rmc(uint8_t *puc_gps_buffer, struct minmea_sentence_rmc *px_frame)
{
	/* Check for NULL pointer */
	if (!puc_gps_buffer || !px_frame )
	{
		DbgPrintf("Invalid puc_gps_buffer or px_frame\n");
		return pdFAIL;
	}

	/* Parse the id */
	switch (minmea_sentence_id((const char *)puc_gps_buffer, false))
	{
		case MINMEA_SENTENCE_RMC:
			if (minmea_parse_rmc(px_frame, (const char *)puc_gps_buffer))
			{
				printhdr();
				DbgPrintf("$xxRMC Time: %02d:%02d:%02d.%d %02d/%02d/%d UTC\n\r",
					px_frame->time.hours,
					px_frame->time.minutes,
					px_frame->time.seconds,
					px_frame->time.microseconds,
					px_frame->date.month,
					px_frame->date.day,
					px_frame->date.year + BASE_YEAR);

				//success, return to caller
				return pdPASS;
			}
			else
			{
				DbgPrintf("$xxRMC sentence is bad\n\r");
			}
			break;

	default:
		// bad sentence or not RMC
		DbgPrintf("Bad sentence??????\n\r");
	}

	return pdFAIL;
}
