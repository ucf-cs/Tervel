---
layout: tervel_documentation
---

# Getting Started with Tervel

## Setup

Clone the [Git repo]({{ site.gitrepo }}) and run the
provided compile script.

[installation guide](install.html).

{% highlight bash %}
$ git clone {{ site.gitrepo }}
$ cd tervel
$ ./compile.sh
{% endhighlight %}

`./compile.sh` compiles the tests in the `test` directory.


## Sanity Check: Running the tests

To make sure everything is runs correctly, run some of the binaries in:
`tests/*/Executables/`

## Next Steps

Now you can added Tervel containers and algorithms into your own applications.
See [user manual](tervel-user-manual.html)
for more information.
Let us know
if you have any questions!
