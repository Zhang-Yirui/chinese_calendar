#ifndef CALENDAR_CALENDAR_H
#define CALENDAR_CALENDAR_H

#define USE_UNION 1
#define USE_STRUCT (!(USE_UNION))
#include "stdio.h"
#include <stdbool.h>
#include "hash.h"

typedef enum ResultCode {
    SUCCESS = 1000,
    ERROR
} ResultCode;
#if defined(USE_UNION) && USE_UNION == 1
typedef union Calendar {
    uint8_t data[116];
    struct {
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
    } calendar;
} Calendar;
#endif

#if defined(USE_STRUCT) && USE_STRUCT == 1
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
#endif

typedef struct ChineseCalendarInfo {
    ResultCode code;
    Calendar calendar;
    char hash[17];
} ChineseCalendarInfo;

ChineseCalendarInfo get_calendar_info(int year, int month, int day, int hour, int minute, int second);

#endif //CALENDAR_CALENDAR_H
