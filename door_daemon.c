#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include "gpio.h"
#include "door_daemon.h"


int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp ("-b", argv[i]) == NULL && (argc > i+1) && argv[i+1][0] != '-')
			script_boot = argv[i+1];
		if (strcmp ("-s", argv[i]) == NULL && (argc > i+1) && argv[i+1][0] != '-')
			script_start = argv[i+1];
		if (strcmp ("-e", argv[i]) == NULL && (argc > i+1) && argv[i+1][0] != '-')
			script_end = argv[i+1];
		if (strcmp ("-d", argv[i]) == NULL)
			DEBUG = 1;
	}

	syslog (LOG_NOTICE, "Door daemon start");
	if (strlen (script_boot))
		system (script_boot);

	gpio_init();

	int breakMarker = 0;
  
	/* initial state must be "0" - "door closed" */
	int lastState = GPIO_STATE_CLOSED;
  
	while (!breakMarker)
	{
		int doorState = gpio_read();
		if (DEBUG)
		{
			printf ("digitalRead: %d\n", doorState);
			syslog (LOG_NOTICE, "digitalread %d", doorState);
		}

		/*  only 0 or 1 are possible
			if we read something different, like 2 or -1
			then  we assume it is non-zero (door open) */
		if (doorState != GPIO_STATE_CLOSED)
			doorState = GPIO_STATE_OPENED;

		int sensorToggled = (doorState != lastState);

		if (sensorToggled && lastState == GPIO_STATE_CLOSED)
			breakMarker = eventDetected();
		else if (sensorToggled && lastState == GPIO_STATE_OPENED)
			breakMarker = eventEnded();
		else if (!sensorToggled && lastState == GPIO_STATE_OPENED)
			breakMarker = insideEvent();

		lastState = doorState;
		usleep (400000);
	}

	return 0;
}

int eventDetected()
{
	syslog (LOG_NOTICE, "Event start");
	if (strlen (script_start))
		system (script_start);
	return 0;
}

int insideEvent()
{
	return 0;
}

int eventEnded()
{
	syslog (LOG_NOTICE, "Event end");
	if (strlen (script_end))
		system (script_end);
	return 0;
}

