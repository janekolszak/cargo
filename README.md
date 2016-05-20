# Cargo
Cargo is a set of C++ libraries that provide support in data parsing and passing via Unix sockets.

# Installation
For now it's only possible to build and install all the Cargo libraries.

```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
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
```
cd utils
./build_in_docker.sh
```
You can also directly use the provided Dockerfile.

Some test won't pass in a Linux container, don't worry.

# Copyright
Cargo's development was started in the Samsung R&D Insitute Poland and now is maintained by the open-source community.