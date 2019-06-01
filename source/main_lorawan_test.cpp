#include "select_app.h"
#if APPLICATION == LORAWAN_TEST

#include "mbed.h"
#include "mbed_trace.h"
#include "mbed_events.h"
#include "LoRaWANInterface.h"
#include "peripherals.h"
#include "CayenneLPP.h"

#define HAS_ANEMOMETER      1
#define HAS_PM25            1

static uint8_t DEV_EUI[8] = { 0x00 };
static uint8_t APP_EUI[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0xEE, 0xBB };
static uint8_t APP_KEY[] = { 0x7C, 0x85, 0x17, 0xDB, 0x19, 0x2B, 0xD2, 0x14, 0xE1, 0x16, 0xB9, 0x78, 0x46, 0x4D, 0xC1, 0xBA };

// The port we're sending and receiving on
#define MBED_CONF_LORA_APP_PORT     15

// EventQueue is required to dispatch events around
static EventQueue ev_queue;

// Constructing Mbed LoRaWANInterface and passing it down the radio object.
static LoRaWANInterface lorawan(radio);

// Application specific callbacks
static lorawan_app_callbacks_t callbacks;

// LoRaWAN stack event handler
static void lora_event_handler(lorawan_event_t event);

// Connecting LED... blink when connecting
static int connect_blink_led_id = -1;

static void print_stats() {
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

static void led2_on() {
    led2 = LED_ON;
}

static void led2_off() {
    led2 = LED_OFF;
}

static void blink_led2() {
    led2 = !led2;
}

// Send a message over LoRaWAN
static void send_message() {
#if HAS_ANEMOMETER == 1
    anemometer.enable();
    anemometer.readWindSpeed(); // reset sampling time
    wait_ms(3000); // OK, now we should have an accurate idea
#endif

    hts221.enable();
    tsl2572.enable();
    lps22hb.enable();

    CayenneLPP lpp(50);

    float value1, value2;

    hts221.get_temperature(&value1);
    hts221.get_humidity(&value2);
    printf("HTS221:  [temp] %.2f C, [hum]   %.2f%%\r\n", value1, value2);
    lpp.addTemperature(1, value1);
    lpp.addRelativeHumidity(2, value2);

    value1 = value2 = 0;
    lps22hb.get_temperature(&value1);
    lps22hb.get_pressure(&value2);
    printf("LPS22HB: [temp] %.2f C, [press] %.2f mbar\r\n", value1, value2);
    lpp.addTemperature(3, value1);
    lpp.addBarometricPressure(4, value2);

    tsl2572.read_ambient_light(&value1);
    printf("TSL2572: [lght] %.2f lux\r\n", value1);
    lpp.addLuminosity(5, static_cast<uint16_t>(value1)); // should be fine.. range is 0..60000 according to datasheet

    printf("GROVE.7: [anlg] %.2f\r\n", grove12_7.read());
    printf("GROVE.8: [anlg] %.2f\r\n", grove12_8.read());
    lpp.addAnalogOutput(6, grove12_7.read());
    lpp.addAnalogOutput(7, grove12_8.read());

#if HAS_ANEMOMETER == 1
    printf("DAVIS:   [drct] %d°,  [speed] %.2f km/h\r\n", anemometer.readWindDirection(), anemometer.readWindSpeed());
    lpp.addAnalogOutput(8, anemometer.readWindDirection());
    lpp.addAnalogOutput(9, anemometer.readWindSpeed());
    anemometer.disable();
#endif

    printf("\r\n");

    hts221.disable();
    tsl2572.disable();
    lps22hb.disable();

    printf("Sending %d bytes\n", lpp.getSize());

    int16_t retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, lpp.getBuffer(), lpp.getSize(), MSG_UNCONFIRMED_FLAG);

    // for some reason send() returns -1... I cannot find out why, the stack returns the right number. I feel that this is some weird Emscripten quirk
    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - duty cycle violation\n")
                : printf("send() - Error code %d\n", retcode);
        return;
    }

    printf("%d bytes scheduled for transmission\n", retcode);
    print_stats();

    hts221.disable();
}

