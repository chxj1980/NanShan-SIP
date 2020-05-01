#pragma once
#pragma comment(lib, "librdkafka.lib")

#include "rdkafka.h"

typedef void (__stdcall *msg_consume)(int64_t offset,char *message, size_t msglen, void *user);

class Consumer
{
public:
	Consumer();
	~Consumer();
public:
	//borkers ip:prot,ip2:port2
	void Start(char *brokers, char *topic, char *group=NULL, msg_consume msgcb=NULL, void *user=NULL);
	void Stop();
public:

private:
	rd_kafka_t *rk;
	rd_kafka_topic_partition_list_t *topics;
};

