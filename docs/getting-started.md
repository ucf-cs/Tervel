---
layout: tervel_documentation
---

# Installing Tervel

## System Requirements

Supported platforms:

*   Ubuntu Linux 16.04

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

Browse to the test directory, pick a test, and have fun!

## Sanity Check: Running the tests

To make sure everything runs correctly, run some of the binaries located in:
`tests/*/Executables/`

## Next Steps

See the [User Manual](tervel-user-manual.html) for a more in-depth overview on how to perform Tervel tests.
See the [Tutorial](beginner-tutorial.html) for a quick walk-through on how to use Tervel to create a wait free and lock free stack.
Let us know if you have any questions!