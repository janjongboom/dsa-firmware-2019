#include "select_app.h"
#if APPLICATION == SENSOR_TEST

#include "mbed.h"
#include "peripherals.h"

void btn1_fall() {
    led1 = !led1;

    printf("Button1 fall\r\n");
}

void btn2_fall() {
    led2 = !led2;

    printf("Button2 fall\r\n");
}

void print_stats() {
    // allocate enough room for every thread's stack statistics
    int cnt = osThreadGetCount();
    mbed_stats_stack_t *stats = (mbed_stats_stack_t*) malloc(cnt * sizeof(mbed_stats_stack_t));

    cnt = mbed_stats_stack_get_each(stats, cnt);
    for (int i = 0; i < cnt; i++) {
        printf("Thread: 0x%lX, Stack size: %lu / %lu\r\n", stats[i].thread_id, stats[i].max_size, stats[i].reserved_size);
    }
    free(stats);

    // Grab the heap statistics
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);
    printf("Heap size: %lu / %lu bytes\r\n", heap_stats.current_size, heap_stats.reserved_size);

    mbed_stats_cpu_t cpu_stats;
    mbed_stats_cpu_get(&cpu_stats);
    printf("Uptime: %llu ", cpu_stats.uptime / 1000);
    printf("Sleep time: %llu ", cpu_stats.sleep_time / 1000);
    printf("Deep Sleep: %llu\n", cpu_stats.deep_sleep_time / 1000);

    printf("\r\n");
}

void print_sensor_data() {
    hts221.enable();
    tsl2572.enable();
    lps22hb.enable();

    float value1, value2;

    hts221.get_temperature(&value1);
    hts221.get_humidity(&value2);
    printf("HTS221:  [temp] %.2f C, [hum]   %.2f%%\r\n", value1, value2);

    value1 = value2 = 0;
    lps22hb.get_temperature(&value1);
    lps22hb.get_pressure(&value2);
    printf("LPS22HB: [temp] %.2f C, [press] %.2f mbar\r\n", value1, value2);

    tsl2572.read_ambient_light(&value1);
    printf("TSL2572: [lght] %.2f lux\r\n", value1);

    printf("GROVE.7: [anlg] %.2f\r\n", grove12_7.read());
    printf("GROVE.8: [anlg] %.2f\r\n", grove12_8.read());

    printf("SX1276:  [rand] %lu\r\n", radio.random());

    printf("DAVIS:   [drct] %d°,  [speed] %.2f km/h\r\n", anemometer.readWindDirection(), anemometer.readWindSpeed());

    printf("\r\n");

    hts221.disable();
    tsl2572.disable();
    lps22hb.disable();
}

void test_blockdevice() {
    printf("Testing block device\n");

    // Initialize the SPI flash device and print the memory layout
    bd.init();
    printf("bd size: %llu\n",         bd.size());
    printf("bd read size: %llu\n",    bd.get_read_size());
    printf("bd program size: %llu\n", bd.get_program_size());
    printf("bd erase size: %llu\n",   bd.get_erase_size());

    int r;

    // Write "Hello DSA!" to the first block
    char *buffer = (char*)malloc(bd.get_erase_size());
    sprintf(buffer, "Hello DSA!");
    r = bd.erase(0, bd.get_erase_size());
    printf("bd erase returned %d\n", r);
    r = bd.program(buffer, 0, bd.get_erase_size());
    printf("bd program returned %d\n", r);

    // clear buffer
    memset(buffer, 0, bd.get_erase_size());

    // Read back what was stored
    r = bd.read(buffer, 0, bd.get_erase_size());
    printf("bd read returned %d: '%s'\n", r, buffer);
}

int main() {
    EventQueue queue;

    test_blockdevice();

    // Add a PullUp to the unused pins
    for (size_t ix = 0; ix < sizeof(unused) / sizeof(unused[0]); ix++) {
        unused[ix].mode(PullUp);
    }

    hts221.init(NULL);
    lps22hb.init(NULL);
    tsl2572.init();

    led1 = LED_ON;
    led2 = LED_ON;

    hts221.enable();
    lps22hb.enable();
    tsl2572.enable();
    anemometer.enable();

    uint8_t id;
    hts221.read_id(&id);
    printf("HTS221  humidity & temperature   ID = 0x%X\r\n", id);
    lps22hb.read_id(&id);
    printf("LPS22HB pressure & temperature   ID = 0x%X\r\n", id);
    tsl2572.read_id(&id);
    printf("TSL2572 light intensity          ID = 0x%X\r\n", id);

    btn1.fall(queue.event(&btn1_fall));
    btn2.fall(queue.event(&btn2_fall));

    queue.call_every(2000, &print_sensor_data);
    queue.call_every(5000, &print_stats);

    queue.dispatch_forever();
}

#endif // APPLICATION == SENSOR_TEST
