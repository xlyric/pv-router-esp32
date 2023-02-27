#include "functions/WifiFunctions.h"

void client_loop(void * parameter)
{
  for(;;){
    client.loop();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}