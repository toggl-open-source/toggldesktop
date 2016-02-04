
# Toggl Desktop Library Documentation

## Introduction

The library is the heart of the desktop applications on all platforms. It's main purpose is to handle the heavy lifting with data and networking. Also it is designed so that it actually does some work for the UI. the library triggers different UI events and releaves the UI to keep track of application states.

## Folder structure

All the library source files are in the `src` directory. The library project is built for each platform separatee according to project files that are located in `src/lib/{$platform}`. For example `src/lib/osx` for OS X. If you are adding new files to the library then the project files should be updated to reference the new files.

## Event flows

_Will add flow charts here to describe the usual event flows_

### App start flow

### Saving and Syncing flow

## Files in the library

Files in the library are described in depth in the following page [Files in the library](files.md)

## Third party libraries used

### Poco

[http://pocoproject.org/](http://pocoproject.org/)
Poco is used in the heart of the library for all networking related needs. 


Description from their homepage:
```
Modern, powerful open source C++ class libraries for building network- and internet-based applications that run on desktop, server, mobile and embedded systems.
```

Poco is used under Boost Software License - Version 1.0. Full licence is available at [https://github.com/pocoproject/poco/blob/develop/LICENSE](https://github.com/pocoproject/poco/blob/develop/LICENSE)

### OpenSSL

[https://www.openssl.org/](https://www.openssl.org/)

OpenSSL is used to make secure connections between servers and the library.

Description from their homepage:
```
The OpenSSL Project is a collaborative effort to develop a robust, commercial-grade, full-featured, and Open Source toolkit implementing the Transport Layer Security (TLS) and Secure Sockets Layer (SSL) protocols as well as a full-strength general purpose cryptography library. The project is managed by a worldwide community of volunteers that use the Internet to communicate, plan, and develop the OpenSSL toolkit and its related documentation.
```

OpenSSL is used under licence described in [https://github.com/openssl/openssl/blob/master/LICENSE](https://github.com/openssl/openssl/blob/master/LICENSE)


### Lua

[http://www.lua.org/](http://www.lua.org/)

Lua is mostly used for writing tests on the library.

Description from their homepage:
```
Lua is a powerful, fast, lightweight, embeddable scripting language.

Lua combines simple procedural syntax with powerful data description constructs based on associative arrays and extensible semantics. Lua is dynamically typed, runs by interpreting bytecode for a register-based virtual machine, and has automatic memory management with incremental garbage collection, making it ideal for configuration, scripting, and rapid prototyping.
```

Lua is used under MIT Licende. Full licence available at [http://www.lua.org/license.html](http://www.lua.org/license.html)

### JsonCpp

[https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)

JsonCpp is used to parse date to and from Json format. All of the messages exchanged between the library and the servers are in JSON format.

Description from their homepage:
```
JsonCpp is a C++ library that allows manipulating JSON values, including serialization and deserialization to and from strings. It can also preserve existing comment in unserialization/serialization steps, making it a convenient format to store user input files.
```

JsonCpp is used under licence described in [https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE)