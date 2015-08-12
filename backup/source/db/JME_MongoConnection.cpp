#include "JME_MongoConnection.h"
#include "JME_GLog.h"

namespace JMEngine
{
	namespace db
	{


		bool JME_MongoConnection::connect( const string& addr, string* err )
		{
			return _conn.connect(addr, *err);
		}

		bool JME_MongoConnection::auth( const string& dbName, const string& userName, const string& pwd, string* err, bool digestPassword /*= true*/ )
		{
			return _conn.auth(dbName, userName, pwd, *err, digestPassword);
		}

		void JME_MongoConnection::selectJson( const string& dbName, const mongo::BSONObj& key, Json::Value* res, const mongo::BSONObj* fileds /*= NULL*/ )
		{
			mongo::BSONObj b = _conn.findOne(dbName, key, fileds);

			if(b.isEmpty())
				return;

			Json::Reader reader;
			reader.parse(b.jsonString(), *res);
		}

		void JME_MongoConnection::insertBsonObj( const string& dbName, const mongo::BSONObj& obj )
		{
			_conn.insert(dbName, obj);
		}

		void JME_MongoConnection::updateBsonVal( const string& dbName, const mongo::BSONObj& key, const mongo::BSONObj& val, bool upsert /*= true*/, bool multi /*= false*/ )
		{
			_conn.update(dbName, key, val, upsert, multi);
			string err = _conn.getLastError();

			if (!err.empty())
			{
				LogE << err << LogEnd;
			}
		}

		int JME_MongoConnection::createAutoIncId( const string& dbName, const string& key, int init /*= 1*/ )
		{
			mongo::BSONObj b;  

			mongo::BSONObjBuilder bj;
			bj.append("findAndModify","AutoIncrement");  
			bj.append("query",BSON("_id"<<key));
			bj.append("update",BSON("$inc"<<BSON("CurrentIdValue"<<1)));  
			bj.append("upsert", true);

			_conn.runCommand(dbName, bj.obj(), b);  
			string err = _conn.getLastError();  
			if(!err.empty())  
				return 0;  

			mongo::BSONElement bel = b.getField("value");  
  
			if(bel.eoo()) 
				return 0;  

			mongo::BSONType t = bel.type();

			if (t == mongo::jstNULL)
				return init;

			if(bel.type() == mongo::NumberDouble)  
				return (int)bel["CurrentIdValue"].Double() + 1;  
			else  
				return bel["CurrentIdValue"].Int() + 1; 
		}

		void JME_MongoConnection::removeBson( const string& dbName, const mongo::BSONObj& key, bool justOne /*= true*/ )
		{
			_conn.remove(dbName, key, justOne);
			string err = _conn.getLastError();
			if ( !err.empty() )
			{
				LogE << err << LogEnd;
			}
		}
	}
}