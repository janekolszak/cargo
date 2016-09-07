# Cargo [![Build Status](https://travis-ci.org/janekolszak/cargo.svg?branch=master)](https://travis-ci.org/janekolszak/cargo)
Cargo is a set of C++ libraries that aim to ease Linux daemon development. It supports:
- C++ `struct` serialization and deserialization from various formats
- Data validation
- IPC mechanism
- Epoll wrapper
- Logger

All libraries are available in **deb** and **rpm** packages for x86 arch in the *release tab*.
You can also build it all from source.

# Libraries:

#### cargo
This package includes only header files that implement the core functionality of the library. Here are the macros that you use to make your structure *visitable*. You use them to make your structure serializable with other cargo-* libraries.

#### cargo-json
Serialization/deserialization from JSON text format

#### cargo-fd
Serialization/deserialization from Unix file descriptor:
- Unix sockets
- Internet sockets
- File sockets
- Pipes
- and anything you can *read* and *write* to

#### cargo-sqlite
Serialization/deserialization to a key-value store implemented in sqlite3

#### cargo-gvariant
Serialization/deserialization from glib's [GVariant](https://developer.gnome.org/glib/stable/glib-GVariant.html) :
- Unix sockets
- Internet sockets
- Files
- Pipes

#### cargo-validator
Extensible data validation. Validation rules are inside a `struct` declaration which improves readability.
It's possible to write custom functions for field validation.

#### cargo-epoll
A very useful C++ wrapper for Linux epoll mechanism, with already integrated with:
- timerfd
- signalfd
- eventfd
and a mechanism for adding your own fd to epoll.

You can dispatch *events*:
- in a dedicated, separate thread
- in a glib loop
- in your own thread, using the EventPoll's API

#### cargo-ipc
Feature full IPC mechanism that sends *cargo* structures to make remote calls via sockets. Some features:
- Synchronous and asynchronous method calls
- Signals
- Error passing

#### cargo-logger
Small, extensible logger, with backends for:
- stdin/stdout
- systemd-journal
- syslog
- file
- file with flushing after every log

#### cargo-sqlite-json
Reading default values on a structure from a JSON file.

#### cargo-utils
A static library and header files with some useful code that some cargo libs use.


# Installation

#### DEB and RPM for 64b
You can download RPMs and DEBs pre-built for x86 architecture from the Release tab.

#### Source
```bash
sudo apt-get install -y cmake clang gcc libboost-test1.58-dev libboost-system1.58-dev libboost-filesystem1.58-dev libglib2.0-dev uuid-dev libsystemd-dev libjson-c-dev libsqlite3-dev

mkdir build && cd build
cmake -DWITHOUT_SYSTEMD=1 -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j4
sudo cmake --build . --target install
```

You may also use rpmbuild, the .spec file is in the /packaging folder.

# Example

```cpp
#include <cargo/fields.hpp>
#include <cargo-gvariant/cargo-gvariant.hpp>
#include <cargo-json/cargo-json.hpp>
#include <cargo-sqlite/cargo-sqlite.hpp>
#include <cargo-sqlite-json/cargo-sqlite-json.hpp>
#include <cargo-fd/cargo-fd.hpp>
#include <iostream>
#include <cstdio>

struct Foo
{
    std::string bar = "plain-text";
    std::vector<int> tab = std::vector<int>{1, 2, 4, 8};
    double number = 3.14;

    CARGO_REGISTER
    (
        bar,
        tab,
        number
    )
};

int main()
{
    Foo foo;

    const std::string jsonString = cargo::saveToJsonString(foo);
    cargo::loadFromJsonString(jsonString, foo);

    const GVariant* gVariantPointer = cargo::saveToGVariant(foo);
    cargo::loadFromGVariant(gVariantPointer, foo);
    g_variant_unref(gVariantPointer);

    constexpr std::string jsonFile = "foo.json";
    cargo::saveToJsonFile(jsonFile, foo);
    cargo::loadFromJsonFile(jsonFile, foo);

    constexpr std::string kvDBPath = "kvdb";
    constexpr std::string key = "foo";
    cargo::saveToKVStore(kvDBPath, foo, key);
    cargo::loadFromKVStore(kvDBPath, foo, key);

    cargo::loadFromKVStoreWithJson(kvDBPath, jsonString, foo, key);
    cargo::loadFromKVStoreWithJsonFile(kvDBPath, jsonFile, foo, key);

    FILE* file = fopen("blob", "wb");
    if (!file)
    {
        return EXIT_FAILURE;
    }
    const int fd = ::fileno(file);
    cargo::saveToFD(fd, foo);
    ::fclose(file);
    file = ::fopen("blob", "rb");
    if(!file) {
        return EXIT_FAILURE;
    }
    cargo::loadFromFD(fd, foo);
    ::fclose(file);

    return 0;
}
```

# Build in a Docker container
You can build Cargo libs in a Docker container.
This way you don't need to fiddle with your host OS to build Cargo binaries.

To build cargo just run the prepared script (install docker and docker-compose before):
```bash
cd utils
./build_in_docker.sh
```
You can also directly use the provided Dockerfile.

Some test won't pass in a Linux container, don't worry.

# Copyright
Cargo's development was started in the Samsung R&D Insitute Poland and now is maintained by the open-source community.
