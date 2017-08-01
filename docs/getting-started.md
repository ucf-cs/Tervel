---
layout: tervel_documentation
---

# Installing Tervel

## System Requirements

Supported platforms:

* Ubuntu Linux 16.04

## Downloading Tervel

Clone the Tervel from the git repo:

{% highlight bash %}
$ git clone {{ site.gitrepo }}
{% endhighlight %}

## Building Tervel

### Dependencies

* g++4.8.0 or greater
* [gflags](http://gflags.github.io/gflags/)

{% highlight bash %}
$ sudo apt-get install libgflags-dev
{% endhighlight %}

### Building

{% highlight bash %}
$ cd tervel/tests
$ make all
{% endhighlight %}

## Running Tervel Tests

You may run the binaries located in `/tests/Executables/` under a subdirectory with a syntax similar to `version_NA_10000_10000`. Inside, you may run one of the binaries with a command such as

{% highlight bash %}
$ ./BIN.x
{% endhighlight %}

Where `BIN` is the name of the executable you wish to run. The test will execute with default parameters, which includes 0 threads. Therefore, it is normal to see output results containing little to no information. For an explanation on how to change these parameters, see the [User Manual](tervel-user-manual.html).

## Next Steps

See the [User Manual](tervel-user-manual.html) for a more in-depth overview on how to perform Tervel tests.
See the [Tutorial](beginner-tutorial.html) for a quick walk-through on how to use Tervel to create a wait free and lock free stack.
Let us know if you have any questions!