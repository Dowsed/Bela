/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example Trill/general-custom-address

Trill use a custom address
======================

In this example we specify an address instead of relying on the default one.

Every different type of Trill sensor has a different default
address which you can see in the below table:

| Type:  | Address |
|--------|---------|
| BAR    |  0x20   |
| SQUARE |  0x28   |
| CRAFT  |  0x30   |
| RING   |  0x38   |
| HEX    |  0x40   |
| FLEX   |  0x48   |

You can change the address of your device by jumpering some solder pads
on the device itself. This is needed when you have several devices of the
same type on the same bus, as there can be no two devices with the same
address. This is explained in detail here:
https://learn.bela.io/products/trill/all-about-i2c/#about-i2c-addresses

In this example we use a Trill Bar where we bridged together the two left-most
pads of the ADR0 line, which corresponds to an address of 0x21 (in hexadecimal
notation), or 33 (in decimal notation).

We have to pass this address to the `touchSensors.setup()` function in order
to be able to use this sensor.

*/

#include <Bela.h>
#include <libraries/Trill/Trill.h>

Trill touchSensor;

AuxiliaryTask readI2cTask;

// Interval for printing the readings from the sensor
float printInterval = 0.1; //s
unsigned int printIntervalSamples = 0;
// Sleep time for auxiliary task
unsigned int gTaskSleepTime = 12000; // microseconds

void loop(void*)
{
	while(!Bela_stopRequested()) {
		touchSensor.readI2C();
		usleep(gTaskSleepTime);
	}
}

bool setup(BelaContext *context, void *userData)
{
	// Setup a Trill Bar on i2c bus 1, using the custom address 0x21 (33 in
	// decimal). The address is the fourth argument to setup(), so we have to
	// explicitate the third argument (scanning mode). We set it to AUTO, so
	// that it will be the default mode for the sensor that is detected at the
	// specified address.
	// If no device is detected at the specified address, or a device of a different
	// type from the one we requested is detected, setup() will return an error
	// code and we should stop.
	if(touchSensor.setup(1, Trill::BAR, Trill::AUTO, 0x21) != 0) {
		fprintf(stderr, "Unable to initialise Trill device. Is the address correct?\n");
		return false;
	}

	readI2cTask = Bela_createAuxiliaryTask(loop, 50, "I2C-read", NULL);
	Bela_scheduleAuxiliaryTask(readI2cTask);

	printIntervalSamples = context->audioSampleRate*(printInterval);
	return true;
}

void render(BelaContext *context, void *userData)
{
	static int readCount = 0;
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		if(readCount >= printIntervalSamples) {
			readCount = 0;
			// we print the sensor readings depending on the device mode
			if(Trill::CENTROID == touchSensor.getMode()) {
				rt_printf("Touches: %d:", touchSensor.numberOfTouches());
				for(unsigned int i = 0; i < touchSensor.numberOfTouches(); i++) {
					rt_printf("%1.3f ", touchSensor.touchLocation(i));
					if(touchSensor.is2D())
						rt_printf("%1.3f ", touchSensor.touchHorizontalLocation(i));
				}
			}
			else {
				for(unsigned int i = 0; i < touchSensor.getNumChannels(); i++)
					rt_printf("%1.3f ", touchSensor.rawData[i]);
			}
			rt_printf("\n");
		}
		readCount++;
	}
}

void cleanup(BelaContext *context, void *userData)
{
}