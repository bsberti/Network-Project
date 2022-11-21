#include "ChatDB.h"
#include <SHA256.h>

// MYSQL80 database connection info
// - ip:		127.0.0.1
// - port:		3306
// - username:	root
// - passowrd:	Beth.824
// - schema:	gameword

ChatDB::ChatDB() {
	this->driver = nullptr;
	this->connection = nullptr;
	this->pStatement = nullptr;
	this->pWebAuthStatement = nullptr;
	this->pUserCreationStatement = nullptr;
	this->pUserUpdateStatement = nullptr;
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
		pWebAuthStatement = connection->prepareStatement(
			"INSERT INTO web_auth " 
			"(`email`, `salt`, `hashed_password`, `userId`) VALUES (?, ?, ?, ?)");

		pUserCreationStatement = connection->prepareStatement(
			"INSERT INTO user "
			"(`id`, `last_login`, `creation_date`) VALUES (?, CURRENT_TIMESTAMP(), current_date())");

		pUserUpdateStatement = connection->prepareStatement(
			"UPDATE user SET `last_login` = CURRENT_TIMESTAMP() "
			"WHERE `id` = ?");
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
	delete pWebAuthStatement;
	delete pUserCreationStatement;
	delete pUserUpdateStatement;
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
		
		pWebAuthStatement->setString(1, pckt->email());
		pWebAuthStatement->setString(2, pckt->salt());
		pWebAuthStatement->setString(3, pckt->hashed_password());
		pWebAuthStatement->setBigInt(4, std::to_string(pckt->userid()));

		try {
			pWebAuthStatement->execute();
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

	// Create user at user table
	int creationResult = CreateUser(pckt);
	if (creationResult != 0) {
		return -1;
	}
	
	// Creation Successfull
	return 0;
}

int ChatDB::CreateUser(Authentication::CreateAccountPacket* pckt) {
	std::string userId;
	try {
		sql::SQLString selectQuery = "SELECT id FROM web_auth WHERE email = '" + pckt->email() + "';";
		pResultSet = pStatement->executeQuery(selectQuery);
	}
	catch (sql::SQLException e) {
		printf("Failed to execute query: %s\n", e.what());
		return -1;
	}
	printf("Successfully retrieved %d rows from the database!\n", (int)pResultSet->rowsCount());
	

	if (pResultSet->rowsCount() == 1) {
		while (pResultSet->next()) {
			userId = std::to_string(pResultSet->getInt(1));

			pUserCreationStatement->setBigInt(1, userId);

			try {
				pUserCreationStatement->execute();
			}
			catch (sql::SQLException e) {
				printf("Failed to create an user: %s\n", e.what());
				return -1;
			}
		}
	}
	else {
		// Account doesn't exist!
		return -1;
	}
	return 0;
}

int ChatDB::UpdateUser(std::string id) {
	
	pUserUpdateStatement->setBigInt(1, id);

	try {
		pUserUpdateStatement->execute();
	}
	catch (sql::SQLException e) {
		printf("Failed to create an user: %s\n", e.what());
		return -1;
	}

	return 0;
}

int ChatDB::Login(Authentication::LoginPacket* pckt)
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

	if (pResultSet->rowsCount() == 1) {
		while (pResultSet->next()) {
			// You can use either numeric offsets...
			std::cout << "id = " << pResultSet->getInt(1); // getInt(1) returns the first column
			// ... or column names for accessing results.
			// The latter is recommended.
			std::cout << ", email = '" << pResultSet->getString("email") << "'" << std::endl;

			std::string currentId = std::to_string(pResultSet->getInt(1));
			std::string currentEmail = pResultSet->getString("email");
			std::string currentSal = pResultSet->getString("salt");
			std::string currentHashedPassword = pResultSet->getString("hashed_password");

			// Hashing salt + password
			std::string hashedPassword;
			SHA256 sha;
			// The packets password is not hashed
			// because it's comming from a login request
			sha.update(currentSal + pckt->hashed_password());
			uint8_t* digest = sha.digest();
			hashedPassword = SHA256::toString(digest);

			delete[] digest; // Don't forget to free the digest!

			std::cout << "hashedPassword: " << hashedPassword << std::endl;
			std::cout << "currentHashedPassword: " << currentHashedPassword << std::endl;

			if (hashedPassword.size() != currentHashedPassword.size()) {
				// Still wanna show INVALID CREDENTIALS
				return 1;
				break;
			}
			
			for (int i = 0; i < hashedPassword.size(); i++) {
				std::cout << "hashedPassword[i] -> " << hashedPassword[i];
				std::cout << " = currentHashedPassword[i] -> " << currentHashedPassword[i] << std::endl;

				if (hashedPassword[i] != currentHashedPassword[i]) {
					return 1;
					break;
				}
			}

			// Insert a login registration
			int userLoginResult = UpdateUser(currentId);
			if (userLoginResult != 0) {
				return -1;
			}

			// Login successfull
			return 0;
		}
	}
	else {
		// Account does not exist!
		// INVALID CREDENTIALS
		return 1;
	}
}
