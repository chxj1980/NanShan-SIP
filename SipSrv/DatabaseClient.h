#pragma once
#include "config.h"

#pragma comment(lib, "ociliba.lib")
#include "ocilib.h"

class DatabaseClient
{
public:
	DatabaseClient();
	~DatabaseClient();
public:
	bool Connect(int driver, const char *addr, const char *server, const char *user, const char *pwd, int thdnum=16);
	void Disconnect();
	bool Open();
	void Close();

	bool GetRegisterInfo(const char *localcode, register_map& regmap);
	bool UpdateRegister(const char *addr, bool online);
	bool SaveRegister(const char * localcode, register_info& reg);
	bool GetChannelInfo(channel_map &chnmap, int role=SIP_ROLE_SUPERIOR);
	bool GetUserCatalog(catalog_list &catlist, const char *deviceid=NULL);
	bool SaveCatalog(CatalogInfo *cataloginfo, int catalogtype);
private:
	int m_driver;
	int m_role;
public:
	//oracle
	OCI_Pool *m_pool;
	OCI_Connection *m_ocn;
	//acl
	acl::db_handle *m_db;
};

