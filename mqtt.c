/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <MQTTAsync.h>

#include "mqtt.h"

#define QOS         1

static int disc_finished = 0;
static int subscribed = 0;
static int finished = 0;
static int must_exit = 0;

#define PARAM_MAXLEN	256

struct mqtt_settings_t
{
	char address[PARAM_MAXLEN];
	char client_id[PARAM_MAXLEN];
	char topic[PARAM_MAXLEN];
	int initialized;
	
	mqtt_process_fcn process_fcn;
};

static struct mqtt_settings_t mqtt_settings = {
		.initialized = 0,
		.process_fcn = NULL,
};

void connlost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync) context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("ERROR: MQTT Failed to start connect, return code %d\n", rc);
		finished = 1;
	}
}

int msgarrvd(void *context, char *topic_name, int topic_len,
		MQTTAsync_message *message)
{
	mqtt_settings.process_fcn(topic_name, topic_len, message->payload, message->payloadlen);

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topic_name);
	return 1;
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
	printf("Successful disconnection\n");
	disc_finished = 1;
}

void onSubscribe(void* context, MQTTAsync_successData* response)
{
	printf("INFO: MQTT Subscribe succeeded\n");
	subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	printf("ERROR: MQTT Subscribe failed, rc %d\n", response ? response->code : 0);
	finished = 1;
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("ERROR: MQTT Connect failed, rc %d\n", response ? response->code : 0);
	finished = 1;
}

void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync) context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	printf("Successful connection\n");

	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n",
			mqtt_settings.topic, mqtt_settings.client_id, QOS);
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = client;

	if ((rc = MQTTAsync_subscribe(client, mqtt_settings.topic, QOS, &opts))
			!= MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe, return code %d\n", rc);
		mqtt_deinit();
	}
}

int mqtt_init(char *address, char *client_id, char *topic, mqtt_process_fcn process_fcn)
{
	if (!address || !client_id || !topic || !process_fcn)
	{
		printf("ERROR: Invalid parameters\n");
		return -EINVAL;
	}

	if (mqtt_settings.initialized)
	{
		printf("ERROR: MQTT has already been initialized\n");
		return -1;
	}

	strncpy(mqtt_settings.address, address, PARAM_MAXLEN);
	strncpy(mqtt_settings.client_id, client_id, PARAM_MAXLEN);
	strncpy(mqtt_settings.topic, topic, PARAM_MAXLEN);
	mqtt_settings.process_fcn = process_fcn;
	mqtt_settings.initialized = 1;

	must_exit = 0;

	return 0;
}

void mqtt_deinit(void)
{
	must_exit = 1;
	
	mqtt_settings.process_fcn = NULL;
	mqtt_settings.initialized = 0;
}

int mqtt_loop(void)
{
	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	if (!mqtt_settings.initialized)
	{
		printf("ERROR: MQTT not initialized\n");
		return -1;
	}

	MQTTAsync_create(&client, mqtt_settings.address, mqtt_settings.client_id,
			MQTTCLIENT_PERSISTENCE_NONE,
			NULL);

	MQTTAsync_setCallbacks(client, NULL, connlost, msgarrvd, NULL);

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		goto exit;
	}

	/* Wait for subscription */
	while (!subscribed && !finished)
		usleep(100000L);

	if (finished)
		goto exit;

	/* Wait for MQTT events (main loop) */
	while (!must_exit)
		usleep(10000L);

	/* Received signal or disconnect */
	disc_opts.onSuccess = onDisconnect;
	if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(-1);
	}
	
	/* Wait for disconnection */
	while (!disc_finished)
		usleep(10000L);

	exit: MQTTAsync_destroy(&client);
	return rc;
}
