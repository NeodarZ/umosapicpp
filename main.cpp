#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "clara.hpp"
#include "shared.h"
#include "config.h"
#include "api/umosapi.h"


std::map<std::string, std::string> config;

int main(int argc, char *argv[]) {

    std::string config_path = "";
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

    std::cout << "INFO: Using " << std::thread::hardware_concurrency() << " cores";
    std::cout << " - " << thr << " threads" << std::endl;
    std::cout << "INFO: Listen on 0.0.0.0:" << config["port"] << std::endl;

    std::cout << "INFO: Using config file '" << config_path << "'" << std::endl;

    if (!std::filesystem::exists(config_path)) {
        std::cout << "ERROR: Error fatal : config file '" << config_path << "' not found" << std::endl;
        std::cout << "ERROR: config.txt is search here: ~/.config/umosapi/config.txt" << std::endl;
        exit (EXIT_FAILURE);
    }

    load_config(config_path);

    std::cout << "Using swaggerui " << config["swaggerui"] << " path" << std::endl;
    std::cout << "INFO: No support for swagger for the moment" << std::endl;
    std::cout << "INFO: Using mongoURI " << config["mongoURI"] << std::endl;

    UmosapiService::Api umosapi;
    umosapi.init();
    umosapi.start(std::stoi(config["port"]), thr);

    return EXIT_SUCCESS;

}
