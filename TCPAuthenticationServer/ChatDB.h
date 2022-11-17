#pragma once
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_error.h>
#include <iostream>

#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>

#include "gen/addressbook.pb.h"

class ChatDB {
public:
	ChatDB();
	~ChatDB();

	bool Connect();
	void Disconnect();
	int CreateAccount(Authentication::CreateAccountPacket* pckt);
	bool LoginIn(Authentication::LoginPacket* pckt);

private:
	sql::Driver* driver;
	sql::Connection* connection;
	sql::Statement* pStatement;
	sql::PreparedStatement* pCreateAccountStatement;
	sql::ResultSet* pResultSet;
};