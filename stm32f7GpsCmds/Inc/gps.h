/*
 * gps.h
 *
 */

#ifndef INC_GPS_H_
#define INC_GPS_H_

/**
 * ----------- HEADER FILES -----------
 */

#include "FreeRTOS.h" 	// for `BaseType_t`
#include "minmea.h"
#include "rtc.h"
#include <stdint.h>

/**
 * ----------- MACROS -----------
 */


/**
 * ----------- FUNCTION DECLARATIONS -----------
 */


/**
 * @brief This function should be called before starting the thread.
 * @param huart  UART handle of UART connected to GPS
 * @retval pdPASS on success, pdFAIL on failure
 * @note
 */
BaseType_t gps_init(UART_HandleTypeDef *huart);


/**
 * @brief This function gets $GPRMC message from GPS
 * @param puc_gps_str  Buffer to store uart string
 * @retval pdPASS on success, pdFAIL on failure
 * @note xStreamBufferReceive sometimes returns a partial string,
 * 		 Thus it needs to be called multiple times
 */
BaseType_t gps_get_uart_string(uint8_t *puc_gps_str);



/**
 * @brief This function fetches time from gps.
 * @param pxFrame minmea_sentence_rmc structure to store time
 * @retval pdPass on success or pdFail on failure
 */
BaseType_t get_gps_time(struct minmea_sentence_rmc *pxFrame);


/**
 * @brief This function takes $GPRMC message sequence from GPS and
 * 		  decodes it to RMC format. Also prints the date-time info.
 * @param puc_gps_buffer Buffer containing GPRMC data
 * @param px_frame minmea_sentence_rmc structure to store rmc data
 */

BaseType_t parse_rmc(uint8_t *puc_gps_buffer, struct minmea_sentence_rmc *px_frame);

/**
 * @brief This function converts @ref minmea_sentence_rmc RMC time format to
 * 		  @ref RTC_TimeTypeDef and  @ref RTC_DateTypeDef
 * @param px_frame rmc data @ref minmea_sentence_rmc
 * @param px_time Structure to store time @ref RTC_TimeTypeDef
 * @param px_date Structure to store date @ref  RTC_DateTypeDef
 *
 */
void conv_gps_to_rtc(struct minmea_sentence_rmc *px_frame,	RTC_TimeTypeDef *px_time, RTC_DateTypeDef *px_date);


#endif /* INC_GPS_H_ */
