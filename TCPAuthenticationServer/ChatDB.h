#pragma once
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_error.h>
#include <iostream>

#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>

#include "cCreateAccountPacket.h"

class ChatDB {
public:
	ChatDB();
	~ChatDB();

	bool Connect();
	void Disconnect();
	int CreateAccount(cCreateAccountPacket* pckt);
	bool LoginIn();

private:
	sql::Driver* driver;
	sql::Connection* connection;
	sql::Statement* pStatement;
	sql::PreparedStatement* pCreateAccountStatement;
	sql::ResultSet* pResultSet;
};