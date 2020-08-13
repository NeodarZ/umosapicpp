#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "clara.hpp"

#include "shared.h"
#include "logging.h"
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

    spdlog::info("Using {} cores - {} threads", std::thread::hardware_concurrency(), thr);
    spdlog::info("Listen on 0.0.0.0:{}", config["port"]);

    spdlog::info("Using config file '{}'", config_path);

    if (!std::filesystem::exists(config_path)) {
        spdlog::error("Error fatal: config file '{}' not found", config_path);
        spdlog::error("config.txt is searched here: ~/.config/umosapi.config.txt");
        exit (EXIT_FAILURE);
    }

    load_config(config_path);

    spdlog::info("Using swaggerui {} path", config["swaggerui"]);
    spdlog::warn("No support for swagger for the moment");
    spdlog::info("Using mongoURI {}", config["mongoURI"]);

    UmosapiService::Api umosapi;
    umosapi.init();
    umosapi.start(std::stoi(config["port"]), thr);

    return EXIT_SUCCESS;

}
