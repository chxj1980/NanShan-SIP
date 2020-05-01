#include "stdafx.h"
#include "Consumer.h"

static bool consumer_running = true;
static int consumer_eof_cnt = 0;
static int consumer_partition_cnt = 0;



static void err_cb(rd_kafka_t *rk, int err, const char *reason, void *opaque) 
{
	if (err == RD_KAFKA_RESP_ERR__FATAL) 
	{
		char errstr[512];
		err = rd_kafka_fatal_error(rk, errstr, sizeof(errstr));
		printf("COMSUMER FATAL ERROR CALLBACK: %s: %s: %s\n", rd_kafka_name(rk), rd_kafka_err2str(rd_kafka_resp_err_t(err)), errstr);
	}
	else 
	{
		printf("COMSUMER ERROR CALLBACK: %s: %s: %s\n", rd_kafka_name(rk), rd_kafka_err2str(rd_kafka_resp_err_t(err)), reason);
	}
}

static void throttle_cb(rd_kafka_t *rk, const char *broker_name, int32_t broker_id, int throttle_time_ms, void *opaque) 
{
	printf("%% THROTTLED %dms by %s (%d)\n", throttle_time_ms, broker_name, broker_id);
}


static void rebalance_cb(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *partitions, void *opaque) 
{
	switch (err)
	{
	case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
		fprintf(stderr, "COMSUMER Group rebalanced: %d partition(s) assigned\n", partitions->cnt);
		consumer_eof_cnt = 0;
		consumer_partition_cnt = partitions->cnt;
		rd_kafka_assign(rk, partitions);
		break;
	case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
		fprintf(stderr, "COMSUMER Group rebalanced: %d partition(s) revoked\n", partitions->cnt);
		consumer_eof_cnt = 0;
		consumer_partition_cnt = 0;
		rd_kafka_assign(rk, NULL);
		break;
	default:
		break;
	}
}

Consumer::Consumer()
{
	rk = NULL;
	topics = NULL;
}


Consumer::~Consumer()
{
	Stop();
}

void Consumer::Start(char *brokers, char *topic, char *group, msg_consume msgcb, void *user)
{
	char errstr[1024] = { 0 };
	rd_kafka_conf_res_t res = RD_KAFKA_CONF_INVALID;
	rd_kafka_resp_err_t err = RD_KAFKA_RESP_ERR_NO_ERROR;
	rd_kafka_conf_t *conf = rd_kafka_conf_new();
	rd_kafka_topic_conf_t *tconf = rd_kafka_topic_conf_new();
	if (!conf)
	{
		printf("COMSUMER Create Conf Failed\n");
		return;
	}
	char groupid[128] = { 0 };
	strcpy(groupid, "rdkafaka_consumer_xinyi");
	if (group && group[0] != '\0')
	{
		memset(groupid, 0, sizeof(groupid));
		strcpy(groupid, group);
	}
	res = rd_kafka_conf_set(conf, "group.id", groupid, errstr, sizeof(errstr));
	res = rd_kafka_conf_set(conf, "enable.partition.eof", "true", errstr, sizeof(errstr));
	res = rd_kafka_conf_set(conf, "auto.offset.reset", "latest", errstr, sizeof(errstr));
	rd_kafka_conf_set_error_cb(conf, err_cb);
	rd_kafka_conf_set_throttle_cb(conf, throttle_cb);
	rd_kafka_conf_set_rebalance_cb(conf, rebalance_cb);
	rd_kafka_conf_set_default_topic_conf(conf, tconf);

	rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
	if (!rk)
	{
		printf("COMSUMER Create Comsumer Failed: %s\n", errstr);
		Stop();
		return;
	}
	err = rd_kafka_poll_set_consumer(rk);
	if (rd_kafka_brokers_add(rk, brokers) < 1)
	{
		printf("COMSUMER Brokers Specified\n");
		Stop();
		return;
	}

	topics = rd_kafka_topic_partition_list_new(1);
	rd_kafka_topic_partition_list_add(topics, topic, RD_KAFKA_PARTITION_UA);
	err = rd_kafka_subscribe(rk, topics);
	if (err)
	{
		printf("COMSUMER Subscribe failed: %s\n", rd_kafka_err2str(err));
		Stop();
		return;
	}

	while (consumer_running)
	{
		rd_kafka_message_t *msg = rd_kafka_consumer_poll(rk, 1000);
		if (msg)
		{
			if (msg->err == RD_KAFKA_RESP_ERR__PARTITION_EOF)
			{
				//if (consumer_eof_cnt++ == consumer_partition_cnt)
				//{
				//	printf("COMSUMER PARTITION_EOF %d\n", consumer_partition_cnt);
				//	consumer_running = false;
				//}
			}
			else if (msg->err == RD_KAFKA_RESP_ERR_NO_ERROR)
			{
				if (msgcb)
				{
					msgcb(msg->offset, (char *)msg->payload, msg->len, user);
				}
				else
				{
					printf("offset:%I64d %s\n", msg->offset, (char *)msg->payload);
				}
			}
			rd_kafka_message_destroy(msg);
		}
	}
}

void Consumer::Stop()
{
	consumer_running = false;
	if (rk)
	{
		rd_kafka_consumer_close(rk);
		rd_kafka_destroy(rk);
		rk = NULL;
	}
	if (topics)
	{
		rd_kafka_topic_partition_list_destroy(topics);
		rd_kafka_wait_destroyed(2000);
		topics = NULL;
	}

}

