/*
# Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
# Copyright (c) 2015 Fabrizio Di Vittorio.
# All rights reserved.

# GNU GPL LICENSE
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; latest version thereof,
# available at: <http://www.gnu.org/licenses/gpl.txt>.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/




#include "fdv.h"




namespace fdv
{

    // static members
    int8_t      DateTime::s_defaultTimezoneHours    = 0;
    uint8_t     DateTime::s_defaultTimezoneMinutes  = 0;
    IPAddress   DateTime::s_defaultNTPServer        = IPAddress(193, 204, 114, 232);    // default is ntp1.inrim.it

    
    // a local copy of defaultNTPServer string is perfomed
    void DateTime::setDefaults(int8_t timezoneHours, uint8_t timezoneMinutes, char const* defaultNTPServer)
    {
        s_defaultNTPServer       = IPAddress(defaultNTPServer);
        s_defaultTimezoneHours   = timezoneHours;
        s_defaultTimezoneMinutes = s_defaultTimezoneMinutes;
        if (s_defaultNTPServer != IPAddress(0, 0, 0, 0))
        {
            // this will force NTP synchronization
            lastMillis() = 0;
        }
    }
    
    
    void DateTime::setCurrentDateTime(DateTime const& dateTime)
    {
        lastDateTime() = dateTime;
        lastMillis()   = millis();
    }


    // 0=sunday...6=saturday
    uint8_t MTD_FLASHMEM DateTime::dayOfWeek() const
    {
        return (date2days(year, month, day) + 6) % 7;
    }    
    
    
    uint16_t MTD_FLASHMEM DateTime::dayOfYear() const
    {
        return date2days(year, month, day) - date2days(year, 1, 1) + 1;
    }


    DateTime& MTD_FLASHMEM DateTime::setUnixDateTime(uint32_t unixTime)
    {
        unixTime      -= SECONDS_FROM_1970_TO_2000;
        seconds        = unixTime % 60;
        unixTime      /= 60;
        minutes        = unixTime % 60;
        unixTime      /= 60;
        hours         = unixTime % 24;
        uint16_t days = unixTime / 24;
        uint8_t leap;
        for (year = 2000; ; ++year)
        {
            leap = year % 4 == 0;
            if (days < 365u + leap)
                break;
            days -= 365 + leap;
        }
        for (month = 1; ; ++month)
        {
            uint8_t daysPerMonth = daysInMonth(month - 1);
            if (leap && month == 2)
                ++daysPerMonth;
            if (days < daysPerMonth)
                break;
            days -= daysPerMonth;
        }
        day = days + 1;    
        return *this;
    }


    uint32_t MTD_FLASHMEM DateTime::getUnixDateTime() const
    {
        uint16_t days = date2days(year, month, day);
        return time2long(days, hours, minutes, seconds) + SECONDS_FROM_1970_TO_2000;
    }


    DateTime& MTD_FLASHMEM DateTime::setNTPDateTime(uint8_t const* datetimeField)
    {
        uint32_t t = 0;
        for (uint8_t i = 0; i < 4; ++i)
            t = t << 8 | datetimeField[i];
        float f = ((long)datetimeField[4] * 256 + datetimeField[5]) / 65535.0; 
        t -= 2208988800UL; 
        t += (timezoneHours * 3600L) + (timezoneMinutes * 60L);
        if (f > 0.4) 
            ++t;
        return setUnixDateTime(t);
    }
    
    
    bool MTD_FLASHMEM DateTime::getFromNTPServer()
    {
        if (s_defaultNTPServer != IPAddress(0, 0, 0, 0))
        {
            SNTPClient sntp(s_defaultNTPServer);
            uint64_t v = 0;
            if (sntp.query(&v))
            {
                setNTPDateTime((uint8_t*)&v);
                return true;
            }
        }
        return false;
    }


    // if this is the first time calling now() or after 6 hours an NTP update is perfomed
    // warn: now() should be called within 50 days from the last call
    // warn: loses about 3 seconds every 10 hours
    DateTime MTD_FLASHMEM DateTime::now()
    {
        uint32_t currentMillis = millis();
        uint32_t locLastMillis = lastMillis();
        uint32_t diff = (currentMillis < locLastMillis) ? (0xFFFFFFFF - locLastMillis + currentMillis) : (currentMillis - locLastMillis);
        
        if (locLastMillis == 0 || diff > 6 * 3600 * 1000)
        {
            if (lastDateTime().getFromNTPServer())
            {
                lastMillis() = currentMillis;
                return lastDateTime();
            }
        }
        
        DateTime result;
        result.setUnixDateTime( lastDateTime().getUnixDateTime() + (diff / 1000) );
        
        if (s_defaultNTPServer == IPAddress(0, 0, 0, 0))
        {
            // NTP synchronizatin is disabled. Take care for millis overflow.
            if (diff > 10 * 24 * 3600 * 1000)   // past 10 days?
            {
                // reset millis counter to avoid overflow (actually it overflows after 50 days)
                lastMillis()   = currentMillis;
                lastDateTime() = result;
            }
        }
        
        return result;
    }


    DateTime& MTD_FLASHMEM DateTime::lastDateTime()
    {
        static DateTime s_lastDateTime;
        return s_lastDateTime;
    }


    uint32_t& MTD_FLASHMEM DateTime::lastMillis()
    {
        static uint32_t s_lastMillis = 0;
        return s_lastMillis;
    }
    
    
    // inbuf can stay only in RAM
    // formatstr can stay in RAM or Flash
    // warn: doesn't check correctness and size of input string!
    // Format:
    //    '%d' : Day of the month, 2 digits with leading zeros (01..31)
    //    '%m' : Numeric representation of a month, with leading zeros (01..12)
    //    '%Y' : A full numeric representation of a year, 4 digits (1999, 2000...)
    //    '%H' : 24-hour format of an hour with leading zeros (00..23)
    //    '%M' : Minutes with leading zeros (00..59)
    //    '%S' : Seconds, with leading zeros (00..59)
    // Returns position of the next character to process
    char const* MTD_FLASHMEM DateTime::decode(char const* inbuf, char const* formatstr)
    {
        for (; inbuf && getChar(formatstr); ++formatstr)
        {
            if (getChar(formatstr) == '%')
            {
                ++formatstr;
                switch (getChar(formatstr))
                {
                    case '%':
                        ++inbuf;
                        break;
                    case 'd':
                        day =  (*inbuf++ - '0') * 10;
                        day += (*inbuf++ - '0');
                        break;
                    case 'm':
                        month =  (*inbuf++ - '0') * 10;
                        month += (*inbuf++ - '0');
                        break;
                    case 'Y':
                        year =  (*inbuf++ - '0') * 1000;
                        year += (*inbuf++ - '0') * 100;
                        year += (*inbuf++ - '0') * 10;
                        year += (*inbuf++ - '0');
                        break;
                    case 'H':
                        hours =  (*inbuf++ - '0') * 10;
                        hours += (*inbuf++ - '0');
                        break;
                    case 'M':
                        minutes =  (*inbuf++ - '0') * 10;
                        minutes += (*inbuf++ - '0');
                        break;
                    case 'S':
                        seconds =  (*inbuf++ - '0') * 10;
                        seconds += (*inbuf++ - '0');
                        break;
                    default:
                        return inbuf;   // error!
                }
            }
            else
                ++inbuf;
        }
        return inbuf;
    }
    
    
    // Format:
    //    '%a' : Weekday abbreviated name (Sun, Mon, ..., Sat)
    //    '%A' : Weekday full name (Sunday, Monday, ..., Saturday)
    //    '%w' : Numeric representation of the day of the week (0 = Sunday, 6 = Saturday)
    //    '%d' : Day of the month, 2 digits with leading zeros (01..31)
    //    '%D' : Day of the month without leading zeros (1..31)
    //    '%b' : Month abbreviated name (Jan, Feb, ..., Dec)
    //    '%B' : Month full name (January, February, ..., December)
    //    '%m' : Numeric representation of a month, with leading zeros (01..12)
    //    '%y' : Two digits year (99, 00...)
    //    '%Y' : A full numeric representation of a year, 4 digits (1999, 2000...)
    //    '%H' : 24-hour format of an hour with leading zeros (00..23)
    //    '%I' : Hour (12-hour clock) as a zero-padded decimal number (01, 02, ..., 12)
    //    '%p' : AM or PM
    //    '%M' : Minutes with leading zeros (00..59)
    //    '%S' : Seconds, with leading zeros (00..59)
    //    '%z' : UTC offset in the form +HHMM or -HHMM (+0000, -0400, +1030, ...)
    //    '%j' : Day of the year as a zero-padded decimal number (001, 002, ..., 366)    
    //    '%c' : Date and time representation (Tue Aug 16 2015 21:30:00 +0000)
    // Returns resulting string length without trailing zero.
    // Example:
    //   char str[50];
    //   datetime.format(str, "%a %b %d %Y %H:%M:%S %z");  // equivalent to "%c"
    uint16_t MTD_FLASHMEM DateTime::format(char* outbuf, char const* formatstr)
    {
        static char const* DAYS[]   = {FSTR("Sunday"), FSTR("Monday"), FSTR("Tuesday"), FSTR("Wednesday"), FSTR("Thursday"), FSTR("Friday"), FSTR("Saturday")};
        static char const* MONTHS[] = {FSTR("January"), FSTR("February"), FSTR("March"), FSTR("April"), FSTR("May"), FSTR("June"), FSTR("July"), FSTR("August"), FSTR("September"), FSTR("October"), FSTR("November"), FSTR("December")};
        
        char* outbuf_start = outbuf;
        
        for (; getChar(formatstr); ++formatstr)
        {
            if (getChar(formatstr) == '%')
            {
                ++formatstr;
                switch (getChar(formatstr))
                {
                    case '%':
                        *outbuf++ = '%';
                        break;
                    case 'a':
                        outbuf += sprintf(outbuf, FSTR("%.3s"), DAYS[dayOfWeek()]);
                        break;
                    case 'A':
                        outbuf += sprintf(outbuf, FSTR("%s"), DAYS[dayOfWeek()]);
                        break;
                    case 'w':
                        outbuf += sprintf(outbuf, FSTR("%d"), dayOfWeek());
                        break;
                    case 'd':
                        outbuf += sprintf(outbuf, FSTR("%02d"), day);
                        break;
                    case 'D':
                        outbuf += sprintf(outbuf, FSTR("%d"), day);
                        break;
                    case 'b':
                        outbuf += sprintf(outbuf, FSTR("%.3s"), MONTHS[month - 1]);
                        break;
                    case 'B':
                        outbuf += sprintf(outbuf, FSTR("%s"), MONTHS[month - 1]);
                        break;
                    case 'm':
                        outbuf += sprintf(outbuf, FSTR("%02d"), month);
                        break;
                    case 'y':
                        outbuf += sprintf(outbuf, FSTR("%02d"), year % 100);
                        break;
                    case 'Y':
                        outbuf += sprintf(outbuf, FSTR("%d"), year);
                        break;
                    case 'H':
                        outbuf += sprintf(outbuf, FSTR("%02d"), hours);
                        break;
                    case 'I':
                        outbuf += sprintf(outbuf, FSTR("%02d"), (hours == 0 || hours == 12)? 12 : (hours % 12));
                        break;
                    case 'p':
                        outbuf += sprintf(outbuf, FSTR("%s"), hours > 11? FSTR("PM") : FSTR("AM"));
                        break;
                    case 'M':
                        outbuf += sprintf(outbuf, FSTR("%02d"), minutes);
                        break;
                    case 'S':
                        outbuf += sprintf(outbuf, FSTR("%02d"), seconds);
                        break;
                    case 'z':
                        outbuf += sprintf(outbuf, FSTR("%+03d%02d"), timezoneHours, timezoneMinutes);
                        break;
                    case 'j':
                        outbuf += sprintf(outbuf, FSTR("%03d"), dayOfYear());
                        break;
                    case 'c':
                        outbuf += format(outbuf, FSTR("%a %b %d %Y %H:%M:%S %z"));
                        break;
                }
            }
            else
                *outbuf++ = getChar(formatstr);
        }
        *outbuf = 0;
        return outbuf - outbuf_start;
    }


    uint8_t MTD_FLASHMEM DateTime::daysInMonth(uint8_t month)
    {
        static uint8_t const DIMO[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        return DIMO[month];
    }


    long MTD_FLASHMEM DateTime::time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
    {
        return ((days * 24L + h) * 60 + m) * 60 + s;
    }  


    uint16_t MTD_FLASHMEM DateTime::date2days(uint16_t y, uint8_t m, uint8_t d)
    {
        if (y >= 2000)
            y -= 2000;
        uint16_t days = d;
        for (uint8_t i = 1; i < m; ++i)
            days += daysInMonth(i - 1);
        if (m > 2 && y % 4 == 0)
            ++days;
        return days + 365 * y + (y + 3) / 4 - 1;
    }





} // end of "fdv" namespace

