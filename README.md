# UMoSApi

This is a second implementation in C++ of the [Python](https://git.neodarz.net/pro/umosapi.git/about/)
version of Unity Mongo Save Api. This is a simple API for save Unity object in
Mongo database.

# Install

## Dependency

Install:

- Pistache
- json-c
- nlohmann/json
- mongocxx

### Why two json lib ?

Because json-c is more fast than nlohmann/json when I do the test, json-c lib
is only used for convert bson to json and vice versa. The nlohmann/json is
only used for simplicty when loading config file.

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

Juste launch `umosapi` and go here: [http://127.0.0.1:9080/doc](http://127.0.0.1:9080/doc)
