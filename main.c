// $CC data.c -Ih/include/ -o data -lpthread -lwraclient -lrt
//  -Lsrc/client 

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <wra.h>

#include "agent.h"
#include "mqtt.h"

#define MQTT_ADDRESS     	"tcp://localhost:1883"
#define MQTT_CLIENTID    	"ExampleClientSub"
#define MQTT_AGENT_TOPIC	"WRA"
#define MQTT_TOPIC       	MQTT_AGENT_TOPIC "/#"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define PARAM_MAXLEN	256
#define DATA_MAXLEN		1024

static wra_handle wra = NULL;

int msg_split(char *topic, char *device_name, char *data_item)
{
	char *token, *saveptr;
	
	/* Parse topic name (should be WRA) */
	token = strtok_r(topic, "/", &saveptr);
	if (!token)
		return -1;
	if (strncmp(MQTT_AGENT_TOPIC, token, PARAM_MAXLEN))
		return -1;
	
	/* Parse device name */
	token = strtok_r(NULL, "/", &saveptr);
	if (!token)
		return -1;
	strncpy(device_name, token, PARAM_MAXLEN);
	
	/* Parse data item name */
	token = strtok_r(NULL, "/", &saveptr);
	if (!token)
			return -1;
	strncpy(data_item, token, PARAM_MAXLEN);

	return 0;
}

/* 
 * Convention for messages: 
 * 
 * TOPIC:
 * WRA/[Device ID]/[Data item name]
 * 
 * PAYLOAD:
 * Value of data item
 * 
 * */
int message_process(char *topic, int topic_len, char *msg, int msg_len)
{
	char msg_payload[DATA_MAXLEN];
	char device_name[PARAM_MAXLEN] = "";
	char data_item[PARAM_MAXLEN] = "";
	int actual_msg_len = 0;

	if ((!msg) || (msg_len < 0))
		return -EINVAL;

	if (msg_len >= DATA_MAXLEN)
		printf("WARNING: Received message is too long. Will trim\n");

	actual_msg_len = MIN(DATA_MAXLEN - 1, msg_len);

	strncpy(msg_payload, msg, actual_msg_len);
	msg_payload[actual_msg_len] = 0;
	msg_split(topic, device_name, data_item);

	printf("Message arrived\n");
	printf("   topic  : %s\n", topic);
	printf("   message: %s\n", msg_payload);
	printf("   device name: %s\n", device_name);
	printf("   data item  : %s\n", data_item);
	printf("   length (received): %d\n", msg_len);
	printf("   length (sent)    : %d\n", actual_msg_len);

	agent_send_string(wra, data_item, msg_payload);

	return 0;
}

void sigint_handler(int dummy)
{
	printf("INFO: Received CTRL-C. Shutting down...\n");
    
	mqtt_deinit();
}

int main(int argc, char * argv[])
{
	printf("INFO: Starting agent\n");
	wra = agent_init();
	if (wra == WRA_NULL)
	{
		printf("ERROR: Failed to initialize agent\n");
		return -1;
	}
	
	/* Catch CTRL-C after the agent has been connected (uninterruptible otherwise) */
	signal(SIGINT, sigint_handler);

	/* Start MQTT messaging */
	mqtt_init(MQTT_ADDRESS, MQTT_CLIENTID, MQTT_TOPIC, message_process);
	mqtt_loop();

	printf("INFO: Stopping agent\n");
	// TODO: process return code
	agent_deinit(wra);
	
	return 0;
}
