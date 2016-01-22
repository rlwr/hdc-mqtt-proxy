/*
 * mqtt.h
 *
 *  Created on: Aug 18, 2015
 *      Author: rlorriau
 */

#ifndef MQTT_H_
#define MQTT_H_

typedef int (*mqtt_process_fcn)(char *topic_name, int topic_len, char *msg, int msg_len);

int mqtt_init(char *address, char *client_id, char *topic, mqtt_process_fcn process_fcn);
void mqtt_deinit(void);
int mqtt_loop(void);

#endif /* MQTT_H_ */
