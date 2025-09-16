#include <stdio.h>
#include "pico/stdlib.h"
#include "native/bmmpi_native.hpp"
#include "AppConfig.h"


static RingQueue<int> test_queue(0);
// /***
//  * Main
//  * @return
//  */
int main( void )
{
    test_queue = RingQueue<int>(0);
    test_queue.push_back(1);

    return 0;
}