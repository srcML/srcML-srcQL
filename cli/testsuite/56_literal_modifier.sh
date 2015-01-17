#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# test literal and modifier combinations

define output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"/>
	STDOUT

define foutput <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++" filename="sub/a.cpp"/>
	STDOUT

createfile sub/a.cpp ""

src2srcml -l C++ --literal --modifier < sub/a.cpp
check 3<<< "$output"

src2srcml --literal --modifier sub/a.cpp
check 3<<< "$foutput"

src2srcml -l C++ --literal --modifier -o sub/a.cpp.xml < sub/a.cpp
check sub/a.cpp.xml 3<<< "$output"
rmfile sub/a.cpp.xml

src2srcml --literal --modifier sub/a.cpp -o sub/a.cpp.xml
check sub/a.cpp.xml 3<<< "$foutput"
rmfile sub/a.cpp.xml

src2srcml -l C++ --modifier --literal < sub/a.cpp
check 3<<< "$output"

src2srcml --modifier --literal sub/a.cpp
check 3<<< "$foutput"

src2srcml -l C++ --modifier --literal -o sub/a.cpp.xml < sub/a.cpp
check sub/a.cpp.xml 3<<< "$output"
rmfile sub/a.cpp.xml

src2srcml --modifier --literal sub/a.cpp -o sub/a.cpp.xml
check sub/a.cpp.xml 3<<< "$foutput"
rmfile sub/a.cpp.xml