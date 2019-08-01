# UMoSApi

This is a second implementation in C++ of the [Python](https://git.neodarz.net/pro/umosapi.git/about/)
version of Unity Mongo Save Api. This is a simple API for save Unity object in
Mongo database.

# Install

## Dependency

Install:

- Pistache
- json-c
- mongocxx

# Build

```
mkdir build
cd build
cmake ..
make
```

move `umosapi` where you want.
move `swagger-ui` where you want.

# Configuration

You can move example file in `~/.config/umosapi/config.json`.

The key `swaggerui` is the path of the `swagger-ui` folder.

# Documentation

Juste launch `umosapi` and go here: [http://127.0.0.1:9080/doc](http://127.0.0.1:9080/doc)
