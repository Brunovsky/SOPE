# SOPE simgrep

A simple version of GNU grep.

Supports multiple processes for directory traversal
and multiple threads to grep each file in a directory.


#### Compile

Simply run make


#### Usage

Run	one of

	./simgrep
	./simgrep --usage

to print a usage message.


### Testing

Run

	./simgrep-test.sh -nc pattern folder

to progress through the tests one by one in ./simgrep-test.sh.

You can pass ./simgrep flags (except the ones below).
You must pass the pattern and the folder.

At the moment it tests all 16 combinations of set options
	--execgrep
	-P
	-T
	--dir/--fts
The output logfiles go to registry/


#### Current situation

I've had a few segfaults on some of the simgrep-test runs. I haven't yet
tested this project versus valgrind to check for memory leaks, I might do
so in the future. You're welcome to do it yourself :P

folder/ is the test folder you cna use for simgrep-test.
