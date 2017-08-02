---
layout: tervel_documentation
---

# User Manual

## Makefile

There are three main Makefile commands.

{% highlight bash %}
$ make all
$ make allBuffer
$ make allTervel
{% endhighlight %}

`allBuffer` will compile all the tests for buffers, that is `buffer_tervel_wf`, `buffer_tervel_mcas_lf`, `buffer_lock_cg`, `buffer_linux_nb`, and `buffer_naive_cg`.

`allTervel` will compile all the tests for Tervel data structures, that is `queue_tervel_wf`, `queue_tervel_lf`, `mcas_tervel_wf`, `vector_tervel_wf`, `stack_tervel_lf`, `stack_tervel_wf`, `hashmap_tervel_wf`, and `hashmap_nodel_tervel_wf`.

`all` will compile all of the binaries mentioned above.

There is one additional test not compiled by the above scripts because it requires a separate library. That would be `buffer_tbb_fg` and if you wish to run this test, start by installing Intel Thread Building Blocks:

{% highlight bash %}
$ sudo apt-get install libtbb-dev
{% endhighlight %}

Then compile the binaries for it by running

{% highlight bash %}
$ make tbb
{% endhighlight %}

Another test using an external library was created for this project, a test for the Tsigas queue. At the time of this project's creation, this data structure was supported in the CDS library. It has since been removed due to an ABA problem, therefore it is now depricated as a Tervel test. If you wish to run this test anyway, the queue is available in [libcds 2.1.0](https://github.com/khizmax/libcds/releases/tag/v2.1.0).

You will also have to uncomment the lines in the Makefiles to do with `tsigasBuffer`. See the `Makefile` and `Makefile.ringbuffer`. Afterwards, you should be able to compile using.

{% highlight bash %}
$ make cds
{% endhighlight %}

There are also four variables you may pass into the makefile:
* `delay`: how long a thread waits before checking the announcement table, 10000 by default, which is quite high

* `limit`: how many times an op fails before posing an announcement, 10000 by default, which is quite high

* `version`: just something to help the user name their directories within `/executables/`, NA by default

^

As an example of how to use the flags:

{% highlight bash %}
$ make allTervel delay=10 limit=10 version=1
{% endhighlight %}

Which will compile all Tervel-related test executables under `/executables/version_1_10_10/`

## Running Tervel Tests

You may run the binaries located in `/tests/Executables/` under a subdirectory with a syntax similar to `version_NA_10000_10000`. Inside, you may run one of the binaries with a command syntax such as

{% highlight bash %}
$ ./BIN.x F T OP
{% endhighlight %}

Where
* `BIN` is the name of the binary you wish to run,

* `F` is the flags

* `T` is the number of threads, and

* `OP` is the ratio of occurrences for each of the data structure's operations.

^
Leaving any of these command line parameters blank will default their values to zero.

There is only one flag that is required, and that is `-num_threads` with a trailing integer that must be greater than or equal to the sum of all `T`s. Therefore, a full sample command for a test would be:

{% highlight bash %}
$ ./stack_tervel_lf.x -num_threads 3  2 1 0  1 0 1
{% endhighlight %}

Which will run the lock-free Tervel stack test with three threads, the first group consisting of two threads performing 100% inserts and the other group just one thread performing 100% removes. More flags are listed below.

As another example:

{% highlight bash %}
$ ./buffer_tervel_wf.x -num_threads 2 2 2 1
{% endhighlight %}

Will run the wait-free Tervel buffer test with two threads, each performing approximately 66% enqueue and 33% dequeue.

### Data Structure Operations

A list of all of the Tervel data structure and algorithm operations in the order that they must be received as command line parameters for the test binaries. Providing an integer for at least one but not all of these ops for their appropriate test will cause the program to list the ops for you.

Buffer/Queue
* enqueue
* dequeue

HashMap
* find
* insert
* update
* delete

MCAS
* mcas
* read

Stack
* pop
* push

Vector
* at
* cas
* pushback
* popBack
* size
* eraseAt
* insertAt

### Universal Flags

A list of universal flags that will work for all binaries. `-num_threads` is required in order to produce any useful output.

| `--help` | type: `bool` | default: `false` | Show all flags.
| `-iter_dist` | type: `bool` | default: `false` | If true, then it iterates between commands.
| `-seq_test` | type: `bool` | default: `false` | If true, then a sequential test is performed.
| `-verbose` | type: `bool` | default: `false` | If true, then verbose output is used.
| `-disable_thread_join` | type: `bool` | default: `false` | Enables skipping of the thread join command, useful if deadlock may occur.
| `-execution_time` | type: `uint64` | default: `5` | The amount of time to run the tests in seconds.
| `-main_sleep` | type: `uint64` | default: `0` | Causes the main thread to sleep before signaling go. Useful for allowing monitors to be attached.
| `-num_threads` | type: `uint64` | default: `0` | The number of executing threads.

