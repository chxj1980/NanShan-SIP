#include "stdafx.h"
#include "DatabaseClient.h"

DatabaseClient::DatabaseClient()
{
	m_pool = NULL;
	m_ocn = NULL;
	m_db = NULL;
}


DatabaseClient::~DatabaseClient()
{
	m_pool = NULL;
	m_ocn = NULL;
	m_db = NULL;
}

void OCI_ERROR_CB(OCI_Error *err)
{
	unsigned int err_type = OCI_ErrorGetType(err);
	const char *err_msg = OCI_ErrorGetString(err);
	if (err_type == OCI_ERR_WARNING)
	{
		printf("DB Waring:%s\n", err_msg);
	}
	else
	{
		printf("DB Error:%s\n", err_msg);
		printf("%s\n", OCI_GetSql(OCI_ErrorGetStatement(err)));
	}
}

bool DatabaseClient::Connect(int driver, const char * addr, const char * server, const char * user, const char * pwd, int thdnum)
{
	if (driver != DB_SQLITE)
	{
		if (addr == NULL || server == NULL || user == NULL || pwd == NULL ||
			addr[0] == '\0' || server[0] == '\0' || user[0] == '\0' || pwd[0] == '\0')
		{
			printf("DB Connect Param Is Invalid!\n");
			return false;
		}
	}
	m_driver = driver;
	char szLibPath[256] = { 0 };
	_getcwd(szLibPath, sizeof(szLibPath));
	if (driver == DB_ORACLE)
	{
		if (!OCI_Initialize(OCI_ERROR_CB, szLibPath, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
		{
			printf("DB Initialize Failure!\n");
			return false;
		}
		char szAddr[1024] = { 0 };
		sprintf(szAddr, "%s/%s", addr, server);
		m_pool = OCI_PoolCreate(szAddr, user, pwd, OCI_POOL_CONNECTION, OCI_SESSION_DEFAULT, 0, thdnum, 1);
		if (!m_pool)
		{
			printf("DB Pool Create Failure!\n");
			return false;
		}
	}
	else if (driver == DB_MYSQL)
	{
#ifdef _WINDOWS
		strcat(szLibPath, "\\libmysql.dll");
#else
		strcat(szLibPath, "/libmysqlclient_r.so");
#endif
		acl::db_handle::set_loadpath(szLibPath);
		char szAddr[1024] = { 0 };
		sprintf(szAddr, "%s:3306", addr);
		m_db = new acl::db_mysql(szAddr, server, user, pwd);
		return m_db->open();
	}
	else if (driver = DB_SQLITE)
	{
#ifdef _WINDOWS
		strcat(szLibPath, "\\sqlite3.dll");
#else
		strcat(szLibPath, "/libsqlite3.so");
#endif
		acl::db_handle::set_loadpath(szLibPath);
		m_db = new acl::db_sqlite("siplite.db");
		return m_db->open();
	}
	else
	{
		return false;
	}
	return true;
}

void DatabaseClient::Disconnect()
{
	if (m_driver == DB_ORACLE)
	{
		if (m_pool)
		{
			OCI_PoolFree(m_pool);
			m_pool = NULL;
			OCI_Cleanup();
		}
	}
	else if (m_driver == DB_SQLITE)
	{
		if (m_db)
		{
			m_db->close();
			delete m_db;
			m_db = NULL;
		}
	}
}

bool DatabaseClient::Open()
{
	if (m_driver == DB_ORACLE)
	{
		if (!m_ocn)
		{
			m_ocn = OCI_PoolGetConnection(m_pool, NULL);
			if (!m_ocn)
				return false;
		}
		else
		{
			if (!OCI_IsConnected(m_ocn))
			{
				OCI_ConnectionFree(m_ocn);
				m_ocn = OCI_PoolGetConnection(m_pool, NULL);
				if (!m_ocn)
					return false;
			}
		}
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		if (m_db)
		{
			if (!m_db->is_opened())
			{
				if (!m_db->open())
					return false;
			}
		}
	}
	return true;
}

void DatabaseClient::Close()
{
	if (m_driver == DB_ORACLE)
	{
		if (m_ocn)
		{
			OCI_ConnectionFree(m_ocn);
			m_ocn = NULL;
		}
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		//m_db->close();
	}
}

bool DatabaseClient::GetRegisterInfo(const char * localcode, register_map& regmap)
{
	if (m_driver == DB_ORACLE)
	{
		OCI_Connection *ocn = OCI_PoolGetConnection(m_pool, NULL);
		if (!ocn)
			return false;
		OCI_Statement *ost = OCI_StatementCreate(ocn);
		if (!ost)
			return false;
		char sqlstr[1024] = { 0 };
		//sprintf(sqlstr, "SELECT NAME,SIPCODE,BINDIP,EMAIL,SIPPASSWORD,(case when USERGRADE = 10 then 1 else 2 end) as userrole, \
		//	(case when DEFAULTDIVIDEDNUM = 1 then 0 when DEFAULTDIVIDEDNUM = 9 then 2 else 1 end) as transfer, ISVALID, USERDESC FROM USERINFO EMAIL = '%s'", localcode);
		sprintf(sqlstr, "select access_name,access_servecode,concat(concat(access_ip,':'),access_no) as access_addr,share_serviceid,access_password,access_role,share_transfer,share_status,mapping_addr \
			from xy_share.resource_apply_bill where share_serviceid = '%s'", localcode);
		int nRet = OCI_ExecuteStmt(ost, sqlstr);
		OCI_Resultset *ors = OCI_GetResultset(ost);
		if (!ors)
			return false;
		while (OCI_FetchNext(ors))
		{
			acl::string sName = OCI_GetString(ors, 1);
			acl::string sSipCode = OCI_GetString(ors, 2);
			acl::string sBindIp = OCI_GetString(ors, 3);
			acl::string sUser = OCI_GetString(ors, 4);
			acl::string sPwd = OCI_GetString(ors, 5);
			int nRole = OCI_GetInt(ors, 6);
			int nTransfer = OCI_GetInt(ors, 7);
			int nValid = OCI_GetInt(ors, 8);
			acl::string sMappingIp = OCI_GetString(ors, 9);
			regmap_iter riter = regmap.find(sBindIp.c_str());
			if (riter != regmap.end())
			{
				riter->second.vaild = nValid ? true : false;
			}
			else
			{
				if (nValid == 1)
				{
					register_info rinfo;
					memset(&rinfo, 0, sizeof(register_info));
					if (sName != "")
						strcpy(rinfo.name, sName.c_str());
					if (sBindIp != "")
						strcpy(rinfo.addr, sBindIp.c_str());
					if (sMappingIp != "")
						strcpy(rinfo.mapping_addr, sMappingIp.c_str());
					if (sSipCode != "")
						strcpy(rinfo.user, sSipCode.c_str());
					if (sPwd != "")
						strcpy(rinfo.pwd, sPwd.c_str());
					rinfo.role = nRole; //。。。
					rinfo.tranfer = nTransfer;
					rinfo.expires = 36000;
					rinfo.vaild = true;
					rinfo.slist = new session_list;
					regmap.insert(register_map::value_type(sBindIp, rinfo));
				}
			}
		}
		OCI_ReleaseResultsets(ost);
		OCI_StatementFree(ost);
		OCI_ConnectionFree(ocn);
		ost = NULL;
		ocn = NULL;
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		if (m_db)
		{
			if (!m_db->is_opened())
			{
				if (!m_db->open())
					return false;
			}
		}
		char sqlstr[1024] = { 0 };
		sprintf(sqlstr, "select UserName,SipCode,SipAddr,SipSrvCode,SipPwd,Role,Transfer,Status,Protocol,SipSrvPort,MappingAddr from sipuser where SipSrvCode='%s' and Status=1", localcode);
		if (m_db->sql_select(sqlstr) == false)
			return false;
		const acl::db_rows *result = m_db->get_result();
		if (result)
		{
			const std::vector<acl::db_row*>& rows = result->get_rows();
			for (size_t i = 0; i < rows.size(); i++)
			{
				const acl::db_row *row = rows[i];
				register_info rinfo;
				memset(&rinfo, 0, sizeof(register_info));
				int j = 0;
				if ((*row)[j])
					strcpy(rinfo.name, (*row)[j]);
				if ((*row)[j + 1])
					strcpy(rinfo.user, (*row)[j + 1]);
				if ((*row)[j + 2])
					strcpy(rinfo.addr, (*row)[j + 2]);
				if ((*row)[j + 4])
					strcpy(rinfo.pwd, (*row)[j + 4]);
				if ((*row)[j + 5])
					rinfo.role = atoi((*row)[j + 5]);
				if ((*row)[j + 6])
					rinfo.tranfer = atoi((*row)[j + 6]);
				if ((*row)[j + 7])
					rinfo.online = atoi((*row)[j + 7]) == 1 ? true : false;
				if ((*row)[j + 8])
					rinfo.protocol = atoi((*row)[j + 8]);
				if ((*row)[j + 9])
					rinfo.tcpport = atoi((*row)[j + 9]);
				if ((*row)[j + 10])
					strcpy(rinfo.mapping_addr, (*row)[j + 10]);
				rinfo.expires = 36000;
				rinfo.vaild = true;
				rinfo.slist = new session_list;
				regmap.insert(register_map::value_type((*row)[j + 2], rinfo));
			}
			m_db->free_result();
		}
	}
	return true;
}

bool DatabaseClient::UpdateRegister(const char * addr, bool online)
{
	if (m_driver == DB_ORACLE)
	{
		OCI_Connection *ocn = OCI_PoolGetConnection(m_pool, NULL);
		if (!ocn)
			return false;
		OCI_Statement *ost = OCI_StatementCreate(ocn);
		if (!ost)
			return false;
		char szIp[24] = { 0 };
		strcpy(szIp, addr);
		char *p = strchr(szIp, ':');
		int nPort = atoi(p + 1);
		p[0] = '\0';
		char sqlstr[1024] = { 0 };
		sprintf(sqlstr, "update xy_share.resource_apply_bill set share_status = %d where access_ip='%s' and access_no=%d", online, szIp, nPort);
		OCI_ExecuteStmt(ost, sqlstr);
		OCI_Commit(ocn);
		OCI_StatementFree(ost);
		OCI_ConnectionFree(ocn);
		ost = NULL;
		ocn = NULL;
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		if (m_db)
		{
			if (!m_db->is_opened())
			{
				if (!m_db->open())
					return false;
			}
		}
		char sqlstr[1024] = { 0 };
		sprintf(sqlstr, "UPDATE sipuser SET Status = %d WHERE SipAddr='%s'", online, addr);
		return m_db->sql_update(sqlstr);
	}
	return true;
}

bool DatabaseClient::SaveRegister(const char * localcode, register_info& reg)
{
	if (m_driver == DB_ORACLE)
	{
		OCI_Connection *ocn = OCI_PoolGetConnection(m_pool, NULL);
		if (!ocn)
			return false;
		OCI_Statement *ost = OCI_StatementCreate(ocn);
		if (!ost)
			return false;
		char sqlstr[1024] = { 0 };
		sprintf(sqlstr, "select count(*) as count from xy_share.resource_apply_bill where access_servecode='%s' and share_serviceid='%s'", reg.user, localcode);
		int nRet = OCI_ExecuteStmt(ost, sqlstr);
		OCI_Resultset *ors = OCI_GetResultset(ost);
		if (!ors)
			return false;
		OCI_FetchNext(ors);
		int nUpdate = OCI_GetInt(ors, 1);
		OCI_ReleaseResultsets(ost);
		char szIp[24] = { 0 };
		strcpy(szIp, reg.addr);
		char *p = strchr(szIp, ':');
		int nPort = atoi(p + 1);
		p[0] = '\0';
		if (nUpdate > 0)
		{
			sprintf(sqlstr, "update xy_share.resource_apply_bill set access_ip='%s', access_no=%d, share_status = %d where access_servecode='%s' and share_serviceid='%s'",szIp, nPort, reg.online, reg.user, localcode);
			OCI_ExecuteStmt(ost, sqlstr);
			OCI_Commit(ocn);
		}
		OCI_StatementFree(ost);
		OCI_ConnectionFree(ocn);
		ost = NULL;
		ocn = NULL;
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		if (m_db)
		{
			if (!m_db->is_opened())
			{
				if (!m_db->open())
					return false;
			}
		}
		char sqlstr[1024] = { 0 };
		sprintf(sqlstr, "SELECT count(*) as count FROM sipuser where SipCode='%s'", reg.user);
		if (m_db->sql_select(sqlstr) == false)
			return false;
		const acl::db_rows *result = m_db->get_result();
		if (!result)
			return false;
		const std::vector<acl::db_row*>& rows = result->get_rows();
		int nUpdate = atoi((*rows[0])["count"]);
		m_db->free_result();
		if (nUpdate > 0)
		{
			sprintf(sqlstr, "UPDATE sipuser SET SipAddr='%s', Status=%d WHERE SipCode='%s' and SipSrvCode='%s'", reg.addr, reg.online, reg.user, localcode);
			m_db->sql_update(sqlstr);
			char szAddr[24] = { 0 };
			strcpy(szAddr, reg.addr);
			char *pos = strrchr(szAddr, ':');
			int nPort = atoi(pos + 1);
			pos[0] = '\0';
			sprintf(sqlstr, "UPDATE sipchannel SET IPAddress='%s', Port=%d WHERE ParentID = '%s'", szAddr, nPort, reg.user);
		}
		else
		{
			if (m_driver == DB_MYSQL)
			{
				sprintf(sqlstr, "INSERT INTO sipuser (UserName,SipCode,SipAddr,SipSrvCode,SipPwd,Role,Transfer,Status) VALUES ('%s','%s','%s','%s','%s',%d,%d,%d)",
					reg.name, reg.user, reg.addr, localcode, reg.pwd, reg.role, reg.tranfer, reg.online);
			}
			else
			{
				sprintf(sqlstr, "INSERT INTO sipuser (UserName,SipCode,SipAddr,SipSrvCode,SipPwd,Role,Transfer,Status,CreateTime) VALUES ('%s','%s','%s','%s','%s',%d,%d,%d,datetime('now','localtime'))",
					reg.name, reg.user, reg.addr, localcode, reg.pwd, reg.role, reg.tranfer, reg.online);
			}
		}
		return m_db->sql_update(sqlstr);
	}
	return true;
}

bool DatabaseClient::GetChannelInfo(channel_map & chnmap, int role)
{
	if (m_driver == DB_ORACLE)
	{
		OCI_Connection *ocn = OCI_PoolGetConnection(m_pool, NULL);
		if (!ocn)
			return false;
		OCI_Statement *ost = OCI_StatementCreate(ocn);
		if (!ost)
			return false;
		char sqlstr[1024] = { 0 };
		if (role == SIP_ROLE_SUPERIOR)
		{
			sprintf(sqlstr, "SELECT DeviceID,Name,Status,ServerID,IPAddress,Port,PTZType,Resolution FROM SIPCHANNEL");
		}
		else
		{
			sprintf(sqlstr, "SELECT C.CHANNO,C.CHANNAME,(case when S.STATEVALUE=1 then 'ON' else 'OFF' end) as STATEVALUE,C.SIPSERVECODE,D.DVRIP,D.DVRPORT, \
			(case when C.ISPTZ = 0 then 3 else 1 end) as PTZTYPE, (case when C.HEVIDEOCODING = 'H265' then 'v/5////a///' else 'v/2////a///' end) as RESOLUTION \
			FROM CHANNELINFO C, DVRINFO D, XY_YUNWEI.DEVICESTATE S WHERE C.CODETYPE = 20 AND C.DVRID = D.ID AND  C.ID = S.DEVICEID");
		}
		int nRet = OCI_ExecuteStmt(ost, sqlstr);
		OCI_Resultset *ors = OCI_GetResultset(ost);
		if (!ors)
			return false;
		while (OCI_FetchNext(ors))
		{
			acl::string sDeviceID = OCI_GetString(ors, 1);
			acl::string sName = OCI_GetString(ors, 2);
			acl::string sStatus = OCI_GetString(ors, 3);
			acl::string sSIPCode = OCI_GetString(ors, 4);
			acl::string sIPAddress = OCI_GetString(ors, 5);
			int nPort = OCI_GetInt(ors, 6);
			int nPTZType = OCI_GetInt(ors, 7);
			acl::string sResolution = OCI_GetString(ors, 8);

			ChannelInfo info;
			memset(&info, 0, sizeof(ChannelInfo));
			if (sDeviceID != "")
				strcpy(info.szDeviceID, sDeviceID.c_str());
			if (sName != "")
				strcpy(info.szName, sName.c_str());
			if (sStatus != "")
			{
				if (sStatus.find("ON", false))
					info.nStatus = 1;
				else
					info.nStatus = 0;
			}
			if (sSIPCode != "")
				strcpy(info.szSIPCode, sSIPCode.c_str());
			if (sIPAddress != "")
				strcpy(info.szIPAddress, sIPAddress.c_str());
			info.nPort = nPort;
			info.nPTZType = nPTZType;
			if (sResolution != "")
				strcpy(info.szResolution, sResolution.c_str());
			
			chnmap[sDeviceID.c_str()] = info;
		}
		OCI_ReleaseResultsets(ost);
		OCI_StatementFree(ost);
		OCI_ConnectionFree(ocn);
		ost = NULL;
		ocn = NULL;
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		if (m_db)
		{
			if (!m_db->is_opened())
			{
				if (!m_db->open())
					return false;
			}
		}
		char sqlstr[1024] = { 0 };
		sprintf(sqlstr, "SELECT DeviceID,Name,Manufacturer,Model,Owner,IPAddress,Port,Password,ChanID,Status,ServerID,PTZType,Resolution,MediaAddress FROM SIPCHANNEL");
		if (m_db->sql_select(sqlstr) == false)
			return false;
		const acl::db_rows *result = m_db->get_result();
		if (result)
		{
			const std::vector<acl::db_row*>& rows = result->get_rows();
			for (size_t i = 0; i < rows.size(); i++)
			{
				const acl::db_row *row = rows[i];
				ChannelInfo info;
				memset(&info, 0, sizeof(ChannelInfo));
				int j = 0;
				if ((*row)[j])										//DeviceID
					strcpy(info.szDeviceID, (*row)[j]);
				if ((*row)[j + 1])									//Name
					strcpy(info.szName, (*row)[j + 1]);
				if ((*row)[j + 2])									//Manufacturer
					strcpy(info.szManufacturer, (*row)[j + 2]);
				if ((*row)[j + 3])									//Model
					strcpy(info.szModel, (*row)[j + 3]);
				if ((*row)[j + 4])									//Owner
					strcpy(info.szUser, (*row)[j + 4]);
				if ((*row)[j + 5])									//IPAddress
					strcpy(info.szIPAddress, (*row)[j + 5]);
				if ((*row)[j + 6])									//Port
					info.nPort = atoi((*row)[j + 6]);
				if ((*row)[j + 7])									//Password
					strcpy(info.szPwd, (*row)[j + 7]);
				if ((*row)[j + 8])									//ChanID
					info.nChannel = atoi((*row)[j + 8]);
				if ((*row)[j + 9])									//Status
				{
					if (strstr((*row)[j + 9], "ON"))
						info.nStatus = 1;
					else
						info.nStatus = 0;
				}
				if ((*row)[j + 10])									//ServerID
					strcpy(info.szSIPCode, (*row)[j + 10]);			
				if ((*row)[j + 11])									//PTZType
					info.nPTZType = atoi((*row)[j + 11]);
				if ((*row)[j + 12])									//Resolution
					strcpy(info.szResolution, (*row)[j + 12]);
				if ((*row)[j + 13])									//MediaAddress
					strcpy(info.szMediaAddress, (*row)[j + 13]);
			
				chnmap[(*row)[j]] = info;
			}
			m_db->free_result();
		}
	}
	return true;
}

bool DatabaseClient::GetUserCatalog(catalog_list & catlist, const char * deviceid)
{
	if (m_driver == DB_ORACLE)
	{
		OCI_Connection *ocn = OCI_PoolGetConnection(m_pool, NULL);
		if (!ocn)
			return false;
		OCI_Statement *ost = OCI_StatementCreate(ocn);
		if (!ost)
			return false;
		char sqlstr[1024] = { 0 };
		//推送所有目录
		if (deviceid == NULL)
		{
			sprintf(sqlstr, "SELECT * FROM SIPORG");
			int nRet = OCI_ExecuteStmt(ost, sqlstr);
			OCI_Resultset *ors = OCI_GetResultset(ost);
			if (!ors)
				return false;
			while (OCI_FetchNext(ors))
			{
				acl::string sDeviceID = OCI_GetString2(ors, "DeviceID");
				acl::string sName = OCI_GetString2(ors, "Name");
				acl::string sManufacturer = OCI_GetString2(ors, "Manufacturer");
				acl::string sModel = OCI_GetString2(ors, "Model");
				acl::string sOwner = OCI_GetString2(ors, "Owner");
				acl::string sCivilCode = OCI_GetString2(ors, "CivilCode");
				acl::string sBlock = OCI_GetString2(ors, "Block");
				acl::string sAddress = OCI_GetString2(ors, "Address");
				acl::string sParentID = OCI_GetString2(ors, "ParentID");
				int nRegisterWay = OCI_GetInt2(ors, "RegisterWay");
				int nSecrecy = OCI_GetInt2(ors, "Secrecy");
				acl::string sIPAddress = OCI_GetString2(ors, "IPAddress");
				int nPort = OCI_GetInt2(ors, "Port");
				CatalogInfo info;
				memset(&info, 0, sizeof(CatalogInfo));
				strcpy(info.szDeviceID, sDeviceID.c_str());
				strcpy(info.szName, sName.c_str());
				strcpy(info.szManufacturer, sManufacturer.c_str());
				strcpy(info.szModel, sModel.c_str());
				strcpy(info.szOwner, sOwner.c_str());
				strcpy(info.szCivilCode, sCivilCode.c_str());
				strcpy(info.szBlock, sBlock.c_str());
				strcpy(info.szAddress, sAddress.c_str());
				strcpy(info.szParentID, sParentID.c_str());
				info.nRegisterWay = nRegisterWay;
				info.nSecrecy = nSecrecy;
				strcpy(info.szIPAddress, sIPAddress.c_str());
				info.nPort = nPort;
				catlist.push_back(info);
			}
			OCI_ReleaseResultsets(ost);
			sprintf(sqlstr, "SELECT * FROM SIPCHANNEL");
			nRet = OCI_ExecuteStmt(ost, sqlstr);
			ors = OCI_GetResultset(ost);
			if (!ors)
				return false;
			while (OCI_FetchNext(ors))
			{
				acl::string sDeviceID = OCI_GetString2(ors, "DeviceID");
				acl::string sName = OCI_GetString2(ors, "Name");
				acl::string sManufacturer = OCI_GetString2(ors, "Manufacturer");
				acl::string sModel = OCI_GetString2(ors, "Model");
				acl::string sOwner = OCI_GetString2(ors, "Owner");
				acl::string sCivilCode = OCI_GetString2(ors, "CivilCode");
				acl::string sBlock = OCI_GetString2(ors, "Block");
				acl::string sAddress = OCI_GetString2(ors, "Address");
				int nParental = OCI_GetInt2(ors, "Parental");
				acl::string sParentID = OCI_GetString2(ors, "ParentID");
				int nSatetyWay = OCI_GetInt2(ors, "SafetyWay");
				int nRegisterWay = OCI_GetInt2(ors, "RegisterWay");
				int nSecrecy = OCI_GetInt2(ors, "Secrecy");
				acl::string sIPAddress = OCI_GetString2(ors, "IPAddress");
				int nPort = OCI_GetInt2(ors, "Port");
				acl::string sPassword = OCI_GetString2(ors, "Password");
				acl::string sStatus = OCI_GetString2(ors, "Status");
				acl::string sLongitude = OCI_GetString2(ors, "Longitude");
				acl::string sLatitude = OCI_GetString2(ors, "Latitude");
				int nPTZType = OCI_GetInt2(ors, "PTZType");
				CatalogInfo info;
				memset(&info, 0, sizeof(CatalogInfo));
				strcpy(info.szDeviceID, sDeviceID);
				strcpy(info.szName, sName);
				strcpy(info.szManufacturer, sManufacturer);
				strcpy(info.szModel, sModel);
				strcpy(info.szOwner, sOwner);
				strcpy(info.szCivilCode, sCivilCode);
				strcpy(info.szBlock, sBlock);
				strcpy(info.szAddress, sAddress);
				info.nParental = nParental;
				strcpy(info.szParentID, sParentID);
				info.nSatetyWay = nSatetyWay;
				info.nRegisterWay = nRegisterWay;
				info.nSecrecy = nSecrecy;
				strcpy(info.szIPAddress, sIPAddress);
				info.nPort = nPort;
				strcpy(info.szPassword, sPassword);
				strcpy(info.szStatus, sStatus);
				strcpy(info.szLongitude, sLongitude);
				strcpy(info.szLatitude, sLatitude);
				info.nPTZType = nPTZType;
				catlist.push_back(info);
			}
			OCI_ReleaseResultsets(ost);
		}
		else if (deviceid[0] != '\0')	//推送区域目录
		{
			sprintf(sqlstr, "SELECT * FROM SIPORG START WITH DEVICEID='%s' CONNECT BY NOCYCLE PRIOR DEVICEID=PARENTID", deviceid);
			int nRet = OCI_ExecuteStmt(ost, sqlstr);
			OCI_Resultset *ors = OCI_GetResultset(ost);
			if (!ors)
				return false;
			while (OCI_FetchNext(ors))
			{
				acl::string sDeviceID = OCI_GetString2(ors, "DeviceID");
				acl::string sName = OCI_GetString2(ors, "Name");
				acl::string sManufacturer = OCI_GetString2(ors, "Manufacturer");
				acl::string sModel = OCI_GetString2(ors, "Model");
				acl::string sOwner = OCI_GetString2(ors, "Owner");
				acl::string sCivilCode = OCI_GetString2(ors, "CivilCode");
				acl::string sBlock = OCI_GetString2(ors, "Block");
				acl::string sAddress = OCI_GetString2(ors, "Address");
				acl::string sParentID = OCI_GetString2(ors, "ParentID");
				int nRegisterWay = OCI_GetInt2(ors, "RegisterWay");
				int nSecrecy = OCI_GetInt2(ors, "Secrecy");
				acl::string sIPAddress = OCI_GetString2(ors, "IPAddress");
				int nPort = OCI_GetInt2(ors, "Port");
				CatalogInfo info;
				memset(&info, 0, sizeof(CatalogInfo));
				strcpy(info.szDeviceID, sDeviceID);
				strcpy(info.szName, sName);
				strcpy(info.szManufacturer, sManufacturer);
				strcpy(info.szModel, sModel);
				strcpy(info.szOwner, sOwner);
				strcpy(info.szCivilCode, sCivilCode);
				strcpy(info.szBlock, sBlock);
				strcpy(info.szAddress, sAddress);
				strcpy(info.szParentID, sParentID);
				info.nRegisterWay = nRegisterWay;
				info.nSecrecy = nSecrecy;
				strcpy(info.szIPAddress, sIPAddress);
				info.nPort = nPort;
				catlist.push_back(info);
			}
			OCI_ReleaseResultsets(ost);
			sprintf(sqlstr, "SELECT * FROM SIPCHANNEL WHERE PARENTID IN (SELECT DEVICEID FROM SIPORG START WITH DEVICEID='%s' CONNECT BY NOCYCLE PRIOR DEVICEID=PARENTID)", deviceid);
			nRet = OCI_ExecuteStmt(ost, sqlstr);
			ors = OCI_GetResultset(ost);
			if (!ors)
				return false;
			while (OCI_FetchNext(ors))
			{
				acl::string sDeviceID = OCI_GetString2(ors, "DeviceID");
				acl::string sName = OCI_GetString2(ors, "Name");
				acl::string sManufacturer = OCI_GetString2(ors, "Manufacturer");
				acl::string sModel = OCI_GetString2(ors, "Model");
				acl::string sOwner = OCI_GetString2(ors, "Owner");
				acl::string sCivilCode = OCI_GetString2(ors, "CivilCode");
				acl::string sBlock = OCI_GetString2(ors, "Block");
				acl::string sAddress = OCI_GetString2(ors, "Address");
				int nParental = OCI_GetInt2(ors, "Parental");
				acl::string sParentID = OCI_GetString2(ors, "ParentID");
				int nSatetyWay = OCI_GetInt2(ors, "SafetyWay");
				int nRegisterWay = OCI_GetInt2(ors, "RegisterWay");
				int nSecrecy = OCI_GetInt2(ors, "Secrecy");
				acl::string sIPAddress = OCI_GetString2(ors, "IPAddress");
				int nPort = OCI_GetInt2(ors, "Port");
				acl::string sPassword = OCI_GetString2(ors, "Password");
				acl::string sStatus = OCI_GetString2(ors, "Status");
				acl::string sLongitude = OCI_GetString2(ors, "Longitude");
				acl::string sLatitude = OCI_GetString2(ors, "Latitude");
				int nPTZType = OCI_GetInt2(ors, "PTZType");
				CatalogInfo info;
				memset(&info, 0, sizeof(CatalogInfo));
				strcpy(info.szDeviceID, sDeviceID);
				strcpy(info.szName, sName);
				strcpy(info.szManufacturer, sManufacturer);
				strcpy(info.szModel, sModel);
				strcpy(info.szOwner, sOwner);
				strcpy(info.szCivilCode, sCivilCode);
				strcpy(info.szBlock, sBlock);
				strcpy(info.szAddress, sAddress);
				info.nParental = nParental;
				strcpy(info.szParentID, sParentID);
				info.nSatetyWay = nSatetyWay;
				info.nRegisterWay = nRegisterWay;
				info.nSecrecy = nSecrecy;
				strcpy(info.szIPAddress, sIPAddress);
				info.nPort = nPort;
				strcpy(info.szPassword, sPassword);
				strcpy(info.szStatus, sStatus);
				strcpy(info.szLongitude, sLongitude);
				strcpy(info.szLatitude, sLatitude);
				info.nPTZType = nPTZType;
				catlist.push_back(info);
			}
			OCI_ReleaseResultsets(ost);
		}
		OCI_StatementFree(ost);
		OCI_ConnectionFree(ocn);
		ost = NULL;
		ocn = NULL;
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		if (m_db)
		{
			if (!m_db->is_opened())
			{
				if (!m_db->open())
					return false;
			}
		}
		char sqlstr[1024] = { 0 };
		if (deviceid == NULL)
		{
			sprintf(sqlstr, "SELECT * FROM SIPORG");
			if (m_db->sql_select(sqlstr) == false)
				return false;
			const acl::db_rows *result = m_db->get_result();
			if (result)
			{
				const std::vector<acl::db_row*>& rows = result->get_rows();
				for (size_t i = 0; i < rows.size(); i++)
				{
					const acl::db_row *row = rows[i];
					acl::string sDeviceID = (*row)["DeviceID"];
					acl::string sName = (*row)["Name"];
					acl::string sManufacturer = (*row)["Manufacturer"];
					acl::string sModel = (*row)["Model"];
					acl::string sOwner = (*row)["Owner"];
					acl::string sCivilCode = (*row)["CivilCode"];
					acl::string sBlock = (*row)["Block"];
					acl::string sAddress = (*row)["Address"];
					acl::string sParentID = (*row)["ParentID"];
					acl::string sRegisterWay = (*row)["RegisterWay"];
					acl::string sSecrecy = (*row)["Secrecy"];
					acl::string sIPAddress = (*row)["IPAddress"];
					acl::string sPort = (*row)["Port"];
					CatalogInfo info;
					memset(&info, 0, sizeof(CatalogInfo));
					strcpy(info.szDeviceID, sDeviceID.c_str());
					strcpy(info.szName, sName.c_str());
					strcpy(info.szManufacturer, sManufacturer.c_str());
					strcpy(info.szModel, sModel.c_str());
					strcpy(info.szOwner, sOwner.c_str());
					strcpy(info.szCivilCode, sCivilCode.c_str());
					if (sBlock != "")
						strcpy(info.szBlock, sBlock.c_str());
					strcpy(info.szAddress, sAddress.c_str());
					strcpy(info.szParentID, sParentID.c_str());
					info.nRegisterWay = atoi(sRegisterWay.c_str());
					info.nSecrecy = atoi(sSecrecy.c_str());
					strcpy(info.szIPAddress, sIPAddress.c_str());
					info.nPort = atoi(sPort.c_str());
					catlist.push_back(info);
				}
			}
			m_db->free_result();
			sprintf(sqlstr, "SELECT * FROM SIPCHANNEL");
			if (m_db->sql_select(sqlstr) == false)
				return false;
			result = m_db->get_result();
			if (result)
			{
				const std::vector<acl::db_row*>& rows = result->get_rows();
				for (size_t i = 0; i < rows.size(); i++)
				{
					const acl::db_row *row = rows[i];
					acl::string sDeviceID = (*row)["DeviceID"];
					acl::string sName = (*row)["Name"];
					acl::string sManufacturer = (*row)["Manufacturer"];
					acl::string sModel = (*row)["Model"];
					acl::string sOwner = (*row)["Owner"];
					acl::string sCivilCode = (*row)["CivilCode"];
					acl::string sBlock = (*row)["Block"];
					acl::string sAddress = (*row)["Address"];
					acl::string sParental = (*row)["Parental"];
					acl::string sParentID = (*row)["ParentID"];
					acl::string sSatetyWay = (*row)["SafetyWay"];
					acl::string sRegisterWay = (*row)["RegisterWay"];
					acl::string sSecrecy = (*row)["Secrecy"];
					acl::string sIPAddress = (*row)["IPAddress"];
					acl::string sPort = (*row)["Port"];
					acl::string sPassword = (*row)["Password"];
					acl::string sStatus = (*row)["Status"];
					acl::string sLongitude = (*row)["Longitude"];
					acl::string sLatitude = (*row)["Latitude"];
					acl::string sPTZType = (*row)["PTZType"];
					CatalogInfo info;
					memset(&info, 0, sizeof(CatalogInfo));
					strcpy(info.szDeviceID, sDeviceID.c_str());
					strcpy(info.szName, sName.c_str());
					strcpy(info.szManufacturer, sManufacturer.c_str());
					strcpy(info.szModel, sModel.c_str());
					strcpy(info.szOwner, sOwner.c_str());
					strcpy(info.szCivilCode, sCivilCode.c_str());
					if (sBlock != "")
						strcpy(info.szBlock, sBlock.c_str());
					if (sAddress != "")
						strcpy(info.szAddress, sAddress.c_str());
					info.nParental = atoi(sParental.c_str());
					strcpy(info.szParentID, sParentID.c_str());
					info.nSatetyWay = atoi(sSatetyWay.c_str());
					info.nRegisterWay = atoi(sRegisterWay.c_str());
					info.nSecrecy = atoi(sSecrecy.c_str());
					strcpy(info.szIPAddress, sIPAddress.c_str());
					info.nPort = atoi(sPort.c_str());
					strcpy(info.szPassword, sPassword.c_str());
					strcpy(info.szStatus, sStatus.c_str());
					strcpy(info.szLongitude, sLongitude.c_str());
					strcpy(info.szLatitude, sLatitude.c_str());
					info.nPTZType = atoi(sPTZType.c_str());
					catlist.push_back(info);
				}
			}
			m_db->free_result();
		}
		else if (deviceid[0] != '\0')
		{
			if (m_driver == DB_MYSQL)
			{
				//待补
			}
			else
			{
				sprintf(sqlstr, "with recursive org as (select * from siporg where deviceid = '%s' union all select siporg.* from org join siporg on org.deviceid = siporg.parentid) select * from org", deviceid);
			}
			if (m_db->sql_select(sqlstr) == false)
				return false;
			const acl::db_rows *result = m_db->get_result();
			if (result)
			{
				const std::vector<acl::db_row*>& rows = result->get_rows();
				for (size_t i = 0; i < rows.size(); i++)
				{
					const acl::db_row *row = rows[i];
					acl::string sDeviceID = (*row)["DeviceID"];
					acl::string sName = (*row)["Name"];
					acl::string sManufacturer = (*row)["Manufacturer"];
					acl::string sModel = (*row)["Model"];
					acl::string sOwner = (*row)["Owner"];
					acl::string sCivilCode = (*row)["CivilCode"];
					acl::string sBlock = (*row)["Block"];
					acl::string sAddress = (*row)["Address"];
					acl::string sParentID = (*row)["ParentID"];
					acl::string sRegisterWay = (*row)["RegisterWay"];
					acl::string sSecrecy = (*row)["Secrecy"];
					acl::string sIPAddress = (*row)["IPAddress"];
					acl::string sPort = (*row)["Port"];
					CatalogInfo info;
					memset(&info, 0, sizeof(CatalogInfo));
					strcpy(info.szDeviceID, sDeviceID.c_str());
					strcpy(info.szName, sName.c_str());
					strcpy(info.szManufacturer, sManufacturer.c_str());
					strcpy(info.szModel, sModel.c_str());
					strcpy(info.szOwner, sOwner.c_str());
					strcpy(info.szCivilCode, sCivilCode.c_str());
					strcpy(info.szBlock, sBlock.c_str());
					strcpy(info.szAddress, sAddress.c_str());
					strcpy(info.szParentID, sParentID.c_str());
					info.nRegisterWay = atoi(sRegisterWay.c_str());
					info.nSecrecy = atoi(sSecrecy.c_str());
					strcpy(info.szIPAddress, sIPAddress.c_str());
					info.nPort = atoi(sPort.c_str());
					catlist.push_back(info);
				}
			}
			m_db->free_result();
			if (m_driver == DB_MYSQL)
			{
				//待补
			}
			else
			{
				sprintf(sqlstr, "with recursive org as (select * from siporg where deviceid = '%s' union all select siporg.* from org join siporg on org.deviceid = siporg.parentid) \
					select * from sipchannel where parentid in (select deviceid from org)", deviceid);
			}
			if (m_db->sql_select(sqlstr) == false)
				return false;
			result = m_db->get_result();
			if (result)
			{
				const std::vector<acl::db_row*>& rows = result->get_rows();
				for (size_t i = 0; i < rows.size(); i++)
				{
					const acl::db_row *row = rows[i];
					acl::string sDeviceID = (*row)["DeviceID"];
					acl::string sName = (*row)["Name"];
					acl::string sManufacturer = (*row)["Manufacturer"];
					acl::string sModel = (*row)["Model"];
					acl::string sOwner = (*row)["Owner"];
					acl::string sCivilCode = (*row)["CivilCode"];
					acl::string sBlock = (*row)["Block"];
					acl::string sAddress = (*row)["Address"];
					acl::string sParental = (*row)["Parental"];
					acl::string sParentID = (*row)["ParentID"];
					acl::string sSatetyWay = (*row)["SafetyWay"];
					acl::string sRegisterWay = (*row)["RegisterWay"];
					acl::string sSecrecy = (*row)["Secrecy"];
					acl::string sIPAddress = (*row)["IPAddress"];
					acl::string sPort = (*row)["Port"];
					acl::string sPassword = (*row)["Password"];
					acl::string sStatus = (*row)["Status"];
					acl::string sLongitude = (*row)["Longitude"];
					acl::string sLatitude = (*row)["Latitude"];
					acl::string sPTZType = (*row)["PTZType"];
					CatalogInfo info;
					memset(&info, 0, sizeof(CatalogInfo));
					strcpy(info.szDeviceID, sDeviceID.c_str());
					strcpy(info.szName, sName.c_str());
					strcpy(info.szManufacturer, sManufacturer.c_str());
					strcpy(info.szModel, sModel.c_str());
					strcpy(info.szOwner, sOwner.c_str());
					strcpy(info.szCivilCode, sCivilCode.c_str());
					strcpy(info.szBlock, sBlock.c_str());
					strcpy(info.szAddress, sAddress.c_str());
					info.nParental = atoi(sParental.c_str());
					strcpy(info.szParentID, sParentID.c_str());
					info.nSatetyWay = atoi(sSatetyWay.c_str());
					info.nRegisterWay = atoi(sRegisterWay.c_str());
					info.nSecrecy = atoi(sSecrecy.c_str());
					strcpy(info.szIPAddress, sIPAddress.c_str());
					info.nPort = atoi(sPort.c_str());
					strcpy(info.szPassword, sPassword.c_str());
					strcpy(info.szStatus, sStatus.c_str());
					strcpy(info.szLongitude, sLongitude.c_str());
					strcpy(info.szLatitude, sLatitude.c_str());
					info.nPTZType = atoi(sPTZType.c_str());
					catlist.push_back(info);
				}
			}
			m_db->free_result();
		}
	}
	return true;
}

bool DatabaseClient::SaveCatalog(CatalogInfo * cataloginfo, int catalogtype)
{
	if (!cataloginfo)
		return false;
	acl::string strTable = "siporg";
	if (catalogtype == SIP_ROLE_CHANNEL || catalogtype == SIP_ROLE_MONITOR)
		strTable = "sipchannel";
	else if (catalogtype == SIP_ROLE_DEVICE || catalogtype == SIP_ROLE_DECODER)
		strTable = "sipdvr";
	Open();
	char sqlstr[4096] = { 0 };
	sprintf(sqlstr, "select count(*) as count from %s where deviceid='%s' and serverid='%s'", strTable.c_str(), cataloginfo->szDeviceID, cataloginfo->szServiceID);
	if (m_driver == DB_ORACLE)
	{
		OCI_Statement *ost = OCI_StatementCreate(m_ocn);
		if (!ost)
			return false;
		int nRet = OCI_ExecuteStmt(ost, sqlstr);
		OCI_Resultset *ors = OCI_GetResultset(ost);
		if (!ors)
			return false;
		OCI_FetchNext(ors);
		int nUpdate = OCI_GetInt(ors, 1);
		OCI_ReleaseResultsets(ost);
		if (nUpdate > 0)
		{
			sprintf(sqlstr, "update %s set name='%s', ParentID='%s', IPAddress='%s', Port=%d, Status='%s', Longitude='%s', Latitude='%s', UpdateTime=sysdate where deviceid='%s' and serverid='%s'", strTable.c_str(), 
				cataloginfo->szName, cataloginfo->szParentID, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szStatus, cataloginfo->szLongitude, cataloginfo->szLatitude, cataloginfo->szDeviceID, cataloginfo->szServiceID);
		}
		else
		{
			sprintf(sqlstr, "INSERT INTO %s (DeviceID,Name,Manufacturer,Model,Owner,CivilCode,Block,Address,Parental,ParentID,SafetyWay,RegisterWay,CertNum,Certifiable,ErrCode,EndTime,Secrecy,IPAddress,Port,Password,Status,Longitude,Latitude,ServerID,UpdateTime)\
				VALUES ('%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,'%s',%d,'%s','%s','%s','%s','%s',sysdate)",
				strTable.c_str(), cataloginfo->szDeviceID, cataloginfo->szName, cataloginfo->szManufacturer, cataloginfo->szModel, cataloginfo->szOwner, cataloginfo->szCivilCode, cataloginfo->szBlock, cataloginfo->szAddress,
				cataloginfo->nParental, cataloginfo->szParentID, cataloginfo->nSatetyWay, cataloginfo->nRegisterWay, cataloginfo->szCertNum, cataloginfo->nCertifiable, cataloginfo->nErrCode, cataloginfo->szEndTime, cataloginfo->nSecrecy,
				cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szPassword, cataloginfo->szStatus, cataloginfo->szLongitude, cataloginfo->szLatitude, cataloginfo->szServiceID);
		}
		OCI_ExecuteStmt(ost, sqlstr);
		OCI_Commit(m_ocn);
		OCI_StatementFree(ost);
		ost = NULL;
	}
	else if (m_driver == DB_MYSQL || m_driver == DB_SQLITE)
	{
		if (m_db->sql_select(sqlstr) == false)
			return false;
		const acl::db_rows *result = m_db->get_result();
		if (!result)
			return false;
		const std::vector<acl::db_row*>& rows = result->get_rows();
		int nUpdate = atoi((*rows[0])["count"]);
		m_db->free_result();
		if (nUpdate > 0)
		{
			if (m_driver == DB_MYSQL)
			{
				if (strTable == "sipchannel")
				{
					sprintf(sqlstr, "update %s set name='%s', ParentID='%s', IPAddress='%s', Port=%d, Status='%s', Longitude='%s', Latitude='%s' where  deviceid='%s' and serverid='%s'", strTable.c_str(),
						cataloginfo->szName, cataloginfo->szParentID, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szStatus, cataloginfo->szLongitude, cataloginfo->szLatitude, cataloginfo->szDeviceID, cataloginfo->szServiceID);
				}
				else
					sprintf(sqlstr, "update %s set name='%s', ParentID='%s', IPAddress='%s', Port=%d, Status='%s' where  deviceid='%s' and serverid='%s'", strTable.c_str(), cataloginfo->szName, cataloginfo->szParentID, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szStatus, cataloginfo->szDeviceID, cataloginfo->szServiceID);
			}
			else  //DB_SQLITE
			{
				if (strTable == "sipchannel")
				{
					sprintf(sqlstr, "update %s set name='%s', ParentID='%s', IPAddress='%s', Port=%d, Status='%s', Longitude='%s', Latitude='%s', UpdateTime=datetime('now','localtime') where  deviceid='%s' and serverid='%s'", strTable.c_str(), 
						cataloginfo->szName, cataloginfo->szParentID, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szStatus, cataloginfo->szLongitude, cataloginfo->szLatitude, cataloginfo->szDeviceID, cataloginfo->szServiceID);
				}
				else
					sprintf(sqlstr, "update %s set name='%s', ParentID='%s', IPAddress='%s', Port=%d, Status='%s', UpdateTime=datetime('now','localtime') where  deviceid='%s' and serverid='%s'", strTable.c_str(), cataloginfo->szName, cataloginfo->szParentID, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szStatus, cataloginfo->szDeviceID, cataloginfo->szServiceID);
			}
		}
		else
		{
			if (m_driver == DB_MYSQL)
			{
				if (strTable == "siporg")
				{
					sprintf(sqlstr, "INSERT INTO siporg (DeviceID,Name,Manufacturer,Model,Owner,CivilCode,Block,Address,ParentID,SafetyWay,RegisterWay,Secrecy,IPAddress,Port,Status,ServerID)\
						VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,%d,%d,'%s',%d,'%s','%s')",
						cataloginfo->szDeviceID, cataloginfo->szName, cataloginfo->szManufacturer, cataloginfo->szModel, cataloginfo->szOwner, cataloginfo->szCivilCode, cataloginfo->szBlock, cataloginfo->szAddress,
						cataloginfo->szParentID, cataloginfo->nSatetyWay, cataloginfo->nRegisterWay, cataloginfo->nSecrecy, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szStatus, cataloginfo->szServiceID);
				}
				else if (strTable == "sipdvr")
				{
					sprintf(sqlstr, "INSERT INTO sipdvr (DeviceID,Name,Manufacturer,Model,Owner,CivilCode,Block,Address,Parental,ParentID,SafetyWay,RegisterWay,Secrecy,IPAddress,Port,Password,Status,ServerID)\
						VALUES ('%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s',%d,%d,%d,'%s',%d,'%s','%s','%s')",
						cataloginfo->szDeviceID, cataloginfo->szName, cataloginfo->szManufacturer, cataloginfo->szModel, cataloginfo->szOwner, cataloginfo->szCivilCode, cataloginfo->szBlock, cataloginfo->szAddress, cataloginfo->nParental,
						cataloginfo->szParentID, cataloginfo->nSatetyWay, cataloginfo->nRegisterWay, cataloginfo->nSecrecy, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szPassword, cataloginfo->szStatus, cataloginfo->szServiceID);
				}
				else //sipchannel
				{
					sprintf(sqlstr, "INSERT INTO sipchannel (DeviceID,Name,Manufacturer,Model,Owner,CivilCode,Block,Address,Parental,ParentID,SafetyWay,RegisterWay,CertNum,Certifiable,ErrCode,EndTime,Secrecy,IPAddress,Port,Password,Status,Longitude,Latitude,\
						PTZType,PositionType,RoomType,UseType,SupplyLightType,Direction,BusinessGroupID,DownloadSpeed,SVCSpaceSupportMode,SVCTimeSupportMode,ServerID)\
						VALUES ('%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,'%s',%d,'%s','%s','%s','%s',%d,%d,%d,%d,%d,%d,'%s','%s',%d,%d,%d,'%s')",
						cataloginfo->szDeviceID, cataloginfo->szName, cataloginfo->szManufacturer, cataloginfo->szModel, cataloginfo->szOwner, cataloginfo->szCivilCode, cataloginfo->szBlock, cataloginfo->szAddress,
						cataloginfo->nParental, cataloginfo->szParentID, cataloginfo->nSatetyWay, cataloginfo->nRegisterWay, cataloginfo->szCertNum, cataloginfo->nCertifiable, cataloginfo->nErrCode, cataloginfo->szEndTime, cataloginfo->nSecrecy,
						cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szPassword, cataloginfo->szStatus, cataloginfo->szLongitude, cataloginfo->szLatitude,
						cataloginfo->nPTZType, cataloginfo->nPositionType, cataloginfo->nRoomType, cataloginfo->nUseType, cataloginfo->nSupplyLightType, cataloginfo->nDirectionType, cataloginfo->szResolution,
						cataloginfo->szBusinessGroupID, cataloginfo->nDownloadSpeed, cataloginfo->nSVCpaceSupportMode, cataloginfo->nSVCTimeSupportMode, cataloginfo->szServiceID);
				}
			}
			else //DB_SQLITE
			{
				if (strTable == "siporg")
				{
					sprintf(sqlstr, "INSERT INTO siporg (DeviceID,Name,Manufacturer,Model,Owner,CivilCode,Block,Address,ParentID,SafetyWay,RegisterWay,Secrecy,IPAddress,Port,Status,ServerID,UpdateTime)\
						VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,%d,%d,'%s',%d,'%s','%s',datetime('now','localtime'))",
						cataloginfo->szDeviceID, cataloginfo->szName, cataloginfo->szManufacturer, cataloginfo->szModel, cataloginfo->szOwner, cataloginfo->szCivilCode, cataloginfo->szBlock, cataloginfo->szAddress,
						cataloginfo->szParentID, cataloginfo->nSatetyWay, cataloginfo->nRegisterWay, cataloginfo->nSecrecy,cataloginfo->szIPAddress, cataloginfo->nPort,cataloginfo->szStatus, cataloginfo->szServiceID);
				}
				else if (strTable == "sipdvr")
				{
					sprintf(sqlstr, "INSERT INTO sipdvr (DeviceID,Name,Manufacturer,Model,Owner,CivilCode,Block,Address,Parental,ParentID,SafetyWay,RegisterWay,Secrecy,IPAddress,Port,Password,Status,ServerID,UpdateTime)\
						VALUES ('%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s',%d,%d,%d,'%s',%d,'%s','%s','%s',datetime('now','localtime'))",
						cataloginfo->szDeviceID, cataloginfo->szName, cataloginfo->szManufacturer, cataloginfo->szModel, cataloginfo->szOwner, cataloginfo->szCivilCode, cataloginfo->szBlock, cataloginfo->szAddress, cataloginfo->nParental, 
						cataloginfo->szParentID, cataloginfo->nSatetyWay, cataloginfo->nRegisterWay, cataloginfo->nSecrecy, cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szPassword, cataloginfo->szStatus, cataloginfo->szServiceID);
				}
				else //sipchannel
				{
					sprintf(sqlstr, "INSERT INTO sipchannel (DeviceID,Name,Manufacturer,Model,Owner,CivilCode,Block,Address,Parental,ParentID,SafetyWay,RegisterWay,CertNum,Certifiable,ErrCode,EndTime,Secrecy,IPAddress,Port,Password,Status,Longitude,Latitude,\
PTZType,PositionType,RoomType,UseType,SupplyLightType,DirectionType,Resolution,BusinessGroupID,DownloadSpeed,SVCSpaceSupportMode,SVCTimeSupportMode,ServerID,UpdateTime) \
VALUES ('%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,'%s',%d,'%s','%s','%s','%s',%d,%d,%d,%d,%d,%d,'%s','%s',%d,%d,%d,'%s',datetime('now','localtime'))",
							cataloginfo->szDeviceID, cataloginfo->szName, cataloginfo->szManufacturer, cataloginfo->szModel, cataloginfo->szOwner, cataloginfo->szCivilCode, cataloginfo->szBlock, cataloginfo->szAddress,
							cataloginfo->nParental, cataloginfo->szParentID, cataloginfo->nSatetyWay, cataloginfo->nRegisterWay, cataloginfo->szCertNum, cataloginfo->nCertifiable, cataloginfo->nErrCode, cataloginfo->szEndTime, cataloginfo->nSecrecy,
							cataloginfo->szIPAddress, cataloginfo->nPort, cataloginfo->szPassword, cataloginfo->szStatus, cataloginfo->szLongitude, cataloginfo->szLatitude, 
							cataloginfo->nPTZType, cataloginfo->nPositionType, cataloginfo->nRoomType, cataloginfo->nUseType, cataloginfo->nSupplyLightType, cataloginfo->nDirectionType, cataloginfo->szResolution,
							cataloginfo->szBusinessGroupID, cataloginfo->nDownloadSpeed, cataloginfo->nSVCpaceSupportMode, cataloginfo->nSVCTimeSupportMode, cataloginfo->szServiceID);
				}

			}
		}
		m_db->sql_update(sqlstr);
	}
	return true;
}


