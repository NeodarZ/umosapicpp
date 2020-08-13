# UMoSApi

This is a second implementation in C++ of the [Python](https://git.neodarz.net/pro/umosapi.git/about/)
version of Unity Mongo Save Api principally for speed issue. This is a simple
API for save Unity object in Mongo database.

In fact it was initially made for an Unity project but because of the
simplicity of the API you can use with whatever you want. ;)

/!\ WARNING: For the moment there is no authentifaction system! So don't use it
over the internet or in an environment where unknow user can connect and also
make sure your network is correctly securised.

# Install

## Dependency

Install:

- Restbed
- json-c
- nlohmann/json
- mongocxx
- spdlog

### Why two json lib ?

Because json-c is more fast than nlohmann/json when I do the test, json-c lib
is only used for convert bson to json and vice versa. The nlohmann/json is
only used for simplicty when loading config file and generate swagger.json
file.

# Build

```
mkdir build
cd build
cmake ..
make
```

move `<project_root>/build/umosapi` binary where you want.

move `<project_root>/swagger-ui` folder where you want.

# Configuration

You can move the [example file](https://git.neodarz.net/pro/umosapicpp.git/tree/config.txt)
in `~/.config/umosapi/config.txt`.

The key `swaggerui` is the path of the `swagger-ui` folder. The other keys
speak by themself.

# Documentation

Juste launch `umosapi` and go here: [http://127.0.0.1:9080/doc/](http://127.0.0.1:9080/doc/)
