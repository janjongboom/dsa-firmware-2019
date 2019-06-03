#include "select_app.h"
#if APPLICATION == WIND_TEST

#include "mbed.h"
#include "peripherals.h"

#define ANEMOMETER_SAMPLING_TIME    3000 // 3 seconds

int main() {
    printf("Hello world\n");
    sleep_manager_lock_deep_sleep();

    while (1) {
        LowPowerTimer timer;
        timer.start();
        bool last_state = wind_speed.read();
        int wind_count = 0;
        while (timer.read_ms() <= ANEMOMETER_SAMPLING_TIME) {
            bool curr_state = wind_speed.read();

            // detect falling edge
            if (last_state == 1 && curr_state == 0) {
                wind_count++;
            }

            last_state = curr_state;
        }
        timer.stop();

        float timePassed = static_cast<float>(timer.read_ms()) / 1000.0f;
        float windSpeedValue = static_cast<float>(wind_count) * (2.25f / timePassed) * 1.609f;

        float windDirection = wind_direction.read() * 360.0f;
        uint16_t windDirectionValue = static_cast<uint16_t>(windDirection);
        printf("[speed] %.2f km/h\r\n", windSpeedValue);

        // printf("DAVIS:   [drct] %u deg,  [speed] %.2f km/h\r\n", windDirectionValue, windSpeedValue);
    }
}

#endif // APPLICATION == WIND_TEST
