#include "calendar.h"
#include "calendar_data.h"

int day_ordinal_common[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
int day_ordinal_leap[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

bool check_data(int year, int month, int day, int hour, int minute, int second) {
    if (hour < 0 || hour > 23) {
        return false;
    }
    if (minute < 0 || minute > 59) {
        return false;
    }
    if (second < 0 || second > 59) {
        return false;
    }
    if (year < start_year || year >= end_year) {
        return false;
    }
    if (month < 1 || month > 12) {
        return false;
    }
    if (day < 1 || day > 31) {
        return false;
    } else {
        if ((month == 4 || month == 6 || month == 9 || month == 11) && day == 31) {
            return false;
        } else if (month == 2) {
            int max_day = is_leap_year(year) ? 29 : 28;
            if (day > max_day) {
                return false;
            }
        }
    }
    return true;
}

int get_day_ordinal(int year, int month, int day) {
    int _ordinal;
    if (is_leap_year(year)) {//判断是否为闰年
        _ordinal = (day_ordinal_leap[month - 1] + day - 1);//元旦为序数0, 因此减1
    } else {
        _ordinal = (day_ordinal_common[month - 1] + day - 1);
    }
    return _ordinal;
}

int get_date_from_ordinal(int year, int ordinal, int *month, int *day) {
    int *_day_ordinal = is_leap_year(year) ? day_ordinal_leap : day_ordinal_common;
    *month = *day = 0;
    for (int i = 0; i < 12; ++i) {
        if (_day_ordinal[i] <= ordinal && ordinal < _day_ordinal[i + 1]) {//找出月份
            *month = i + 1;
            *day = ordinal - _day_ordinal[i] + 1;//计算出“日”
            break;
        }
    }
    return *month << 8 | *day;
}

int get_days_of_month(int year, int month) {
    int days = -1;
    if (month == 1 || month == 12) {
        days = 31;
    } else if (month == 2) {
        days = is_leap_year(year) ? 29 : 28;
    } else if (3 <= month && month <= 11) {
        days = day_ordinal_common[month] - day_ordinal_common[month - 1];
    }
    return days;
}

int get_day_of_week(int year, int month, int day) {
    int dayOrdinal = get_day_ordinal(year, month, day);
    dayOrdinal++;//一年中的第几天, 因为 get_day_ordinal 所得到的是索引, 因此要加1
    year -= 1;
    return (year + year / 4 - year / 100 + year / 400 + dayOrdinal) % 7;//这个只是算星期的通用公式
}

uint32_t get_month_info(int lunar_year) {
    return month_info[lunar_year - start_year + 1] & 0x7FFFFFFF;
}

uint8_t get_lunar_leap_month(int lunar_year) {
    return (get_month_info(lunar_year) >> 13) & 0x0F;
}

uint8_t get_lunar_new_year_ordinal(int lunar_year) {
    return (get_month_info(lunar_year) >> 17) & 0x3F;
}

uint8_t get_solar_new_year_solar_terms(int lunar_year) {
    return (get_month_info(lunar_year) >> 27) & 0x0F;
}

uint8_t get_solar_new_year_terrestrial_branch(int lunar_year) {
    return (get_month_info(lunar_year) >> 23) & 0x0F;
}

int get_lunar_days_of_month(int lunar_year, int lunar_month, bool leap_month) {
    uint16_t dx_data;//该年大小月情况
    uint8_t accurate_leap_month; // 正确的农历闰月

    dx_data = get_month_info(lunar_year) & 0x1FFF;//整年大小月情况
    accurate_leap_month = get_lunar_leap_month(lunar_year);//获取真实闰月月份

    if (leap_month) {//指定查询的当前月是闰月
        if (accurate_leap_month != lunar_month) {
            return 0;//不存在的闰月
        }
        dx_data >>= lunar_month;//闰月要多找一位
    } else {
        if (accurate_leap_month > 0) {//存在闰月
            if (lunar_month <= accurate_leap_month) {
                dx_data >>= (lunar_month - 1);//月份在闰月之前, 和无闰月一样
            } else {
                dx_data >>= lunar_month;//月份在闰月之后, 倒找需要多找一位
            }
        } else {
            dx_data >>= (lunar_month - 1);//无闰月
        }
    }
    return (dx_data & 0b1) ? 30 : 29;
}

bool lunar_expand_dx(int lunar_year, int days_of_month[15]) {
    int pos = 0; // 数组写入位置
    uint8_t leap_month; // 闰月
    for (int i = 0; i < 15; ++i) {
        days_of_month[i] = 0;
    }
    // 获取前一年农历冬月和腊月的天数
    leap_month = get_lunar_leap_month(lunar_year - 1); // 取得前一个农历年的闰月情况

    if (leap_month < 11) { // 闰月在前一年的一月至十月或者无闰月
        days_of_month[pos] = get_lunar_days_of_month(lunar_year - 1, 11, false);//取上一年十一月天数
        pos++;
        days_of_month[pos] = get_lunar_days_of_month(lunar_year - 1, 12, false);//取上一年十二月天数
        pos++;
    } else {
        days_of_month[pos] = get_lunar_days_of_month(lunar_year - 1, 11, false);//取上一年十一月的天数
        pos++;
        if (leap_month == 11) { // 闰十一月
            days_of_month[pos] = get_lunar_days_of_month(lunar_year - 1, 11, true);//取上一年闰十一月的天数
            pos++;
            days_of_month[pos] = get_lunar_days_of_month(lunar_year - 1, 12, false);//取上一年十二月天数
            pos++;
        } else if (leap_month == 12) { // 闰十二月
            days_of_month[pos] = get_lunar_days_of_month(lunar_year - 1, 12, false);//取上一年十二月天数
            pos++;
            days_of_month[pos] = get_lunar_days_of_month(lunar_year - 1, 12, true);//取上一年闰十二月天数
            pos++;
        }
    }

    // 获取今年农历月份的天数
    leap_month = get_lunar_leap_month(lunar_year); // 获取当前农历年的闰月情况
    if (leap_month == 0) { // 无闰月
        for (int i = 0; i < 12; ++i) {
            days_of_month[pos] = get_lunar_days_of_month(lunar_year, i + 1, false); // 取每个农历月天数
            pos++;
        }
    } else {
        for (int i = 0; i < 12; ++i) {
            if (i == leap_month) {
                days_of_month[pos] = get_lunar_days_of_month(lunar_year, i, true); // 取闰月的天数
                pos++;
            }
            days_of_month[pos] = get_lunar_days_of_month(lunar_year, i + 1, false);// 取非闰月的天数
            pos++;
        }
    }
    return true;
}

bool get_jieqi_data(int year, int month, int jieqi_date[2], int jieqi_ordinal[2]) {
    int index = (year - start_year) * 3; // 对应每公历年首个节气所在字节的索引
    int xiaohan_order;//小寒年内序数
    int jq_data;//节气数据
    int current_jq_data;//当前计算的节气数据
    int days;//当前节气距离该年小寒的天数
    int remain;
    jq_data = solar_terms_source[solar_terms_index[index]];
    xiaohan_order = (jq_data >> 14) + 3;//加上3，转为小寒的年内序数
    current_jq_data = (jq_data >> 12) & 0x03;//当前计算的节气与上一个节气的天数差信息
    if (month == 1) {
        jieqi_ordinal[0] = xiaohan_order;//第一个节气的年内序数
        jieqi_date[0] = xiaohan_order + 1;//加1转到日期

        jieqi_ordinal[1] = current_jq_data + 14 + xiaohan_order;//第二个节气的年内序数
        jieqi_date[1] = current_jq_data + 14 + xiaohan_order + 1;//大寒：小寒的年内序数加上距离小寒的天数
    } else {
        days = current_jq_data + 14;//距离小寒的天数，当前为大寒距离小寒的天数
        days += xiaohan_order;//加上小寒，转为年内序数
        for (int i = 1; i < month; i++) {
            remain = i % 4;
            current_jq_data = (jq_data >> (18 - ((remain + 1) << 2))) & 0x03;
            days += current_jq_data + 14;
            current_jq_data = (jq_data >> (16 - ((remain + 1) << 2))) & 0x03;
            days += current_jq_data + 14;
            if (remain == 3) {
                jq_data = solar_terms_source[solar_terms_index[index + (i + 1) / 4]];
            }
        }
        jieqi_ordinal[0] = days - current_jq_data - 14;//第一个节气的年内序数
        jieqi_ordinal[1] = days;//第二个节气的年内序数

        get_date_from_ordinal(year, jieqi_ordinal[0], &month, jieqi_date);//month中的第一个节气
        get_date_from_ordinal(year, jieqi_ordinal[1], &month, jieqi_date + 1);//month中的第二个节气
    }
    return true;
}

Calendar get_calendar_info_no_check(int year, int month, int day, int hour, int minute, int second) {
    Calendar calendar = {0};
    int current_ordinal, lunar_new_year, lichun_order, days_since_lunar_new_year;//年内序数, 立春的年内序数, 冬至的年内序数，农历新年的公历年内序数, 剩余天数
    int days_of_lunar_month[15] = {0};//存放农历月份天数
    int pre_leap_month, leap_month = 0;//农历上一年闰月, 今年闰月
    int jieqi_date[2] = {0}, jieqi_ordinal[2] = {0};
    int four_pillars_year, four_pillars_month, index;//以立春为正月初一的月份, 循环变量
    // 时间信息
    calendar.time.hour = hour;
    calendar.time.minute = minute;
    calendar.time.second = second;

    // 公历信息
    calendar.solar.year = year;
    calendar.solar.month = month;
    calendar.solar.day = day;
    calendar.solar.week = get_day_of_week(year, month, day);
    calendar.solar.daysOfMonth = get_days_of_month(year, month);
    calendar.solar.daysOfYear = is_leap_year(year) ? 366 : 365;
    calendar.solar.dayOfYear = get_day_ordinal(year, month, day) + 1;

    //获取立春和冬至的年内序数
    get_jieqi_data(year, 2, jieqi_date, jieqi_ordinal);
    lichun_order = jieqi_ordinal[0];

    //节气信息
    // ---1---jieqi_date[0]---2---jieqi_date[1]---3---
    index = (2 * month + 21) % 24; // 将月份转为序号
    // 获取节气本月节气的日期和年内序数
    get_jieqi_data(calendar.solar.year, calendar.solar.month, jieqi_date, jieqi_ordinal);
    if (calendar.solar.day < jieqi_date[0]) { // 1的位置
        calendar.solarTerms.solarTerm = (index + 22) % 24 + 1; // 上一个
    } else if (calendar.solar.day >= jieqi_date[1]) { // 3的位置
        calendar.solarTerms.solarTerm = index + 1; // 下一个
    } else { // 2的位置
        calendar.solarTerms.solarTerm = index; // 当前
    }
    // 下一个节气
    calendar.solarTerms.nextSolarTerm = (calendar.solarTerms.solarTerm % 24) + 1;
    if (calendar.solar.day == jieqi_date[0] || calendar.solar.day == jieqi_date[1]) {
        calendar.solarTerms.isToday = true;
    }
    // 计算剩余天数
    if (calendar.solar.day < jieqi_date[0]) { // 1的位置
        calendar.solarTerms.nextSolarTermRemainDays = jieqi_date[0] - calendar.solar.day;
    } else if (jieqi_date[0] <= calendar.solar.day && calendar.solar.day < jieqi_date[1]) {
        calendar.solarTerms.nextSolarTermRemainDays = jieqi_date[1] - calendar.solar.day;
    } else {
        current_ordinal = calendar.solar.dayOfYear - 1;
        month = ((calendar.solarTerms.nextSolarTerm + 1) / 2) % 12 + 1;//将下一个节气的序号转换到月份
        if (month == 1) {
            if (calendar.solar.year + 1 < end_year) {
                get_jieqi_data(calendar.solar.year + 1, month, jieqi_date, jieqi_ordinal);
                calendar.solarTerms.nextSolarTermRemainDays = 31 - day + jieqi_date[0];
            } else if (calendar.solar.year + 1 == end_year) {
                calendar.solarTerms.nextSolarTermRemainDays =
                        calendar.solar.daysOfYear - current_ordinal + 4;
            }
        } else {
            get_jieqi_data(calendar.solar.year, month, jieqi_date, jieqi_ordinal);
            calendar.solarTerms.nextSolarTermRemainDays = jieqi_ordinal[0] - current_ordinal;
        }
    }

    //农历信息
    calendar.lunar.chineseCalendarYear = calendar.solar.year;
    calendar.lunar.isChineseCalendarLeapMonth = false;//默认不是闰月
    current_ordinal = calendar.solar.dayOfYear - 1;//获取日期的年内序数
    lunar_new_year = get_lunar_new_year_ordinal(calendar.lunar.chineseCalendarYear);//获取春节的年内序数
    days_since_lunar_new_year = current_ordinal - lunar_new_year;//距离农历新年的天数
    //获取月份天数, 数组从上一年十一月开始到今年(闰)十二月, 包含闰月
    lunar_expand_dx(calendar.lunar.chineseCalendarYear, days_of_lunar_month);
    pre_leap_month = get_lunar_leap_month(calendar.lunar.chineseCalendarYear - 1);//获取上一年的闰月
    if (pre_leap_month == 0) {//上一年没有闰月
        leap_month = get_lunar_leap_month(calendar.lunar.chineseCalendarYear);//查询今年的闰月
    }
    index = pre_leap_month > 10 ? 3 : 2;//上一年十一月或十二月有闰月时, 今年正月在 days_of_lunar_month 中的索引为3, 否则为2
    if (current_ordinal < lunar_new_year) {//年内序数早于农历新年
        calendar.lunar.chineseCalendarYear -= 1;//农历年减1
        while (days_since_lunar_new_year < 0) {
            index--;//第一次先减去是因为当前i是正月, 减1表示上一年十二月(或闰十二月)
            days_since_lunar_new_year += days_of_lunar_month[index];//加上上一年十二月、十一月的总天数(含闰月)直到日数大于0
        }
        calendar.lunar.isDXYue = days_of_lunar_month[index] == 30;
        if (pre_leap_month < 11) {//如果上一年十一月或十二月不存在闰月
            calendar.lunar.chineseCalendarMonth = 11 + index;
            calendar.lunar.isChineseCalendarLeapMonth = false;
        } else if (pre_leap_month == 11) {//闰十一月
            if (index == 0) {//十一月(即在闰月之前)
                calendar.lunar.chineseCalendarMonth = 11 + index;//转换到月份
                calendar.lunar.isChineseCalendarLeapMonth = false;
            } else {
                calendar.lunar.chineseCalendarMonth = 10 + index;//转换到月份
                calendar.lunar.isChineseCalendarLeapMonth =
                        calendar.lunar.chineseCalendarMonth == pre_leap_month;
            }
        } else if (pre_leap_month == 12) {//闰十二月
            if (index < 2) {//十一月(即在闰月之前)
                calendar.lunar.chineseCalendarMonth = 11 + index;//转换到月份
                calendar.lunar.isChineseCalendarLeapMonth = false;
            } else {
                calendar.lunar.chineseCalendarMonth = 10 + index;//转换到月份
                calendar.lunar.isChineseCalendarLeapMonth =
                        calendar.lunar.chineseCalendarMonth == pre_leap_month;
            }
        }
        calendar.lunar.chineseCalendarDay = days_since_lunar_new_year;
    } else {
        while (days_since_lunar_new_year >= days_of_lunar_month[index]) {
            days_since_lunar_new_year -= days_of_lunar_month[index];//寻找农历月
            index++;//移至下个月
        }
        if (pre_leap_month > 10) {
            calendar.lunar.chineseCalendarMonth = index - 2;
            calendar.lunar.isChineseCalendarLeapMonth = false;
        } else {
            if (0 < leap_month && leap_month <= index - 2) {
                calendar.lunar.chineseCalendarMonth = index - 2;
                calendar.lunar.isChineseCalendarLeapMonth =
                        calendar.lunar.chineseCalendarMonth == leap_month;
            } else {
                calendar.lunar.chineseCalendarMonth = index - 1;
                calendar.lunar.isChineseCalendarLeapMonth = false;
            }
        }
        calendar.lunar.chineseCalendarDay = days_since_lunar_new_year;
    }
    calendar.lunar.chineseCalendarDay += 1;//索引转换到数量
    calendar.lunar.isDXYue = days_of_lunar_month[index] == 30;//days_of_lunar_month[index]为当前农历月的天数
    calendar.lunar.celestialStem = (calendar.lunar.chineseCalendarYear - 4) % 10 + 1; // 天干
    calendar.lunar.terrestrialBranch = (calendar.lunar.chineseCalendarYear - 4) % 12 + 1; // 地支
    calendar.lunar.chineseZodiac = calendar.lunar.terrestrialBranch; // 生肖

    //四柱-年干支
    four_pillars_year = calendar.lunar.chineseCalendarYear;
    if (calendar.lunar.chineseCalendarYear == calendar.solar.year && current_ordinal < lichun_order) {
        four_pillars_year -= 1;
    }
    calendar.fourPillars.year.celestialStem = (four_pillars_year - 4) % 10 + 1;
    calendar.fourPillars.year.terrestrialBranch = (four_pillars_year - 4) % 12 + 1;

    //四柱-月干支
    //将节气的序号转换到以立春为正月初一的月份
    four_pillars_month = calendar.solarTerms.solarTerm / 2 + calendar.solarTerms.solarTerm % 2;
    switch (calendar.fourPillars.year.celestialStem) {
        case 1:
        case 6:
            calendar.fourPillars.month.celestialStem = (2 + four_pillars_month - 1) % 10 + 1;
            calendar.fourPillars.month.terrestrialBranch = (2 + four_pillars_month - 1) % 12 + 1;
            break;
        case 2:
        case 7:
            calendar.fourPillars.month.celestialStem = (4 + four_pillars_month - 1) % 10 + 1;
            calendar.fourPillars.month.terrestrialBranch = (2 + four_pillars_month - 1) % 12 + 1;
            break;
        case 3:
        case 8:
            calendar.fourPillars.month.celestialStem = (6 + four_pillars_month - 1) % 10 + 1;
            calendar.fourPillars.month.terrestrialBranch = (2 + four_pillars_month - 1) % 12 + 1;
            break;
        case 4:
        case 9:
            calendar.fourPillars.month.celestialStem = (8 + four_pillars_month - 1) % 10 + 1;
            calendar.fourPillars.month.terrestrialBranch = (2 + four_pillars_month - 1) % 12 + 1;
            break;
        case 5:
        case 10:
            calendar.fourPillars.month.celestialStem = (0 + four_pillars_month - 1) % 10 + 1;
            calendar.fourPillars.month.terrestrialBranch = (2 + four_pillars_month - 1) % 12 + 1;
            break;
        default:
            calendar.fourPillars.month.celestialStem = calendar.fourPillars.month.terrestrialBranch = -1;
    }

    //四柱-日干支
    calendar.fourPillars.day.celestialStem = get_solar_new_year_solar_terms(year);
    calendar.fourPillars.day.terrestrialBranch = get_solar_new_year_terrestrial_branch(year);
    switch (calendar.solar.month) {
        case 2:
        case 6:
            calendar.fourPillars.day.celestialStem += 1;
            calendar.fourPillars.day.terrestrialBranch += 7;
            break;
        case 3:
            calendar.fourPillars.day.celestialStem -= 1;
            calendar.fourPillars.day.terrestrialBranch -= 1;
            break;
        case 4:
            calendar.fourPillars.day.celestialStem += 0;
            calendar.fourPillars.day.terrestrialBranch += 6;
            break;
        case 5:
            calendar.fourPillars.day.celestialStem += 0;
            calendar.fourPillars.day.terrestrialBranch += 0;
            break;
        case 7:
            calendar.fourPillars.day.celestialStem += 1;
            calendar.fourPillars.day.terrestrialBranch += 1;
            break;
        case 8:
            calendar.fourPillars.day.celestialStem += 2;
            calendar.fourPillars.day.terrestrialBranch += 8;
            break;
        case 9:
            calendar.fourPillars.day.celestialStem += 3;
            calendar.fourPillars.day.terrestrialBranch += 3;
            break;
        case 10:
            calendar.fourPillars.day.celestialStem += 3;
            calendar.fourPillars.day.terrestrialBranch += 9;
            break;
        case 11:
            calendar.fourPillars.day.celestialStem += 4;
            calendar.fourPillars.day.terrestrialBranch += 4;
            break;
        case 12:
            calendar.fourPillars.day.celestialStem += 4;
            calendar.fourPillars.day.terrestrialBranch += 10;
            break;
        default:
            break;
    }
    if (calendar.solar.month > 2 && is_leap_year(calendar.solar.year)) {
        calendar.fourPillars.day.celestialStem += 1;
        calendar.fourPillars.day.terrestrialBranch += 1;
    }
    if (calendar.time.hour == 23 || calendar.time.hour == 0) {
        calendar.fourPillars.day.celestialStem += 1;
        calendar.fourPillars.day.terrestrialBranch += 1;
    }
    calendar.fourPillars.day.celestialStem =
            (calendar.fourPillars.day.celestialStem + day - 1) % 10 + 1;
    calendar.fourPillars.day.terrestrialBranch =
            (calendar.fourPillars.day.terrestrialBranch + day - 1) % 12 + 1;

    //四柱-时干支
    switch (calendar.fourPillars.day.celestialStem) {
        case 1:
        case 6:
            calendar.fourPillars.hour.celestialStem = (0 + (calendar.time.hour + 1) / 2) % 10 + 1;
            break;
        case 2:
        case 7:
            calendar.fourPillars.hour.celestialStem = (2 + (calendar.time.hour + 1) / 2) % 10 + 1;
            break;
        case 3:
        case 8:
            calendar.fourPillars.hour.celestialStem = (4 + (calendar.time.hour + 1) / 2) % 10 + 1;
            break;
        case 4:
        case 9:
            calendar.fourPillars.hour.celestialStem = (6 + (calendar.time.hour + 1) / 2) % 10 + 1;
            break;
        case 5:
        case 10:
            calendar.fourPillars.hour.celestialStem = (8 + (calendar.time.hour + 1) / 2) % 10 + 1;
            break;
        default:
            calendar.fourPillars.hour.celestialStem = calendar.fourPillars.hour.terrestrialBranch = -1;
    }
    calendar.fourPillars.hour.terrestrialBranch = ((calendar.time.hour + 1) / 2) % 12 + 1;
    return calendar;
}

ChineseCalendarInfo get_calendar_info(int year, int month, int day, int hour, int minute, int second) {
    ChineseCalendarInfo calendarInfo = {SUCCESS};
    if (check_data(year, month, day, hour, minute, second)) {
        calendarInfo.calendar = get_calendar_info_no_check(year, month, day, hour, minute, second);
    } else {
        calendarInfo.code = ERROR;
    }
    uint64_t seed = ((((year * 100ll + month) * 100ll + day) * 100ll + hour) * 100ll + minute) * 100ll + second;
    uint64_t hash = default_murmurhash3_64(&calendarInfo.calendar, sizeof(Calendar), seed);
    int counter = 15;
    while (hash) {
        uint64_t tmp = hash & 0x0f;
        if (tmp < 10) {
            calendarInfo.hash[counter] = (char) (tmp + '0');
        } else {
            calendarInfo.hash[counter] = (char) ((tmp - 10) + 'A');
        }
        hash >>= 4;
        counter--;
    }
    return calendarInfo;
}