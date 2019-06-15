#include <stdio.h>
#include "periph/gpio.h"
#include <xtimer.h>
#include "hdc2010.h"
#include "hdc2010_params.h"
#include "net/netdev.h"
#include "net/netdev/lora.h"
#include "net/lora.h"

#include "board.h"

#include "sx127x_internal.h"
#include "sx127x_params.h"
#include "sx127x_netdev.h"

#define LED GPIO_PIN(PA, 27)
#define DONE GPIO_PIN(PA, 19)

#define SX127X_LORA_MSG_QUEUE   (16U)
#define SX127X_STACKSIZE        (THREAD_STACKSIZE_DEFAULT)

#define MSG_TYPE_ISR            (0x3456)

static char stack[SX127X_STACKSIZE];
static kernel_pid_t _recv_pid;

static char message[32];

hdc2010_t hdc2010 = { };

struct METRICS
{
	int16_t temperature;
	int16_t humidity;
	int16_t bat_voltage;
	uint32_t seq;
};

struct METRICS data = { };

#define SX1276_PARAMS             { .spi       = SPI_DEV(0),         \
                                    .nss_pin   = GPIO_PIN(PA, 23),   \
                                    .reset_pin = GPIO_PIN(PA, 11),   \
                                    .dio0_pin  = GPIO_PIN(PA, 9),    \
                                    .dio1_pin  = GPIO_UNDEF,         \
                                    .dio2_pin  = GPIO_UNDEF,         \
                                    .dio3_pin  = GPIO_UNDEF,         \
                                    .paselect  = SX127X_PA_RFO }

static const sx127x_params_t sx1276_params[] =
{
	SX1276_PARAMS
};

static sx127x_t sx127x;

void lora_init(uint32_t chan, uint16_t tx_power, uint8_t lora_bw, uint8_t lora_sf, uint8_t lora_cr) {
	uint16_t type = NETDEV_TYPE_LORA;
	netdev_t *netdev = (netdev_t*) &sx127x;

	netdev->driver->set(netdev, NETOPT_DEVICE_TYPE, &type, sizeof(type));
	netdev->driver->set(netdev, NETOPT_TX_POWER, &tx_power, sizeof(tx_power));
	netdev->driver->set(netdev, NETOPT_CHANNEL_FREQUENCY, &chan, sizeof(chan));
	netdev->driver->set(netdev, NETOPT_BANDWIDTH,
			&lora_bw, sizeof(lora_bw));
	netdev->driver->set(netdev, NETOPT_SPREADING_FACTOR,
			&lora_sf, sizeof(lora_sf));
	netdev->driver->set(netdev, NETOPT_CODING_RATE,
			&lora_cr, sizeof(lora_cr));
}

void flash_led(int num_times) {
	while (num_times > 0) {
		gpio_set(LED);
		xtimer_usleep(500000);
		gpio_clear(LED);
		xtimer_usleep(500000);
		num_times--;
	}
}

void flash_value(int num_times) {
	while (num_times > 0) {
		gpio_set(LED);
		xtimer_usleep(200000);
		gpio_clear(LED);
		xtimer_usleep(200000);
		num_times--;
	}
}

static void _event_cb(netdev_t *dev, netdev_event_t event)
{
	if (event == NETDEV_EVENT_ISR) {
		msg_t msg;

		msg.type = MSG_TYPE_ISR;
		msg.content.ptr = dev;

		if (msg_send(&msg, _recv_pid) <= 0) {
			puts("gnrc_netdev: possibly lost interrupt.");
		}
	}
	else {
		size_t len;
		netdev_lora_rx_info_t packet_info;
		switch (event) {
			case NETDEV_EVENT_RX_STARTED:
				puts("Data reception started");
			break;

			case NETDEV_EVENT_RX_COMPLETE:
				len = dev->driver->recv(dev, NULL, 0, 0);
				dev->driver->recv(dev, message, len, &packet_info);
				printf("{Payload: \"%s\" (%d bytes), RSSI: %i, SNR: %i, TOA: %" PRIu32 "}\n",
					message, (int)len,
					packet_info.rssi, (int)packet_info.snr,
					sx127x_get_time_on_air((const sx127x_t*)dev, len));
			break;

			case NETDEV_EVENT_TX_COMPLETE:
	flash_value(5);
	gpio_set(DONE);
				sx127x_set_sleep(&sx127x);
				puts("Transmission completed");
			break;

			case NETDEV_EVENT_CAD_DONE:
			break;

			case NETDEV_EVENT_TX_TIMEOUT:
	flash_value(7);
	gpio_set(DONE);
				sx127x_set_sleep(&sx127x);
			break;

			default:
				printf("Unexpected netdev event received: %d\n", event);
			break;
		}
	}
}

