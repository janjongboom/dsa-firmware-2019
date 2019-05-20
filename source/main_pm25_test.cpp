#include "select_app.h"
#if APPLICATION == PM25_TEST

#include "mbed.h"
#include "PMS5003.h"

static EventQueue queue;
static PMS5003 pm25(PA_2, PA_3, NC, &queue);

void pm25_data_callback(pms5003_data_t data) {
    printf("---------------------------------------\n");
    printf("Concentration Units (standard)\n");
    printf("PM 1.0: %u", data.pm10_standard);
    printf("\t\tPM 2.5: %u", data.pm25_standard);
    printf("\t\tPM 10: %u\n", data.pm100_standard);
    printf("---------------------------------------\n");
    printf("Concentration Units (environmental)\n");
    printf("PM 1.0: %u", data.pm10_env);
    printf("\t\tPM 2.5: %u", data.pm25_env);
    printf("\t\tPM 10: %u\n", data.pm100_env);
    printf("---------------------------------------\n");
    printf("Particles > 0.3um / 0.1L air: %u\n", data.particles_03um);
    printf("Particles > 0.5um / 0.1L air: %u\n", data.particles_05um);
    printf("Particles > 1.0um / 0.1L air: %u\n", data.particles_10um);
    printf("Particles > 2.5um / 0.1L air: %u\n", data.particles_25um);
    printf("Particles > 5.0um / 0.1L air: %u\n", data.particles_50um);
    printf("Particles > 10.0 um / 0.1L air: %u\n", data.particles_100um);
    printf("---------------------------------------\n");
}

int main() {
    printf("Hello world\n");

    pm25.enable(queue.event(&pm25_data_callback));

    queue.dispatch_forever();
}

#endif // APPLICATION == PM25_TEST
