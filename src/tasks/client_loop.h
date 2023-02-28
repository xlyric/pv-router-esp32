#ifndef TASK_CLIENT_LOOP
#define TASK_CLIENT_LOOP

#include "functions/WifiFunctions.h"

void client_loop(void * parameter)
{
  for(;;){
    client.loop();
    // logging.start += "client.loop\r\n";
    vTaskDelay(1500 / portTICK_PERIOD_MS);
  }
}
#endif