#ifndef CALENDAR_CALENDAR_H
#define CALENDAR_CALENDAR_H

#include <stdbool.h>
#include "hash.h"

typedef enum ResultCode {
    SUCCESS = 1000,
    ERROR
} ResultCode;

typedef struct Calendar {
    struct TimeData {
        int hour, minute, second;
    } time;
    struct SolarData {
        int year, month, day, week, daysOfMonth, dayOfYear, daysOfYear;
    } solar;
    struct LunarData {
        int chineseCalendarYear, chineseCalendarMonth, chineseCalendarDay;
        int celestialStem, terrestrialBranch, chineseZodiac;
        bool isChineseCalendarLeapMonth, isDXYue;
    } lunar;
    struct SolarTerms {
        int solarTerm, nextSolarTerm, nextSolarTermRemainDays;
        bool isToday;
    } solarTerms;
    struct FourPillars {
        struct {
            int celestialStem, terrestrialBranch;
        } year, month, day, hour;
    } fourPillars;
} Calendar;

typedef struct ChineseCalendarInfo {
    ResultCode code;
    Calendar calendar;
    char hash[17];
} ChineseCalendarInfo;

ChineseCalendarInfo get_calendar_info(int year, int month, int day, int hour, int minute, int second);

#endif //CALENDAR_CALENDAR_H
