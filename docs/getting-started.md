---
layout: tervel_documentation
---

# Installing Tervel

## System Requirements

Supported platforms:

* Ubuntu Linux 16.04

## Downloading Tervel

Clone the Tervel from the git repo:

```bash
$ git clone {{ site.gitrepo }}
```

## Building Tervel

An already set-up runtime environment is already available for Tervel online at [IEEE Xplore](http://ieeexplore.ieee.org/document/7363668/algorithms).

### Dependencies

* g++4.8.0 or greater
* [gflags](http://gflags.github.io/gflags/)

```bash
$ sudo apt-get install libgflags-dev
```

### Building

```bash
$ cd tervel/tests
$ make all
```

## Running Tervel Tests

You may run the binaries located in `/tests/executables/` under a subdirectory with a syntax similar to `version_NA_10000_10000`. Inside, you may run one of the binaries with a command such as

```bash
$ ./BIN.x
```

Where `BIN` is the name of the executable you wish to run. The test will execute with default parameters, which includes 0 threads. Therefore, it is normal to see output results containing little to no information. For an explanation on how to change these parameters, see the [User Manual](tervel-user-manual.html).

## Next Steps

See the [User Manual](tervel-user-manual.html) for a more in-depth overview on how to perform Tervel tests.
See the [Tutorial](beginner-tutorial.html) for a quick walk-through on how to use Tervel to create a wait free and lock free stack.
Let us know if you have any questions!