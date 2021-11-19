#include "co_time.h"
#include "can.h"
#include "canopen.h"


time_t now;

bool SetupTimefromNTP()
{

    configTime(4 * 3600,0, "pool.ntp.org", "time.nist.gov");

    int max_try = 0;

    time(&now);
    while (now < 8 * 3600 * 2 && max_try < 4)
    {
        delay(500);

        time(&now);
        max_try++;
    }

if (max_try >= 4)
    return false;
else
    return true;
}

timestamp co_time()
{

    timestamp cotime;

    time(&now);

    unsigned long timediff = now - OFFSET;

    div_t co_timestamp;
    co_timestamp = div(timediff + 3600, ONE_DAY);

    cotime.days = co_timestamp.quot;
    cotime.ms_since_mn = co_timestamp.rem * 1000;

    return cotime;
}

void sendCOTimestamp()
{

    can_frame timeframe;
    timestamp curTime = co_time();
    timeframe.id = TIME_STAMP;
    timeframe.dlc = 6;

    for (int i = 0; i < 6; i++){
            timeframe.data[i] = *((char*)(&curTime) + i); 
    }
    
    can_send_frame(timeframe);

    
    
    
}