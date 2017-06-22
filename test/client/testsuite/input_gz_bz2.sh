#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# test on compressed files with .gz.bz2 extension
define src <<- 'STDOUT'

	a;
	STDOUT

define empty_output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" revision="REVISION"/>
	STDOUT

define foutput <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++" filename="archive/a.cpp.gz.bz2">
	<expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>
	STDOUT

define output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++">
	<expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>
	STDOUT

define archive_output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" revision="REVISION">

	<unit xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++" filename="archive/a.cpp.gz.bz2" hash="1a2c5d67e6f651ae10b7673c53e8c502c97316d6">
	<expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>

	</unit>
	STDOUT

xmlcheck "$archive_output"
xmlcheck "$foutput"
xmlcheck "$output"
xmlcheck "$empty_output"

createfile archive/a.cpp "$src"
gzip -c archive/a.cpp > archive/a.cpp.gz
bzip2 -c archive/a.cpp.gz > archive/a.cpp.gz.bz2

createfile list.txt "archive/a.cpp.gz.bz2"
gzip -c list.txt > list.txt.gz
bzip2 -c list.txt.gz > list.txt.gz.bz2

createfile empty.txt " "
gzip -c empty.txt > empty.txt.gz
bzip2 -c empty.txt.gz > empty.txt.gz.bz2


# src --> srcml
src2srcml archive/a.cpp.gz.bz2 -o archive/a.cpp.xml
check archive/a.cpp.xml "$foutput"

src2srcml archive/a.cpp.gz.bz2
check "$foutput"

src2srcml -l C++ < archive/a.cpp.gz.bz2
check "$output"

src2srcml -l C++ -o archive/a.cpp.xml < archive/a.cpp.gz.bz2
check archive/a.cpp.xml "$output"


# files from
src2srcml --files-from list.txt
check "$archive_output"

src2srcml --files-from list.txt.gz.bz2
check "$archive_output"

src2srcml --files-from list.txt -o archive/list.xml
check archive/list.xml "$archive_output"

src2srcml --files-from list.txt.gz.bz2 -o archive/compressed_list.xml
check archive/compressed_list.xml "$archive_output"


# files from empty
src2srcml --files-from empty.txt
check "$empty_output"

src2srcml --files-from empty.txt.gz.bz2
check "$empty_output"

src2srcml --files-from empty.txt -o archive/empty.xml
check archive/empty.xml "$empty_output"

src2srcml --files-from empty.txt.gz.bz2 -o archive/compressed_empty.xml
check archive/compressed_empty.xml "$empty_output"


rmfile list.txt
rmfile list.txt.gz
rmfile list.txt.gz.b2
rmfile empty.txt
rmfile empty.txt.gz
rmfile empty.txt.gz.b2
rmfile archive/a.cpp
rmfile archive/a.cpp.gz
rmfile archive/a.cpp.gz.bz2


# srcml --> src
srcml2src archive/a.cpp.xml
check "$src"

srcml2src archive/a.cpp.xml -o archive/a.cpp
check archive/a.cpp "$src"

srcml2src < archive/a.cpp.xml
check "$src"

srcml2src -o archive/a.cpp < archive/a.cpp.xml
check archive/a.cpp "$src"