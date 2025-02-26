#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "calendar.h"
#include "cJSON.h"

#define DEBUG 0

typedef struct Arguments {
    bool json;
    FILE *output;
    char datetime_str[16];
    struct DateTime {
        int year, month, day;
        int hour, minute, second;
    } datetime;
} Arguments;

bool read_args(int nargc, char **nargv, Arguments *arguments);

void processing_parameter(Arguments *arguments);

void get_calendar_info_str(ChineseCalendarInfo info);

void get_calendar_info_json(ChineseCalendarInfo info);

void print_json(const char *json_str, size_t len);

void print_separator(int len, char separator, char end_char) {
    for (int i = 0; i < len; ++i) {
        printf("%c", separator);
    }
    printf("%c", end_char);
}

#if defined(DEBUG) && DEBUG == 1
void printChineseCalendarInfo(ChineseCalendarInfo info) {
    char *nums[10] = {"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九"};
    char *chinese_calendar_day[30] = {"初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
                                      "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
                                      "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"};
    char *chinese_calendar_month[12] = {"正月", "二月", "三月", "四月", "五月", "六月", "七月", "八月", "九月", "十月",
                                        "冬月", "腊月"};
    char *celestial_stem[10] = {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"};
    char *terrestrial_branch[12] = {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"};
    char *chinese_zodiac[12] = {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"};
    char *solar_terms[24] = {"立春", "雨水", "惊蛰", "春分", "清明", "谷雨", "立夏", "小满", "芒种", "夏至", "小暑", "大暑",
                             "立秋", "处暑", "白露", "秋分", "寒露", "霜降", "立冬", "小雪", "大雪", "冬至", "小寒", "大寒"};
    char *weeks[7] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    char *others[4] = {"小", "大", "", "闰"};
    int separator_num = 60;
    print_separator(separator_num, '-', '\n');
    if (info.code == SUCCESS) {
        printf("%d-%02d-%02d %02d:%02d:%02d %s 今天是本年的第%d天 本月有%d天 本年有%d天\n"
               "农历为: %s%s%s%s年 %s%s(%s)%s [%s%s(%s)年]\n"
               "当前节气是%s%s, 距下个节气%s还有%d天\n"
               "当前的四柱为:\n年 月 日 时\n%s %s %s %s\n%s %s %s %s\n",
               info.calendar.calendar.solar.year, info.calendar.calendar.solar.month, info.calendar.calendar.solar.day,
               info.calendar.calendar.time.hour, info.calendar.calendar.time.minute, info.calendar.calendar.time.second,
               weeks[info.calendar.calendar.solar.week], info.calendar.calendar.solar.dayOfYear, info.calendar.calendar.solar.daysOfMonth,
               info.calendar.calendar.solar.daysOfYear, nums[info.calendar.calendar.lunar.chineseCalendarYear / 1000],
               nums[info.calendar.calendar.lunar.chineseCalendarYear / 100 % 10], nums[info.calendar.calendar.lunar.chineseCalendarYear % 100 / 10],
               nums[info.calendar.calendar.lunar.chineseCalendarYear % 10], others[info.calendar.calendar.lunar.isChineseCalendarLeapMonth + 2],
               chinese_calendar_month[info.calendar.calendar.lunar.chineseCalendarMonth - 1], others[info.calendar.calendar.lunar.isDXYue],
               chinese_calendar_day[info.calendar.calendar.lunar.chineseCalendarDay - 1],
               celestial_stem[info.calendar.calendar.lunar.celestialStem - 1],
               terrestrial_branch[info.calendar.calendar.lunar.terrestrialBranch - 1], chinese_zodiac[info.calendar.calendar.lunar.chineseZodiac - 1],
               solar_terms[info.calendar.calendar.solarTerms.solarTerm], info.calendar.calendar.solarTerms.isToday ? "*" : "",
               solar_terms[info.calendar.calendar.solarTerms.nextSolarTerm], info.calendar.calendar.solarTerms.nextSolarTermRemainDays,
               celestial_stem[info.calendar.calendar.fourPillars.year.celestialStem - 1],
               celestial_stem[info.calendar.calendar.fourPillars.month.celestialStem - 1],
               celestial_stem[info.calendar.calendar.fourPillars.day.celestialStem - 1],
               celestial_stem[info.calendar.calendar.fourPillars.hour.celestialStem - 1],
               terrestrial_branch[info.calendar.calendar.fourPillars.year.terrestrialBranch - 1],
               terrestrial_branch[info.calendar.calendar.fourPillars.month.terrestrialBranch - 1],
               terrestrial_branch[info.calendar.calendar.fourPillars.day.terrestrialBranch - 1],
               terrestrial_branch[info.calendar.calendar.fourPillars.hour.terrestrialBranch - 1]
        );
    } else {
        printf("ERROR\n");
    }
    print_separator(separator_num, '-', '\n');
}
#endif

int main(int argc, char *argv[]) {
    Arguments arguments = {false, stdout, "", {0}};
    if (read_args(argc, argv, &arguments)) {
        // 没有任何时间参数
        if (strlen(arguments.datetime_str) == 0) {
            time_t raw_time;
            time(&raw_time);
            struct tm *time_info = localtime(&raw_time); // 转换为本地时间
            // 获取当前时间写入到 arguments.datetime_str 中
            strftime(arguments.datetime_str, sizeof(arguments.datetime_str), "%Y%m%d%H%M%S", time_info);
        }
        //处理日期数据
        processing_parameter(&arguments);
    }
    // 计算农历数据
    ChineseCalendarInfo info = get_calendar_info(arguments.datetime.year, arguments.datetime.month, arguments.datetime.day,
                                                 arguments.datetime.hour, arguments.datetime.minute, arguments.datetime.second);
#if defined(DEBUG) && DEBUG == 1
    printChineseCalendarInfo(info);
#endif
    if (arguments.json) {
        get_calendar_info_json(info);
    } else {
        get_calendar_info_str(info);
    }
    fclose(arguments.output);
    return 0;
}

bool read_args(int nargc, char **nargv, Arguments *arguments) {
    if (nargv == nullptr || arguments == nullptr) {
        return false;
    }
    int opt;
    struct option long_options[] = {
            {"datetime_str", required_argument, nullptr, 0x7464},
            {"output",       no_argument,       nullptr, 'o'},
            {"json",         no_argument,       nullptr, 'j'},
            {nullptr,        0,                 nullptr, 0}
    };
    int idx = -1;
    while ((opt = getopt_long(nargc, nargv, "o:jhv", long_options, &idx)) != -1) {
        switch (opt) {
            case 0x7464:
                strcpy(arguments->datetime_str, optarg);
                break;
            case 'o':
                arguments->output = fopen(optarg, "wb");
                break;
            case 'j':
                arguments->json = true;
                break;
            default:
                return false;
        }
        idx = -1;
    }
    return true;
}

int parse_part(const char *str, int start, int length) {
    int value = 0;
    for (int j = 0; j < length; j++) {
        value = value * 10 + (str[start + j] - '0');
    }
    return value;
}

void processing_parameter(Arguments *arguments) {
    int positions[] = {0, 4, 6, 8, 10, 12};  // 各部分的起始位置
    int lengths[] = {4, 2, 2, 2, 2, 2};  // 各部分的长度
    int *values[] = {&arguments->datetime.year, &arguments->datetime.month,
                     &arguments->datetime.day, &arguments->datetime.hour,
                     &arguments->datetime.minute, &arguments->datetime.second};

    for (int i = 0; i < 6; i++) {
        *values[i] = parse_part(arguments->datetime_str, positions[i], lengths[i]);
    }

}

void get_calendar_info_str(ChineseCalendarInfo info) {
    printf("%d %d %d %d %d %d %d %d %d %d %s %s %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d",
           info.calendar.calendar.solar.year,
           info.calendar.calendar.solar.month,
           info.calendar.calendar.solar.day,

           info.calendar.calendar.time.hour,
           info.calendar.calendar.time.minute,
           info.calendar.calendar.time.second,
           info.calendar.calendar.solar.week,

           info.calendar.calendar.lunar.chineseCalendarYear,
           info.calendar.calendar.lunar.chineseCalendarMonth,
           info.calendar.calendar.lunar.chineseCalendarDay,
           info.calendar.calendar.lunar.isChineseCalendarLeapMonth ? "true" : "false",
           info.calendar.calendar.lunar.isDXYue ? "true" : "false",

           info.calendar.calendar.lunar.celestialStem,
           info.calendar.calendar.lunar.terrestrialBranch,
           info.calendar.calendar.lunar.chineseZodiac,

           info.calendar.calendar.solarTerms.solarTerm,
           info.calendar.calendar.solarTerms.nextSolarTerm,
           info.calendar.calendar.solarTerms.nextSolarTermRemainDays,
           info.calendar.calendar.solarTerms.isToday ? "true" : "false",

           info.calendar.calendar.fourPillars.year.celestialStem, info.calendar.calendar.fourPillars.year.terrestrialBranch,
           info.calendar.calendar.fourPillars.month.celestialStem, info.calendar.calendar.fourPillars.month.terrestrialBranch,
           info.calendar.calendar.fourPillars.day.celestialStem, info.calendar.calendar.fourPillars.day.terrestrialBranch,
           info.calendar.calendar.fourPillars.hour.celestialStem, info.calendar.calendar.fourPillars.hour.terrestrialBranch
    );
}

void get_calendar_info_json(ChineseCalendarInfo info) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "code", info.code);
    cJSON_AddStringToObject(json, "hash", info.hash);
    if (info.code != SUCCESS) {
        cJSON_AddNullToObject(json, "calendar");
    } else {
        cJSON *calendar = cJSON_AddObjectToObject(json, "calendar");
        cJSON *calendar_time = cJSON_AddObjectToObject(calendar, "time");
        cJSON *calendar_solar = cJSON_AddObjectToObject(calendar, "solar");
        cJSON *calendar_lunar = cJSON_AddObjectToObject(calendar, "lunar");
        cJSON *calendar_solar_terms = cJSON_AddObjectToObject(calendar, "solarTerms");
        cJSON *calendar_four_pillars = cJSON_AddObjectToObject(calendar, "fourPillars");
        cJSON *calendar_four_pillars_year = cJSON_AddObjectToObject(calendar_four_pillars, "year");
        cJSON *calendar_four_pillars_month = cJSON_AddObjectToObject(calendar_four_pillars, "month");
        cJSON *calendar_four_pillars_day = cJSON_AddObjectToObject(calendar_four_pillars, "day");
        cJSON *calendar_four_pillars_hour = cJSON_AddObjectToObject(calendar_four_pillars, "hour");
        cJSON_AddNumberToObject(calendar_time, "hour", info.calendar.calendar.time.hour);
        cJSON_AddNumberToObject(calendar_time, "minute", info.calendar.calendar.time.minute);
        cJSON_AddNumberToObject(calendar_time, "second", info.calendar.calendar.time.second);
        cJSON_AddNumberToObject(calendar_solar, "year", info.calendar.calendar.solar.year);
        cJSON_AddNumberToObject(calendar_solar, "month", info.calendar.calendar.solar.month);
        cJSON_AddNumberToObject(calendar_solar, "day", info.calendar.calendar.solar.day);
        cJSON_AddNumberToObject(calendar_solar, "week", info.calendar.calendar.solar.week);
        cJSON_AddNumberToObject(calendar_solar, "dayOfYear", info.calendar.calendar.solar.dayOfYear);
        cJSON_AddNumberToObject(calendar_solar, "daysOfMonth", info.calendar.calendar.solar.daysOfMonth);
        cJSON_AddNumberToObject(calendar_solar, "daysOfYear", info.calendar.calendar.solar.daysOfYear);
        cJSON_AddNumberToObject(calendar_lunar, "chineseCalendarYear", info.calendar.calendar.lunar.chineseCalendarYear);
        cJSON_AddNumberToObject(calendar_lunar, "chineseCalendarMonth", info.calendar.calendar.lunar.chineseCalendarMonth);
        cJSON_AddNumberToObject(calendar_lunar, "chineseCalendarDay", info.calendar.calendar.lunar.chineseCalendarDay);
        cJSON_AddNumberToObject(calendar_lunar, "celestialStem", info.calendar.calendar.lunar.celestialStem);
        cJSON_AddNumberToObject(calendar_lunar, "terrestrialBranch", info.calendar.calendar.lunar.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_lunar, "chineseZodiac", info.calendar.calendar.lunar.chineseZodiac);
        cJSON_AddBoolToObject(calendar_lunar, "isChineseCalendarLeapMonth",
                              info.calendar.calendar.lunar.isChineseCalendarLeapMonth);
        cJSON_AddBoolToObject(calendar_lunar, "isDXYue", info.calendar.calendar.lunar.isDXYue);
        cJSON_AddNumberToObject(calendar_solar_terms, "solarTerm", info.calendar.calendar.solarTerms.solarTerm);
        cJSON_AddNumberToObject(calendar_solar_terms, "nextSolarTerm", info.calendar.calendar.solarTerms.nextSolarTerm);
        cJSON_AddNumberToObject(calendar_solar_terms, "nextSolarTermRemainDays",
                                info.calendar.calendar.solarTerms.nextSolarTermRemainDays);
        cJSON_AddBoolToObject(calendar_solar_terms, "isToday", info.calendar.calendar.solarTerms.isToday);
        cJSON_AddNumberToObject(calendar_four_pillars_year, "celestialStem",
                                info.calendar.calendar.fourPillars.year.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_year, "terrestrialBranch",
                                info.calendar.calendar.fourPillars.year.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_four_pillars_month, "celestialStem",
                                info.calendar.calendar.fourPillars.month.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_month, "terrestrialBranch",
                                info.calendar.calendar.fourPillars.month.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_four_pillars_day, "celestialStem",
                                info.calendar.calendar.fourPillars.day.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_day, "terrestrialBranch",
                                info.calendar.calendar.fourPillars.day.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_four_pillars_hour, "celestialStem",
                                info.calendar.calendar.fourPillars.hour.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_hour, "terrestrialBranch",
                                info.calendar.calendar.fourPillars.hour.terrestrialBranch);
    }
    char *json_str = cJSON_Print(json);
    size_t len = strlen(json_str);
    print_json(json_str, len);
    cJSON_Delete(json); // 清理分配的内存
}

void print_json(const char *json_str, size_t len) {
    char *result = (char *) malloc(sizeof(char) * len);
    int pos = 0;
    for (size_t i = 0; i < len; i++) {
        if (json_str[i] != '\t' && json_str[i] != '\n') {
            result[pos++] = json_str[i];
        }
    }
    result[pos] = 0;
    printf("%s", result);
    free(result);
}
