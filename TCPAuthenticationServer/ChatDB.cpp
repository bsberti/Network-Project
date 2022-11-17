#include "ChatDB.h"

// MYSQL80 database connection info
// - ip:		127.0.0.1
// - port:		3306
// - username:	root
// - passowrd:	beth.824
// - schema:	gameword

ChatDB::ChatDB() {
	this->driver = nullptr;
	this->connection = nullptr;
	this->pStatement = nullptr;
	this->pCreateAccountStatement = nullptr;
	this->pResultSet = nullptr;
}

ChatDB::~ChatDB() {

}

bool ChatDB::Connect()
{
	try {
		this->driver = sql::mysql::get_driver_instance();
	}
	catch (sql::SQLException e) {
		printf("Failed to get_driver_instance: %s\n", e.what());
		return false;
	}
	std::cout << "Successfully retrieved our cpp-conn-sql driver!\n";
	
	try {
		sql::SQLString hostname = "127.0.0.1::3306";
		sql::SQLString username = "root";
		sql::SQLString password = "Beth.824";
		this->connection = this->driver->connect(hostname, username, password);
		this->connection->setSchema("gameworld");
	}
	catch (sql::SQLException e) {
		printf("Failed to connect to our database: %s\n", e.what());
		return false;
	}
	std::cout << "Successfully connected to our Database!\n";

	try {
		pStatement = connection->createStatement();
		pCreateAccountStatement = connection->prepareStatement(
			"INSERT INTO web_auth " 
			"(`email`, `salt`, `hashed_password`, `userId`) VALUES (?, ?, ?, ?)");
	}
	catch (sql::SQLException e) {
		printf("Failed to create statements: %s\n", e.what());
		return false;
	}

	return true;
}

void ChatDB::Disconnect() {
	// Disconnect
	try {
		this->connection->close();
	} catch (sql::SQLException e) {
		std::cout << "Failed to  close the connection to our database: %s\n", e.what();
	}
	std::cout << "Successfully close the connection to our Database!\n";

	delete pStatement;
	delete pCreateAccountStatement;
	delete pResultSet;
}

int ChatDB::CreateAccount(Authentication::CreateAccountPacket* pckt)
{
	try {
		sql::SQLString selectQuery = "SELECT * FROM web_auth WHERE email = '" + pckt->email() + "';";
		pResultSet = pStatement->executeQuery(selectQuery);
	}
	catch (sql::SQLException e) {
		printf("Failed to execute query: %s\n", e.what());
		return -1;
	}
	printf("Successfully retrieved %d rows from the database!\n", (int)pResultSet->rowsCount());
	
	if (pResultSet->rowsCount() == 0) {
		sql::SQLString salt;
		salt = std::to_string(rand() * 100000);
		pCreateAccountStatement->setString(1, pckt->email());
		pCreateAccountStatement->setString(2, salt);
		pCreateAccountStatement->setString(3, pckt->hashed_password());
		pCreateAccountStatement->setBigInt(4, std::to_string(pckt->userid()));

		try {
			pCreateAccountStatement->execute();
		}
		catch (sql::SQLException e) {
			printf("Failed to create an account: %s\n", e.what());
			return -1;
		}
	}
	else {
		// Account already exist!
		return 1;
	}
	
	return 0;
}

bool ChatDB::LoginIn(Authentication::LoginPacket* pckt)
{
	return false;
}
