#include <SPI.h>
#include <TimerOne.h>
#include <DMD.h>
#include <Artnet.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// dmd stuff
#define DISPLAYS_ACROSS 5
#define DISPLAYS_DOWN 1
#define DISPLAYS_BPP 1
#define NUM_LEDS DISPLAYS_ACROSS * DISPLAYS_DOWN * 512
#define RED 0xFF
#define BLACK 0
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, DISPLAYS_BPP);
int brightness = 50;

byte pixels[NUM_LEDS];

// artnet stuff:
// 8 universes, each one is 2 lines of 32*5=160, so 320 pixels per universe
Artnet artnet;
const int startUniverse = 0;
const int maxUniverse = 7;
bool universesReceived[maxUniverse];
bool sendFrame = 1;

byte ip[] = { 192, 168, 1, 2 };
byte mac[] = { 0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC };

void setup() {
	Serial.begin(115200);
	// set brightness of the display
	pinMode(9, OUTPUT);
	analogWrite(9, brightness);
	// initialize timer for spi communication
	Timer1.initialize(2000);
	Timer1.attachInterrupt(scanDMD);
	// flash two time to say setup is ok
	dmd.clearScreen(RED);
	delay(500);
	dmd.clearScreen(BLACK);
	delay(500);
	dmd.clearScreen(RED);
	delay(500);
	dmd.clearScreen(BLACK);
	delay(500);
	// start artnet stuff
	artnet.begin(mac, ip);
	artnet.setArtDmxCallback(onDmxFrame);
}

void loop() {
	Timer1.detachInterrupt(); //noInterrupts();
	artnet.read();
	/* if (artnet.read() == ART_DMX) {
		// Serial.println(artnet.getUniverse());
		onDmxFrame(artnet.getUniverse(), artnet.getLength(), artnet.getSequence(), artnet.getDmxFrame());
	}*/
	Timer1.attachInterrupt(scanDMD); //interrupts();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
	Serial.println(universe);
	sendFrame = 1;
	// set brightness of the whole strip
	if (universe == 8) brightness = data[0];

	// store which universe has got in
	if ((universe - startUniverse) < maxUniverse) universesReceived[universe - startUniverse] = 1;

	// check if all universes has been received (eg. the whole frame has been received)
	// if so, we can display the new frame
	for (int i = 0 ; i < maxUniverse ; i++) {
		if (universesReceived[i] == 0) {
			sendFrame = 0;
			break;
		}
	}

	// read universe and put into the right part of the display buffer
	for (int i = 0; i < 320; i++) {
		// int x = i < 160 ? i : i - 160;
		// int y = i < 160 ? universe * 2 : universe * 2 + 1;
		int idx = i + universe * 320;
		byte on = data[i] > 0 ? RED : BLACK;
		pixels[idx] = on;
	}

	// all universes received
	if (sendFrame) {
		Serial.println("draw!");
		for (int i = 0; i < NUM_LEDS; i++) {
			dmd.writePixel(floor(i/160), i % 160, pixels[i]);
		}
		memset(pixels, 0, NUM_LEDS);
		// reset universeReceived to 0
		memset(universesReceived, 0, maxUniverse);
	}
}

void scanDMD() {
	dmd.scanDisplayBySPI();
	analogWrite(9, brightness);
}
