#include "Arduino.h"
#include <TZ.h>

#define LTZ 
#define OFFSET 441763200        //Difference CANOPEN Timestampe - Epoche
#define ONE_DAY 60 * 60 * 24

struct timestamp{
    uint32_t ms_since_mn;
    uint16_t days;

};


bool SetupTimefromNTP();
timestamp co_time();
void sendCOTimestamp();

