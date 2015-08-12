#ifndef JME_MongoConnection_h__
#define JME_MongoConnection_h__

#include "client/dbclient.h"
#include "json/json.h"

#include <string>

using namespace std;
namespace JMEngine
{
	namespace db
	{
		class JME_MongoConnection
		{
		public:

			//************************************
			// Method:    connect
			// FullName:  JMEngine::db::JME_MongoConnection::connect
			// Access:    public 
			// Returns:   bool, succeed ==> true, failed ==> false
			// Qualifier:
			// Parameter: const string & addr
			// Parameter: string * err
			//************************************
			bool connect(const string& addr, string* err);


			//************************************
			// Method:    auth
			// FullName:  JMEngine::db::JME_MongoConnection::auth
			// Access:    public 
			// Returns:   bool, succeed ==> true, failed ==> false
			// Qualifier:
			// Parameter: const string & dbName
			// Parameter: const string & userName
			// Parameter: const string & pwd
			// Parameter: string * err
			// Parameter: bool digestPassword
			//************************************
			bool auth(const string& dbName, const string& userName, const string& pwd, string* err, bool digestPassword = true);

			//************************************
			// Method:    selectJson
			// FullName:  JMEngine::db::JME_MongoConnection::selectJson
			// Access:    public 
			// Returns:   void
			// Qualifier:
			// Parameter: const string & dbName
			// Parameter: const mongo::BSONObj & key
			// Parameter: Json::Value * res
			// Parameter: const mongo::BSONObj * fileds
			//************************************

			void selectJson(const string& dbName, const mongo::BSONObj& key, Json::Value* res, const mongo::BSONObj* fileds = NULL);
			//************************************
			// Method:    insertBsonObj
			// FullName:  JMEngine::db::JME_MongoConnection::insertBsonObj
			// Access:    public 
			// Returns:   void
			// Qualifier:
			// Parameter: const string & dbName
			// Parameter: const mongo::BSONObj & obj
			//************************************

			void insertBsonObj(const string& dbName, const mongo::BSONObj& obj);
			//************************************
			// Method:    updateBsonVal
			// FullName:  JMEngine::db::JME_MongoConnection::updateBsonVal
			// Access:    public 
			// Returns:   void
			// Qualifier:
			// Parameter: const string & dbName
			// Parameter: const mongo::BSONObj & key
			// Parameter: const mongo::BSONObj & val
			// Parameter: bool upsert
			// Parameter: bool multi
			//************************************
			void updateBsonVal(const string& dbName, const mongo::BSONObj& key, const mongo::BSONObj& val, bool upsert = true, bool multi = false);
			
			//************************************
			// Method:    removeBson
			// FullName:  JMEngine::db::JME_MongoConnection::removeBson
			// Access:    public 
			// Returns:   void
			// Qualifier:
			// Parameter: const string & dbName
			// Parameter: const mongo::BSONObj & key
			// Parameter: bool justOne
			//************************************
			void removeBson(const string& dbName, const mongo::BSONObj& key, bool justOne = true);
			
			//************************************
			// Method:    createAutoIncId
			// FullName:  JMEngine::db::JME_MongoConnection::createAutoIncId
			// Access:    public 
			// Returns:   int, 0 ==> error
			// Qualifier:
			// Parameter: const string & dbName
			// Parameter: const string & key
			// Parameter: int init
			//************************************
			int createAutoIncId(const string& dbName, const string& key, int init = 1);
		private:
			mongo::DBClientConnection _conn;
		};
	}
}
#endif // JME_MongoConnection_h__