### Data Structure Flags

A list of flags for every data structure.

| Buffer
|-|
| `-capacity` | type: `int32` | default: `32768` | The capacity of the buffer.
| `-prefill` | type: int32 | default: 0 | The number of elements to place in the buffer on init.

| HashMap
|-|
| `-capacity` | type: `int32` | default: `32768` | The capacity of the HashMap - should be power of two.
| `-expansion_factor` | type: `int32` | default: `5` | The size by which the HashMap expands on collision. 2^expansion_factor = positions.
| `-prefill` | type: `int32` | default: `0` | The number of elements to place in the HashMap on init.

| MCAS
|-|
| `-array_length`| type: `int32` | default: `32` | The size of the region to test on.
| `-mcas_size` | type: `int32` | default: `2` | The number of words in an mcas operation.
| `-multipleObjects` | type: `bool` | default: `false` | Whether or not multiple disjoint mcas operations can occur.
| `-overlapping` | type: `bool` | default: `false` | Whether or not the mcas operations can be overlapping.

| Stack
|-|
| `-prefill` | type: `int32` | default: `0` | The number of elements to place in the stack on init.

| Vector
|-|
| `-capacity` | type: `int32` | default: `0` | The capacity of the vector.
| `-prefill` | type: `int32` | default: `0` | The number of elements to place in the vector on init.

## Python Scripts

For more comprehensive tests, the python files in `/tests/scripts/` should be used. Before continuing, ensure Python and likwid are installed. See [likwid's GitHub](https://github.com/RRZE-HPC/likwid). You should also make sure to run the `Makefile` first because the default scripts will check for binaries within `/executables/version_NA_10000_10000/`

You may run the script which will run multiple tests simply by.

{% highlight bash %}
$ python execute_test.py
{% endhighlight %}

Which will prompt you for a config file. Feel free to enter the provided `example.config` or create your own. The script will run multiple tests based on the `.config` file and place their output in `/scripts/logs/`.

The syntax of the config file should be fairly straightforward. At the top of the config file are a bunch of universal options that will carry for all tests the Python script will run. This includes:

* `description`: A string of text that will be echoed in the output .log files.
* `log_git`: A bool that determines if .log files should be created for the current status of the git branch.
* `log_directory`: A file path for where all of the output should be generated.
* `main_sleep_time`: An int for how many seconds the main thread will sleep before signaling go. Useful for allowing monitors to be attached.
* `exe_repetition`: An int for how many times you wish for the exact same execution configuration to run
* `exe_time`: An array of ints where each entry will coincide with a new test given that many seconds to execute
* `system_name`: A string to describe the machine the test is running on.
* `papi_flag`: Power API flags
* `misc_flags`: Any flags that don't fit into the other categories should go here.
* `thread_levels`: An array listing the different numbers of threads the tests should be ran with. Powers of two reccomended.
* `disable_thread_join`: A bool that enables skipping of the thread join command, useful if deadlock may occur.
* `verbose`: A bool that if true, verbose output is used.
* `exe_prefix`: Flags that must go before the executable. Usually used for likwid.

Then there are the flags for the tests. The 'tests' object itself is an array, so you may add more simply with ',[test body goes here]'
Within each test, there are more options:

* `name`: A string of text to describe this specific test.

* `executables`: An array of the executables that will run. These are the same executables compiled by the Makefile.

* `path`: The file path to the executables. By default '../executables/version_NA_10000_10000'. That final directory may be different for you based on what options were used when running the Makefile.

* `flags`: The flags that are specific to this executable's data structure. See previous section.

* `dist`: An array where each entry is a new configuration for op ratios. It follows the following syntax:

^
> `lambda t: None if t < T else "%d OP1 ... OPN %d OP1 ... OPN %d ..." %((t*D1), ...(t*DG))`

Where:
* `T` is the number of threads
* There are `G` occurrences of `%d` for `G` number of thread groups
* `OP` is each op's ratio
* `D` is a decimal value for the percentage of threads to be doing this particular thread group

For example, if the following dist entry occurred in a buffer test entry:
> `lambda t: None if t < 4 else "%d 1 0 %d 0 1" %((t*.25), (t*.75))`

Will be an execution where four or more threads have 25% of threads doing all enqueues and the other 75% doing all dequeues. The reason `T` is four is because a thread group configuration with less than four threads cannot be evenly split into a 25:75 thread group configuration. We can be assured that any amount of threads more than four will work if all entries in thread_levels are powers of two.