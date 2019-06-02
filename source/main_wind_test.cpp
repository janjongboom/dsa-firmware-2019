#include "select_app.h"
#if APPLICATION == WIND_TEST

#include "mbed.h"
#include "peripherals.h"

Thread realtimeThread(osPriorityRealtime);
EventQueue queue;
static InterruptIn wind(PD_4);

void fall() {
    anemometer.handleIrq();
}

int main() {
    printf("Hello world\n");

    anemometer.enable();
    anemometer.readWindSpeed();

    wind.fall(queue.event(&fall));

    realtimeThread.start(callback(&queue, &EventQueue::dispatch_forever));

    while (1) {
        printf("Speed: %f\n", anemometer.readWindSpeed());
        wait_ms(3000);
    }
}

#endif // APPLICATION == WIND_TEST
