#include "FreeRTOS.h"
#include "../include/FreeRTOSConfig.h"


void vAssertCalled( const char * pcFile, 
                    unsigned long ulLine )
{
volatile unsigned long ul = 0;
    portENTER_CRITICAL();
    {
        while( ul == 0 )
        {
            portNOP();
        }
    }
    portEXIT_CRITICAL();
}
