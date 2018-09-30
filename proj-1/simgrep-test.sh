#!/bin/sh

waitenter () {
	echo
	echo "Done!"
	echo
	read key
	clear
}

dotests() {
	## BEGIN TESTS
	
	clear



	echo " Usage + Version"
	./simgrep

	waitenter



	echo " Usage"
	./simgrep --usage

	waitenter



	echo " Version"
	./simgrep --version

	waitenter



	export LOGFILENAME=registry/textfile000dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        FALSE          FALSE         DIR"
	./simgrep "$@" -r --color --dir

	waitenter



	export LOGFILENAME=registry/textfile000fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        FALSE          FALSE         FTS"
	./simgrep "$@" -r --color --fts

	waitenter



	export LOGFILENAME=registry/textfile001dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        FALSE          TRUE          DIR"
	./simgrep "$@" -r --color --dir -T

	waitenter



	export LOGFILENAME=registry/textfile001fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        FALSE          TRUE          FTS"
	./simgrep "$@" -r --color --fts -T

	waitenter



	export LOGFILENAME=registry/textfile010dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        TRUE           FALSE         DIR"
	./simgrep "$@" -r --color --dir -P

	waitenter



	export LOGFILENAME=registry/textfile010fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        TRUE           FALSE         FTS"
	./simgrep "$@" -r --color --fts -P

	waitenter



	export LOGFILENAME=registry/textfile011dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        TRUE           TRUE          DIR"
	./simgrep "$@" -r --color --dir -P -T

	waitenter



	export LOGFILENAME=registry/textfile011fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   FALSE        TRUE           TRUE          FTS"
	./simgrep "$@" -r --color --fts -P -T

	waitenter



	export LOGFILENAME=registry/textfile100dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         FALSE          FALSE         DIR"
	./simgrep "$@" -r --color --dir --execgrep

	waitenter



	export LOGFILENAME=registry/textfile100fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         FALSE          FALSE         FTS"
	./simgrep "$@" -r --color --fts --execgrep

	waitenter



	export LOGFILENAME=registry/textfile101dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         FALSE          TRUE          DIR"
	./simgrep "$@" -r --color --dir --execgrep -T

	waitenter



	export LOGFILENAME=registry/textfile101fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         FALSE          TRUE          FTS"
	./simgrep "$@" -r --color --fts --execgrep -T

	waitenter



	export LOGFILENAME=registry/textfile110dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         TRUE           FALSE         DIR"
	./simgrep "$@" -r --color --dir --execgrep -P

	waitenter



	export LOGFILENAME=registry/textfile110fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         TRUE           FALSE         FTS"
	./simgrep "$@" -r --color --fts --execgrep -P

	waitenter



	export LOGFILENAME=registry/textfile111dir.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         TRUE           TRUE          DIR"
	./simgrep "$@" -r --color --dir --execgrep -P -T

	waitenter



	export LOGFILENAME=registry/textfile111fts.txt
	echo " execgrep   multiprocess   multithreads   traversal"
	echo "   TRUE         TRUE           TRUE          FTS"
	./simgrep "$@" -r --color --fts --execgrep -P -T

	waitenter

	## END TESTS
}

TMP_LOGFILENAME=${LOGFILENAME}

mkdir -p registry

dotests "$@"

export LOGFILENAME=${TMP_LOGFILENAME}

exit 0
