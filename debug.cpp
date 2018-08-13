#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#include <modbus.h>

static modbus_t *mb = 0;

static void closeSerial()
{
	if (mb) {
		modbus_close(mb);
		modbus_free(mb);
		mb = NULL;
	}
}

static int openSerial(const char *device, int baudrate, char parity, int data_bits, int stop_bits)
{
	int ret = 0;
	if (!mb) {
		mb = modbus_new_rtu(device, baudrate, parity, data_bits, stop_bits);
		struct timeval tv = { 0, 100000 };
		modbus_set_response_timeout(mb, &tv);
		modbus_set_debug(mb, 1);
		ret = modbus_connect(mb);
		if (ret < 0)
			closeSerial();
	}
	return ret;
}

static void dump(uint16_t *values, int idx, int count)
{
	for (int i=0; i<count; ++i) {
		if (i%2)
			printf("%6i: %04x %5i\n", i+idx, values[i], values[i]);
		else {
			union {
				unsigned long l;
				float f;
			} v;

			v.l = (values[i] << 16) | values[i+1];
			printf("%6i: %04x %5i %8.3f\n", i+idx, values[i], values[i], (double) v.f);
		}
	}
}

static int mbReadInput(int argc, const char *argv[])
{
	int ret = -1;

	int address = strtol(argv[0], NULL, 0);
	int idx = strtol(argv[1], NULL, 0);
	int count = strtol(argv[2], NULL, 0);

	uint16_t *values = (uint16_t*) malloc(count*sizeof(uint16_t));

	modbus_set_slave(mb, address);

	if ((idx < 30000) || (idx > 40000))
		printf("Warning: accessing %i but expecting the range %i...%i\n", idx, 30000, 40000);

	ret = modbus_read_input_registers(mb, idx-30001, count, values);
	if (ret < 0)
		fprintf(stderr, "R err %s addr %i\n", strerror(errno), address);

	printf("Reading %i input registers from device %i from index %i:\n", count, address, idx);
	dump(values, idx, count);

	return ret;
}

static int mbReadReg(int argc, const char *argv[])
{
	int ret = -1;

	int address = strtol(argv[0], NULL, 0);
	int idx = strtol(argv[1], NULL, 0);
	int count = strtol(argv[2], NULL, 0);

	uint16_t *values = (uint16_t*) malloc(count*sizeof(uint16_t));

	modbus_set_slave(mb, address);

	if ((idx < 40000) || (idx > 50000))
		printf("Warning: accessing %i but expecting the range %i...%i\n", idx, 40000, 50000);

	ret = modbus_read_registers(mb, idx-40001, count, values);
	if (ret < 0)
		fprintf(stderr, "R err %s addr %i\n", strerror(errno), address);

	printf("Reading %i holding registers from device %i from index %i:\n", count, address, idx);
	dump(values, idx, count);

	return ret;
}

static int mbWriteReg(int argc, const char *argv[])
{
	int ret = -1;

	int address = strtol(argv[0], NULL, 0);
	int idx = strtol(argv[1], NULL, 0);
	int count = argc-2;

	uint16_t *values = (uint16_t*) malloc(count*sizeof(uint16_t));
	for (int i=0; i<count; ++i)
		values[i] = strtol(argv[i+2], NULL, 0);

	modbus_set_slave(mb, address);

	if ((idx < 40000) || (idx > 50000))
		printf("Warning: accessing %i but expecting the range %i...%i\n", idx, 40000, 50000);

	ret = modbus_write_registers(mb, idx-40001, count, values);
	if (ret < 0)
		fprintf(stderr, "R err %s addr %i\n", strerror(errno), address);

	printf("Written %i holding registers to device %i from index %i with:\n", count, address, idx);
	dump(values, idx, count);

	return ret;
}
//
// static int mbWriteOutput(int argc, const char *argv[])
// {
// 	int ret = -1;
//
// 	int address = strtol(argv[0], NULL, 0);
// 	int idx = strtol(argv[1], NULL, 0);
// 	int count = argc-2;
//
// 	uint16_t *values = (uint16_t*) malloc(count*sizeof(uint16_t));
// 	for (int i=0; i<count; ++i)
// 		values[i] = strtol(argv[i+2], NULL, 0);
//
// 	modbus_set_slave(mb, address);
//
// 	if (idx > 10000)
// 		printf("Warning: accessing %i but expecting the range %i...%i\n", idx, 0, 10000);
//
// 	ret = modbus_write_output_registers(mb, idx-1, count, values);
// 	if (ret < 0)
// 		fprintf(stderr, "R err %s addr %i\n", strerror(errno), address);
//
// 	printf("Written %i output registers to device %i from index %i with:\n", count, address, idx);
// 	for (int i=0; i<count; ++i) {
// 		if (i%2)
// 			printf("%6i: %04x %5i\n", i+idx, values[i], values[i]);
// 		else
// 			printf("%6i: %04x %5i %8.3f\n", i+idx, values[i], values[i], 0);
// 	}
//
// 	return ret;
// }

static const struct AppType {
	const char *name;
	int (*func) (int argc, const char *argv[]);
	int args;
	const char *help;
} APP_TYPES[] = {
	{ "mbreadin", mbReadInput, 3, "addr idx count" },
	{ "mbreadreg", mbReadReg, 3, "addr idx count" },
	{ "mbwritereg", mbWriteReg, -3, "addr idx val..." },
//	{ "mbwriteout", mbWriteOutput, -3, "addr idx val..." },
	{ NULL }
};

static void fail(const struct AppType *at)
{
	fprintf(stderr, "Usage: %s %s\n", at->name, at->help);
	exit(1);
}

int main(int argc, char *argv[])
{
	int ret;

	const struct AppType *at = NULL;

	int speed = 9600;
	const char *device = "/dev/ttyUSB0";

	for (int i=0; APP_TYPES[i].name; ++i)
		if (!strcmp(argv[0], APP_TYPES[i].name))
			at = &APP_TYPES[i];

	if (!at) {
		printf("Unsupported command '%s'. Use one of:", argv[0]);
		for (int i=0; APP_TYPES[i].name; ++i)
			printf(" %s", APP_TYPES[i].name);
		printf("\n");
		return 1;
	}

	int c;
	while ((c = getopt (argc, argv, "s:d:D:")) != -1)
		switch (c)
		{
			case 's':
				speed = strtol(optarg, NULL, 0);
				break;
			case 'd':
			case 'D':
				device = strdup(optarg);
				break;
			case '?':
				if ((optopt == 's') || (optopt == 'd') || (optopt == 'D'))
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				fail(at);
		}

	printf("%i %i %i\n", optind, argc, at->args);
	if (at->args < 0) {
		if ((argc-optind) < -at->args)
			fail(at);
	} else {
		if ((argc-optind) != at->args)
			fail(at);
	}

	ret = openSerial(device, speed, 'N', 8, 1);
	if (ret) {
		char buf[256];
		sprintf(buf, "Opening %s at %i", device, speed);
		perror(buf);
		return ret;
	}

	ret = at->func(argc-optind, (const char **)(argv+optind));

	closeSerial();
	return ret;
}