void *_recv_thread(void *arg)
{
	(void)arg;

	static msg_t _msg_q[SX127X_LORA_MSG_QUEUE];
	msg_init_queue(_msg_q, SX127X_LORA_MSG_QUEUE);

	while (1) {
		msg_t msg;
		msg_receive(&msg);
		if (msg.type == MSG_TYPE_ISR) {
			netdev_t *dev = msg.content.ptr;
			dev->driver->isr(dev);
		}
		else {
			puts("Unexpected msg type");
		}
	}
}

void init(void) {
	int status;
	netdev_t *netdev = (netdev_t*) &sx127x;

	gpio_init(LED, GPIO_OUT);
	gpio_set(LED);
	gpio_init(DONE, GPIO_OUT);
	gpio_clear(DONE);
	xtimer_usleep(1000000);
	gpio_clear(LED);
	xtimer_usleep(1000000);
	if ((status = hdc2010_init(&hdc2010, hdc2010_params)) != HDC2010_OK) {
		/*
		switch(status) {
			case HDC2010_NOBUS:
				flash_led(2);
			break;
			case HDC2010_NODEV:
				flash_led(3);
			break;
			case HDC2010_BUSERR:
				flash_led(4);
			break;
		}
		*/
		while(1)
			xtimer_usleep(1000000);
	}
//	sx127x_setup(&sx127x, &sx1276_params[0]);
	sx127x.params = sx1276_params[0];
	netdev->driver = &sx127x_driver;

	if ((status = netdev->driver->init(netdev)) < 0) {
		puts("Failed to initialize SX127x device, exiting");
		switch(status) {
			int res;
			case -SX127X_ERR_SPI:

				flash_led(2);
			xtimer_usleep(1000000);
				/* Setup SPI for SX127X */
				res = spi_init_cs(((sx127x_t *)netdev)->params.spi, ((sx127x_t *)netdev)->params.nss_pin);

				switch(res) {
					case SPI_OK:
						flash_led(1);
					break;
					case SPI_NODEV:
						flash_led(2);
					break;
					case SPI_NOCS:
						flash_led(3);
					break;
					case SPI_NOMODE:
						flash_led(4);
					break;
					case SPI_NOCLK:
						flash_led(5);
					break;
					default:
						flash_led(6);
					break;
				}
			break;
			case -SX127X_ERR_NODEV:
				flash_led(3);
			break;
			case -SX127X_ERR_GPIOS:
				flash_led(4);
			break;
			default:
				flash_led(5);
			break;
		}
		while(1)
			xtimer_usleep(1000000);
	}

	netdev->event_callback = _event_cb;
	_recv_pid = thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1,
			THREAD_CREATE_STACKTEST, _recv_thread, NULL,
			"recv_thread");

	if (_recv_pid <= KERNEL_PID_UNDEF) {
		puts("Creation of receiver thread failed");
		return;
	}

	lora_init(915000000, 14, LORA_BW_125_KHZ, LORA_SF10, LORA_CR_4_5);
}

void send_temp(uint16_t temp, uint16_t hum)
{
	int ret;
        data.temperature = temp;
        data.humidity = hum;
        data.bat_voltage = 0;
        data.seq++;

	iolist_t iolist = {
		.iol_base = &data,
		.iol_len = sizeof(data)
	};

	netdev_t *netdev = (netdev_t*) &sx127x;
	if ((ret = netdev->driver->send(netdev, &iolist)) <= 0) {
		puts("Cannot send: radio is still transmitting");
		return;
	}
//	flash_value(2);
}

int main(void)
{
	int16_t temp = 0, hum = 0;
	puts("Hello World!");

	printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
	printf("This board features a(n) %s MCU.\n", RIOT_MCU);
	init();
	xtimer_usleep(2000000);
	if (hdc2010_read(&hdc2010, &temp, &hum) == HDC2010_OK) {
	}
	send_temp(temp, hum);

	return 0;
}
