/*
 * agent.h
 *
 *  Created on: Aug 18, 2015
 *      Author: rlorriau
 */

#ifndef AGENT_H_
#define AGENT_H_

#include <wra.h>

int agent_send_string(wra_handle wra, char *data_item, char *data);
wra_handle agent_init(void);
int agent_deinit(wra_handle wra);


#endif /* AGENT_H_ */
