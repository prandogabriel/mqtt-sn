/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - clarifications and/or documentation extension
 *
 * Description:
 * Short topic name used to avoid registration process
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MQTTSNPacket.h"
#include "rfcomm.h"


int main(int argc, char** argv)
{
	int rc = 0;
	unsigned char buf[200];
	int buflen = sizeof(buf);
	MQTTSN_topicid topic;
	unsigned char* payload = (unsigned char*)"mypayload";
	int payloadlen = strlen((char*)payload);
	int len = 0;
	int dup = 0;
	int qos = 0;
	int retained = 0;
	short packetid = 0;
//	char *topicname = "a long topic name";
    char *host = "";
    int channel = 1;
	MQTTSNPacket_connectData options = MQTTSNPacket_connectData_initializer;

	if (argc > 1)
		host = argv[1];

	if (argc > 2)
        channel = atoi(argv[2]);

    printf("Sending to address %s port %d\n", host, channel);
    if (rfcomm_open(host, channel) < 0)
    {
        goto exit;
    }

	options.clientID.cstring = "myclientid";
	len = MQTTSNSerialize_connect(buf, buflen, &options);
    rc = rfcomm_sendPacketBuffer(buf, len);

	/* wait for connack */
    if (MQTTSNPacket_read(buf, buflen, rfcomm_getdata) == MQTTSN_CONNACK)
	{
		int connack_rc = -1;

		if (MQTTSNDeserialize_connack(&connack_rc, buf, buflen) != 1 || connack_rc != 0)
		{
			printf("Unable to connect, return code %d\n", connack_rc);
			goto exit;
		}
		else 
			printf("connected rc %d\n", connack_rc);
	}
	else
		goto exit;

	 
	/* publish with short name */
	topic.type = MQTTSN_TOPIC_TYPE_SHORT;
	memcpy(topic.data.short_name, "tt", 2);
	len = MQTTSNSerialize_publish(buf, buflen, dup, qos, retained, packetid,
			topic, payload, payloadlen);
    rc = rfcomm_sendPacketBuffer(buf, len);

	printf("rc %d from send packet for publish length %d\n", rc, len);

exit:
    rfcomm_close();

	return 0;
}
