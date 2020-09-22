#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../libblastpit/blastpit.h"
#include "steamcontroller.h"
#include "../../.git/untracked/t_common.h"

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int  light	    = 0;
	int  doora	    = 0;
	int  doorb	    = 0;
	bool light_is_on    = 1;
	bool door_is_closed = 1;

	printf("Starting steam controller test...\n");

	t_Blastpit *client = blastpitNew();

	// Connect to the server
	connectToServer(client, WS_SERVER, 0);

	SteamControllerDeviceEnum *pEnum = SteamController_EnumControllerDevices();
	while (pEnum) {
		SteamControllerEvent   event, previous_event;
		SteamControllerDevice *pDevice = SteamController_Open(pEnum);
		if (pDevice) {
			for (;;) {
				uint8_t res = SteamController_ReadEvent(pDevice, &event);
				if (res == STEAMCONTROLLER_EVENT_CONNECTION &&
				    event.connection.details == STEAMCONTROLLER_CONNECTION_EVENT_DISCONNECTED) {
					fprintf(stderr, "Device %p is not connected (anymore), trying next one...\n",
						pDevice);
					break;
				}

				if (res == STEAMCONTROLLER_EVENT_CONNECTION &&
				    event.connection.details == STEAMCONTROLLER_CONNECTION_EVENT_CONNECTED) {
					fprintf(stderr, "Device %p is connected, configuring...\n", pDevice);
					SteamController_Configure(pDevice, STEAMCONTROLLER_CONFIG_SEND_BATTERY_STATUS |
										   STEAMCONTROLLER_CONFIG_SEND_GYRO);
					SteamController_SetHomeButtonBrightness(pDevice, 0);
					SteamController_EnableGyro(pDevice);
				}

				if (res == 1) {
					if (event.update.buttons & 32)
						return 0;
					if (event.update.buttons & 64) {
						if (light == 0) {
							light_is_on = !light_is_on;
							BpSetLightState(client, light_is_on);
							light = 1;
						}
					} else {
						light = 0;
					}
					if (event.update.buttons & 16) {
						if (doora == 0) {
							BpSetDoorState(client, 0);
							doora = 1;
						}
					} else {
						doora = 0;
					}
					if (event.update.buttons & 128) {
						if (doorb == 0) {
							BpSetDoorState(client, 1);
							doorb = 1;
						}
					} else {
						doorb = 0;
					}
					if (abs(event.update.orientation.x - previous_event.update.orientation.x) > 5) {
						// just print the value of the left touch pad / stick position
						fprintf(stderr,
							"% 8d % 6d % 6d % 6d % 6d % 6d % 6d % 6hd % 6hd % 6hd % 6hd % "
							"6hd % 6hd % 6hd % 6hd % 6hd\n",
							event.update.buttons, event.update.leftTrigger,
							event.update.rightTrigger, event.update.leftXY.x,
							event.update.leftXY.y, event.update.rightXY.x,
							event.update.rightXY.y, event.update.acceleration.x,
							event.update.acceleration.y, event.update.acceleration.z,
							event.update.orientation.x, event.update.orientation.y,
							event.update.orientation.z, event.update.angularVelocity.x,
							event.update.angularVelocity.y, event.update.angularVelocity.z);
						previous_event = event;
						// 16 open 128 close 64 light 32 exit
					}
				}
				pollMessages(client);
			}

			SteamController_Close(pDevice);
		}
		pEnum = SteamController_NextControllerDevice(pEnum);
	}

	disconnectFromServer(client);
	blastpitDelete(client);

	printf("Ending steam controller test...\n");

	return 0;
}
