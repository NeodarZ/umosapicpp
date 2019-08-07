#include "config.h"
#include "shared.h"

void load_config(std::string config_path) {
std::ifstream is_file(config_path);
    std::string line;
    while( std::getline(is_file, line) )
    {
      std::istringstream is_line(line);
      std::string key;
      if( std::getline(is_line, key, '=') )
      {
          std::string value;
          if( std::getline(is_line, value) )
             config[key] = value;
        }
    }
    is_file.close();

    std::string mongoURI = "mongodb://";


    if (config["mongo_db"] == "") {
        config["mongo_db"] = "umosapi";
    }

    if (config["mongo_user"] != "") {
        mongoURI.append(config["mongo_user"] + ":");
    }

    if (config["mongo_password"] != "") {
        mongoURI.append(config["mongo_password"] + "@");
    }

    if (config["mongo_host"] == "") {
        config["mongo_host"] = "127.0.0.1";
    }
    mongoURI.append(config["mongo_host"]);

    if (config["mongo_port"] == "") {
        config["mongo_port"] = "umosapi";
    }
    mongoURI.append(":" + config["mongo_port"]);

    if (config["swaggerui"] == "") {
        config["swaggerui"] = "/srv/http/swagger-ui";
    }

    config["mongoURI"] = mongoURI;

}