static void print_buffer(uint8_t *buffer, size_t size) {
    for (size_t ix = 0; ix < size; ix++) {
        printf("%02x", buffer[ix]);
    }
}

int main() {
    // Enable trace output so we can see what the LoRaWAN stack does
    mbed_trace_init();

    // Calculate DevEUI, need to check if this is in the self-assignment range...
    uint32_t w1 = HAL_GetUIDw0();
    uint32_t w2 = HAL_GetUIDw1();
    uint32_t w3 = HAL_GetUIDw2();

    w1 += w3;

    // keep DEV_EUI[0] to 0x0 => local range
    DEV_EUI[1] = w2 >> 16 & 0xff;
    DEV_EUI[2] = w2 >> 8 & 0xff;
    DEV_EUI[3] = w2 >> 0 & 0xff;
    DEV_EUI[4] = w1 >> 24 & 0xff;
    DEV_EUI[5] = w1 >> 16 & 0xff;
    DEV_EUI[6] = w1 >> 8 & 0xff;
    DEV_EUI[7] = w1 >> 0 & 0xff;

    printf("Data Science Africa 2019\n");
    printf("DevEUI:        ");
    print_buffer(DEV_EUI, sizeof(DEV_EUI));
    printf("\nAppEUI:        ");
    print_buffer(APP_EUI, sizeof(APP_EUI));
    printf("\nAppKey:        ");
    print_buffer(APP_KEY, sizeof(APP_KEY));
    printf("\n\n");


    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("LoRa initialization failed!\n");
        return -1;
    }

    hts221.init(NULL);
    hts221.disable();

    lps22hb.init(NULL);
    lps22hb.disable();

    tsl2572.init();
    tsl2572.disable();

#if HAS_PM25 == 1
    // todo
#endif

    // Add a PullUp to the unused pins
    for (size_t ix = 0; ix < sizeof(unused) / sizeof(unused[0]); ix++) {
        unused[ix].mode(PullUp);
    }

    // This way we can check if device is on
    btn2.fall(&led2_on);
    btn2.rise(&led2_off);

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Enable adaptive data rating
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("enable_adaptive_datarate failed!\n");
        return -1;
    }

    lorawan.set_device_class(CLASS_A);

    lorawan_connect_t connect_params;
    connect_params.connect_type = LORAWAN_CONNECTION_OTAA;

    connect_params.connection_u.otaa.dev_eui = DEV_EUI;
    connect_params.connection_u.otaa.app_eui = APP_EUI;
    connect_params.connection_u.otaa.app_key = APP_KEY;
    connect_params.connection_u.otaa.nb_trials = 3;

    lorawan_status_t retcode = lorawan.connect(connect_params);

    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("Connection error, code = %d\n", retcode);
        return -1;
    }

    printf("Connection - In Progress ...\r\n");

    // Blink until device is connected
    connect_blink_led_id = ev_queue.call_every(1000, &blink_led2);

    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;
}

// This is called from RX_DONE, so whenever a message came in
static void receive_message()
{
    uint8_t rx_buffer[50] = { 0 };
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf("RX Data on port %u (%d bytes): ", port, retcode);
    for (uint8_t i = 0; i < retcode; i++) {
        printf("%02x ", rx_buffer[i]);
    }
    printf("\r\n");
}

// Event handler
static void lora_event_handler(lorawan_event_t event) {
    loramac_protocol_params params;

    switch (event) {
        case CONNECTED:
            printf("Connection - Successful\n");
            ev_queue.call_every(10000, &send_message);

            ev_queue.cancel(connect_blink_led_id);

            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("Disconnected Successfully\n");
            break;
        case TX_DONE:
        {
            printf("Message Sent to Network Server\n");
            break;
        }
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("Transmission Error - EventCode = %d\n", event);
            break;
        case RX_DONE:
            printf("Received message from Network Server\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("Error in reception - Code = %d\n", event);
            break;
        case JOIN_FAILURE:
            printf("OTAA Failed - Check Keys\n");
            NVIC_SystemReset();
            break;
        default:
            printf("Unknown Event\n");
            NVIC_SystemReset();
            break;
    }
}

#endif // APPLICATION == LORAWAN_TEST
