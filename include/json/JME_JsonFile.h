#ifndef JME_Json_h__
#define JME_Json_h__

#include <string>
#include <vector>
#include <map>
#include <json/json.h>

using namespace std;
namespace JMEngine
{
	namespace file
	{
		std::string load_jsonfile(const std::string& file_name);
		Json::Value load_jsonfile_val(const std::string& file_name);
		int load_jsonfiles_from_dir(const std::string& files_path, vector<string>& vec );
		int load_jsonfiles_from_dir(const std::string& files_path, vector<Json::Value>& vec );
		int load_jsonfiles_from_dir(const std::string& files_path, map<int, Json::Value>& m );
	}
}



#endif // JME_Json_h__
