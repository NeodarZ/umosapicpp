#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "clara.hpp"
#include "shared.h"
#include "config.h"
#include "api/umosapi.h"

#include "service.hpp"

std::map<std::string, std::string> config;

int main(int argc, char *argv[]) {

    string config_path = "";
    const char *homedir;

    if ((homedir = getenv("XDG_CONFIG_HOME")) == NULL || (homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    config_path.append(homedir);
    config_path.append("/.config/umosapi/config.txt");

    bool showHelp = false;
    config["port"] = "9080";
    int thr = 2;
    auto cli = clara::detail::Help(showHelp)
             | clara::detail::Opt( config_path, "config" )["-c"]["--config"]("Config file path. Default `~/.config/umosapi/config.txt`.")
             | clara::detail::Opt( config["port"], "port" )["-p"]["--port"]("Port to listen. Default: `9080`.")
             | clara::detail::Opt( thr, "treads" )["-t"]["--threads"]("Number of threads. Default: `2`.");
    auto result = cli.parse( clara::detail::Args( argc, argv ) );
    if( !result )
    {
        std::cerr << "ERROR: Error in command line: " << result.errorMessage() << std::endl;
        std::cerr << cli << std::endl;
        exit(1);
    }

    if ( showHelp ) {
        std::cerr << cli << std::endl;
        exit(1);
    }

    //Address addr(Ipv4::any(), Port(config["port"]));

    cout << "INFO: Using " << std::thread::hardware_concurrency() << " cores";
    cout << " - " << thr << " threads" << endl;
    cout << "INFO: Listen on 0.0.0.0:" << config["port"] << endl;

    cout << "INFO: Using config file '" << config_path << "'" << endl;

    if (!std::filesystem::exists(config_path)) {
        cout << "ERROR: Error fatal : config file '" << config_path << "' not found" << endl;
        cout << "ERROR: config.txt is search here: ~/.config/umosapi/config.txt" << endl;
        exit (EXIT_FAILURE);
    }

    load_config(config_path);

    //cout << "Using swaggerui " << config["swaggerui"] << " path" << endl;
    cout << "INFO: No support for swagger for the moment" << endl;
    cout << "INFO: Using mongoURI " << config["mongoURI"] << endl;
    /*

    UmosapiService umosapi(addr);

    umosapi.init(thr);
    umosapi.start(config["swaggerui"]);
    */

    UmosapiService::Api umosapi;
    umosapi.init();
    umosapi.start(std::stoi(config["port"]), thr);

    return EXIT_SUCCESS;

}
