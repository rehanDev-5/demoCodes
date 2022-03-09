/*
 * dbg_gps.c
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "dbgUart.h"
#include "dbgCmds.h"
#include <errno.h>
#include "usart.h"
#include "minmea.h"
#include "rtc.h"
#include "gps.h"
#include "time_service.h"

// instead of a header file
BaseType_t gpsUARTReceiveStr(uint8_t *receivedStr, int *len, int maxLen,
                             uint32_t millisec);

static int get_gps(int argc, char **argv, int min_args)
{
	/* This is a test, collect a gps input loop */
	struct minmea_sentence_rmc frame;
	if(!get_gps_time(&frame)) DbgPrintf("Error getting GPS data\n");
    return 0; //success
}

//fixme - remove, use gps, or take cmd line inputs and use locks
static int set_date(int argc, char **argv, int min_args)
{
    RTC_DateTypeDef date;

    date.Date = 23;
    date.Month = 7;
    date.Year = 20;
    date.WeekDay = find_day(&date);

    DbgPrintf("Setting date to %s %02d.%02d.%04d UTC\n\r",
                daynames[date.WeekDay],
                date.Date,
                date.Month,
                date.Year + 2000);

    HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);

    return 0; //success
}

//fixme - remove, use gps, or take cmd line inputs and use locks
static int set_time(int argc, char **argv, int min_args)
{
    RTC_TimeTypeDef time;

    time.Hours = 14;
    time.Minutes = 28;
    time.Seconds = 0;
    time.SubSeconds = 0;
    //these values are vaguely defined in the tech ref
    time.TimeFormat = RTC_HOURFORMAT12_PM;       /*!< Specifies the RTC AM/PM Time.
                                 This parameter can be a value of @ref RTC_AM_PM_Definitions */

    time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;  /*!< Specifies RTC_DayLightSaveOperation: the value of hour adjustment.
                                 This parameter can be a value of @ref RTC_DayLightSaving_Definitions */
    time.StoreOperation = RTC_STOREOPERATION_SET;  /*!< Specifies RTC_StoreOperation value to be written in the BCK bit
                                 in CR register to store the operation.
                                 This parameter can be a value of @ref RTC_StoreOperation_Definitions */

    DbgPrintf("Setting time to: %02d:%02d:%02d\n\r",
           time.Hours,
           time.Minutes,
           time.Seconds);

    HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);

    return 0; //success
}

/*
 * ToFix: This function currently does not get time from RTC,
 * osMutexWait(rtc_lock, osWaitForever) waits to get access.
 */
static int rtc_time(int argc, char **argv, int min_args)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    int debug_print = 1;	// make it one to print the date-time from RTC
    get_rtc_time_date(&time, &date, debug_print);

    return 0; //success
}

static int set_rtc_to_gps(int argc, char **argv, int min_args)
{
	DbgPrintf("Setting RTC using GPS...\n");
	gps_time_sync();
    return 0; //success
}

// all commands return 0 for good, or non-zero for an error
const CommandStruct_t gps_mon_table[] = {	// const CommandStruct_t const gps_mon_table[]
    {"GPS", 0, "Print gps data", &get_gps},
    {"TIME", 0, "loop to print time and date from rtc", &rtc_time},
    {"SET_DATE", 0, "set date mm/dd/yyyy", &set_date},
    {"SET_TIME", 0, "set time hh:mm:ss", &set_time},
    {"SET_RTC", 0, "set time, date using gps", &set_rtc_to_gps},
   {NULL, 0, NULL, NULL}
};
