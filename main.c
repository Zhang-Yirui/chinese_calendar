#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "calendar.h"
#include "cJSON.h"

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

#define AAA 0
#if AAA
//void generate_data();

void print_separator(int len, char separator, char end_char) {
    for (int i = 0; i < len; ++i) {
        printf("%c", separator);
    }
    printf("%c", end_char);
}

void printChineseCalendarInfo(ChineseCalendarInfo info) {
    char *celestial_stem[10] = {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"};
    char *terrestrial_branch[12] = {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"};
    char *chinese_zodiac[12] = {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"};

    switch (info.code) {
        case SUCCESS:
            printf("SUCCESS\n");
            break;
        case WARNING:
            printf("WARNING\n");
            break;
        case ERROR:
            printf("ERROR\n");
    }

    printf("Time - Hour: %d, Minute: %d, Second: %d\n",
           info.calendar.time.hour,
           info.calendar.time.minute,
           info.calendar.time.second);

    printf("Solar - Year: %d, Month: %d, Day: %d, Week: %d, Days of Month: %d, Day of Year: %d, Days of Year: %d\n",
           info.calendar.solar.year,
           info.calendar.solar.month,
           info.calendar.solar.day,
           info.calendar.solar.week,
           info.calendar.solar.daysOfMonth,
           info.calendar.solar.dayOfYear,
           info.calendar.solar.daysOfYear);

    printf("Lunar - Year: %d, Month: %d, Day: %d, Celestial Stem: %d, Terrestrial Branch: %d, Chinese Zodiac: %d\n",
           info.calendar.lunar.chineseCalendarYear,
           info.calendar.lunar.chineseCalendarMonth,
           info.calendar.lunar.chineseCalendarDay,
           info.calendar.lunar.celestialStem,
           info.calendar.lunar.terrestrialBranch,
           info.calendar.lunar.chineseZodiac);
    printf("Is Lunar Leap Month: %s, Is DXYue: %s\n",
           info.calendar.lunar.isChineseCalendarLeapMonth ? "Yes" : "No",
           info.calendar.lunar.isDXYue ? "Yes" : "No");

    printf("Solar Terms - Current: %d, Next: %d, Next Remain Days: %d, Is Today: %s\n",
           info.calendar.solarTerms.solarTerm,
           info.calendar.solarTerms.nextSolarTerm,
           info.calendar.solarTerms.nextSolarTermRemainDays,
           info.calendar.solarTerms.isToday ? "Yes" : "No");

    printf("Four Pillars - Year: %d-%d, Month: %d-%d, Day: %d-%d, Hour: %d-%d\n",
           info.calendar.fourPillars.year.celestialStem, info.calendar.fourPillars.year.terrestrialBranch,
           info.calendar.fourPillars.month.celestialStem, info.calendar.fourPillars.month.terrestrialBranch,
           info.calendar.fourPillars.day.celestialStem, info.calendar.fourPillars.day.terrestrialBranch,
           info.calendar.fourPillars.hour.celestialStem, info.calendar.fourPillars.hour.terrestrialBranch);
    print_separator(100, '-', '\n');
    if (info.code == SUCCESS) {
        printf("%s%s\n", celestial_stem[info.calendar.lunar.celestialStem - 1],
               terrestrial_branch[info.calendar.lunar.chineseZodiac - 1]);
        printf("Four Pillars - Year: %s%s, Month: %s%s, Day: %s%s, Hour: %s%s\n",
               celestial_stem[info.calendar.fourPillars.year.celestialStem - 1],
               terrestrial_branch[info.calendar.fourPillars.year.terrestrialBranch - 1],
               celestial_stem[info.calendar.fourPillars.month.celestialStem - 1],
               terrestrial_branch[info.calendar.fourPillars.month.terrestrialBranch - 1],
               celestial_stem[info.calendar.fourPillars.day.celestialStem - 1],
               terrestrial_branch[info.calendar.fourPillars.day.terrestrialBranch - 1],
               celestial_stem[info.calendar.fourPillars.hour.celestialStem - 1],
               terrestrial_branch[info.calendar.fourPillars.hour.terrestrialBranch - 1]);
    }

    print_separator(100, '-', '\n');
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
    ChineseCalendarInfo info = get_calendar_info(arguments.datetime.year, arguments.datetime.month,
                                                 arguments.datetime.day, arguments.datetime.hour,
                                                 arguments.datetime.minute, arguments.datetime.second);
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
    const char *options = "o:jhv";
    int idx = -1;
    while ((opt = getopt_long(nargc, nargv, options, long_options, &idx)) != -1) {
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
//    ChineseCalendarInfo info = get_calendar_info(year, month, day, hour, minute, second);
    printf("%d %d %d %d %d %d %d %d %d %d %s %s %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d",
           info.calendar.solar.year,
           info.calendar.solar.month,
           info.calendar.solar.day,

           info.calendar.time.hour,
           info.calendar.time.minute,
           info.calendar.time.second,
           info.calendar.solar.week,

           info.calendar.lunar.chineseCalendarYear,
           info.calendar.lunar.chineseCalendarMonth,
           info.calendar.lunar.chineseCalendarDay,
           info.calendar.lunar.isChineseCalendarLeapMonth ? "true" : "false",
           info.calendar.lunar.isDXYue ? "true" : "false",

           info.calendar.lunar.celestialStem,
           info.calendar.lunar.terrestrialBranch,
           info.calendar.lunar.chineseZodiac,

           info.calendar.solarTerms.solarTerm,
           info.calendar.solarTerms.nextSolarTerm,
           info.calendar.solarTerms.nextSolarTermRemainDays,
           info.calendar.solarTerms.isToday ? "true" : "false",

           info.calendar.fourPillars.year.celestialStem, info.calendar.fourPillars.year.terrestrialBranch,
           info.calendar.fourPillars.month.celestialStem, info.calendar.fourPillars.month.terrestrialBranch,
           info.calendar.fourPillars.day.celestialStem, info.calendar.fourPillars.day.terrestrialBranch,
           info.calendar.fourPillars.hour.celestialStem, info.calendar.fourPillars.hour.terrestrialBranch
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
        cJSON_AddNumberToObject(calendar_time, "hour", info.calendar.time.hour);
        cJSON_AddNumberToObject(calendar_time, "minute", info.calendar.time.minute);
        cJSON_AddNumberToObject(calendar_time, "second", info.calendar.time.second);
        cJSON_AddNumberToObject(calendar_solar, "year", info.calendar.solar.year);
        cJSON_AddNumberToObject(calendar_solar, "month", info.calendar.solar.month);
        cJSON_AddNumberToObject(calendar_solar, "day", info.calendar.solar.day);
        cJSON_AddNumberToObject(calendar_solar, "week", info.calendar.solar.week);
        cJSON_AddNumberToObject(calendar_solar, "dayOfYear", info.calendar.solar.dayOfYear);
        cJSON_AddNumberToObject(calendar_solar, "daysOfMonth", info.calendar.solar.daysOfMonth);
        cJSON_AddNumberToObject(calendar_solar, "daysOfYear", info.calendar.solar.daysOfYear);
        cJSON_AddNumberToObject(calendar_lunar, "chineseCalendarYear", info.calendar.lunar.chineseCalendarYear);
        cJSON_AddNumberToObject(calendar_lunar, "chineseCalendarMonth", info.calendar.lunar.chineseCalendarMonth);
        cJSON_AddNumberToObject(calendar_lunar, "chineseCalendarDay", info.calendar.lunar.chineseCalendarDay);
        cJSON_AddNumberToObject(calendar_lunar, "celestialStem", info.calendar.lunar.celestialStem);
        cJSON_AddNumberToObject(calendar_lunar, "terrestrialBranch", info.calendar.lunar.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_lunar, "chineseZodiac", info.calendar.lunar.chineseZodiac);
        cJSON_AddBoolToObject(calendar_lunar, "isChineseCalendarLeapMonth",
                              info.calendar.lunar.isChineseCalendarLeapMonth);
        cJSON_AddBoolToObject(calendar_lunar, "isDXYue", info.calendar.lunar.isDXYue);
        cJSON_AddNumberToObject(calendar_solar_terms, "solarTerm", info.calendar.solarTerms.solarTerm);
        cJSON_AddNumberToObject(calendar_solar_terms, "nextSolarTerm", info.calendar.solarTerms.nextSolarTerm);
        cJSON_AddNumberToObject(calendar_solar_terms, "nextSolarTermRemainDays",
                                info.calendar.solarTerms.nextSolarTermRemainDays);
        cJSON_AddBoolToObject(calendar_solar_terms, "isToday", info.calendar.solarTerms.isToday);
        cJSON_AddNumberToObject(calendar_four_pillars_year, "celestialStem",
                                info.calendar.fourPillars.year.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_year, "terrestrialBranch",
                                info.calendar.fourPillars.year.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_four_pillars_month, "celestialStem",
                                info.calendar.fourPillars.month.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_month, "terrestrialBranch",
                                info.calendar.fourPillars.month.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_four_pillars_day, "celestialStem",
                                info.calendar.fourPillars.day.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_day, "terrestrialBranch",
                                info.calendar.fourPillars.day.terrestrialBranch);
        cJSON_AddNumberToObject(calendar_four_pillars_hour, "celestialStem",
                                info.calendar.fourPillars.hour.celestialStem);
        cJSON_AddNumberToObject(calendar_four_pillars_hour, "terrestrialBranch",
                                info.calendar.fourPillars.hour.terrestrialBranch);
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

#if AAA

//void generate_data() {
//    int sy = 1601, ey = 6101, num = ey - sy, counter;
//    FILE *file = nullptr;
//    file = fopen("./calendar_data.c", "w");
//    if (file == nullptr) {
//        file = stdout;
//    }
//    fprintf(file, "#include <stdint.h>\n");
//    counter = 0;
//    fprintf(file, "int const month_info[%d] = {\n0x%08X, // %d\n", num + 1, month_info[0], sy - 1);
//    for (int i = 0; i < ey - sy; ++i) {
//        fprintf(file, "0x%08X, ", month_info[i + 1]);
//        if (i % 5 == 4) {
//            fprintf(file, "// %d-%d\n", i - 4 + sy, i + sy);
//        }
//        counter++;
//    }
//    if (counter % 5 > 0) {
//        fprintf(file, "//%d-%d\n", sy + counter - counter % 5, sy + counter - 1);
//    }
//    fprintf(file, "};\n");
//
//    counter = 0;
//    fprintf(file, "unsigned char const solar_terms_index[%d] = {\n", (num + 1) * 3);
//
//    fprintf(file, "0x%02X, 0x%02X, 0x%02X, // %d\n", solar_terms_index[0], solar_terms_index[1], solar_terms_index[2],
//            sy - 1);
//    for (int i = 0; i < num; ++i) {
//        int pos = (i + 1) * 3;
//        const unsigned char *p = solar_terms_index;
//        fprintf(file, "0x%02X, 0x%02X, 0x%02X, ", p[pos + 0], p[pos + 1], p[pos + 2]);
//        if (i % 5 == 4) {
//            fprintf(file, "// %d-%d\n", i - 4 + sy, i + sy);
//        }
//        counter++;
//    }
//    if (counter % 5 > 0) {
//        fprintf(file, "//%d-%d\n", sy + counter - counter % 5, sy + counter - 1);
//    }
//    fprintf(file, "};\n");
//
//
//    counter = 0;
//    fprintf(file, "const uint16_t extremeSeason[%d] = {\n0x%04X, // %d\n", num + 1, extremeSeason[0], sy - 1);
//    for (int i = 0; i < num; ++i) {
//        fprintf(file, "0x%04X, ", extremeSeason[i + 1]);
//        if (i % 10 == 9) {
//            fprintf(file, "// %d-%d\n", i - 4 + sy, i + sy);
//        }
//    }
//    if (counter % 10 > 0) {
//        fprintf(file, "//%d-%d\n", sy + counter - counter % 10, sy + counter - 1);
//    }
//    fprintf(file, "};\n");
//    if (file != nullptr && file != stdout) {
//        fclose(file);
//    }
//}
//void generate_data() {
//    int sy = 1601, ey = 6101, num = ey - sy, counter;
//    FILE *file = nullptr;
//    file = fopen("./calendar_data.c", "w");
//    if (file == nullptr) {
//        file = stdout;
//    }
//    fprintf(file, "#include <stdint.h>\n");
//    counter = 0;
//    fprintf(file, "int const month_info[%d] = {\n0x%08X, // %d\n", num + 1, month_info[0], sy - 1);
//    for (int i = 0; i < ey - sy; ++i) {
//        fprintf(file, "0x%08X, ", month_info[i + 1]);
//        if (i % 5 == 4) {
//            fprintf(file, "// %d-%d\n", i - 4 + sy, i + sy);
//        }
//        counter++;
//    }
//    if (counter % 5 > 0) {
//        fprintf(file, "//%d-%d\n", sy + counter - counter % 5, sy + counter - 1);
//    }
//    fprintf(file, "};\n");
//
//    counter = 0;
//    fprintf(file, "unsigned char const solar_terms_index[%d] = {\n", (num + 1) * 3);
//
//    fprintf(file, "0x%02X, 0x%02X, 0x%02X, // %d\n", solar_terms_index[0], solar_terms_index[1], solar_terms_index[2],
//            sy - 1);
//    for (int i = 0; i < num; ++i) {
//        int pos = (i + 1) * 3;
//        const unsigned char *p = solar_terms_index;
//        fprintf(file, "0x%02X, 0x%02X, 0x%02X, ", p[pos + 0], p[pos + 1], p[pos + 2]);
//        if (i % 5 == 4) {
//            fprintf(file, "// %d-%d\n", i - 4 + sy, i + sy);
//        }
//        counter++;
//    }
//    if (counter % 5 > 0) {
//        fprintf(file, "//%d-%d\n", sy + counter - counter % 5, sy + counter - 1);
//    }
//    fprintf(file, "};\n");
//
//
//    counter = 0;
//    fprintf(file, "const uint16_t extremeSeason[%d] = {\n0x%04X, // %d\n", num + 1, extremeSeason[0], sy - 1);
//    for (int i = 0; i < num; ++i) {
//        fprintf(file, "0x%04X, ", extremeSeason[i + 1]);
//        if (i % 10 == 9) {
//            fprintf(file, "// %d-%d\n", i - 4 + sy, i + sy);
//        }
//    }
//    if (counter % 10 > 0) {
//        fprintf(file, "//%d-%d\n", sy + counter - counter % 10, sy + counter - 1);
//    }
//    fprintf(file, "};\n");
//    if (file != nullptr && file != stdout) {
//        fclose(file);
//    }
//}
#endif
