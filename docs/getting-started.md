---
layout: tervel_documentation
---

# Installing Tervel

## System Requirements

Supported platforms:

*   Ubuntu Linux, Mac OS X


## Downloading Tervel

Clone the Tervel from the git repo:

{% highlight bash %}
$ git clone {{ site.gitrepo }}
{% endhighlight %}

## Building Tervel

### Dependencies

* g++4.8.0 or greater

* [gflags](http://gflags.github.io/gflags/)

### Building

{% highlight bash %}
$ cd tervel/tests
$ ./compile.sh
{% endhighlight %}



## Running Tervel Tests

Browse to the test directory, pick a test, and have fun!

## Sanity Check: Running the tests

To make sure everything runs correctly, run some of the binaries located in:
`tests/*/Executables/`

## Next Steps

Now you can added Tervel containers and algorithms into your own applications.
See [user manual](tervel-user-manual.html) for more information.
Let us know if you have any questions!
