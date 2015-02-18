#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# test extract_archive_unit

define input <<- 'INPUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:lc="http://www.sdml.info/srcML/linecol" revision="REVISION" language="C++">

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="REVISION" language="C" directory="sub" filename="a.cpp" mytag="foo"><expr_stmt lc:line="1"><expr><name>a</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="REVISION" language="Java" directory="sub" filename="b.cpp" mytag="foo"><expr_stmt lc:line="1"><expr><name>b</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="REVISION" language="C++" directory="sub" filename="c.cpp" mytag="foo"><expr_stmt lc:line="1"><expr><name>c</name></expr>;</expr_stmt>
	</unit>

	</unit>
	INPUT

define file1 <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" xmlns:lc="http://www.sdml.info/srcML/linecol" revision="REVISION" language="C" directory="sub" filename="a.cpp" mytag="foo"><expr_stmt lc:line="1"><expr><name>a</name></expr>;</expr_stmt>
	</unit>
	STDOUT

define file2 <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" xmlns:lc="http://www.sdml.info/srcML/linecol" revision="REVISION" language="Java" directory="sub" filename="b.cpp" mytag="foo"><expr_stmt lc:line="1"><expr><name>b</name></expr>;</expr_stmt>
	</unit>
	STDOUT

define file3 <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" xmlns:lc="http://www.sdml.info/srcML/linecol" revision="REVISION" language="C++" directory="sub" filename="c.cpp" mytag="foo"><expr_stmt lc:line="1"><expr><name>c</name></expr>;</expr_stmt>
	</unit>
	STDOUT

srcml2src -X --unit "1" - <<< "$input"

check 3<<< "$file1"

srcml2src -X --unit "1" <<< "$input"

check 3<<< "$file1"

srcml2src -X --unit "2" - <<< "$input"

check 3<<< "$file2"

srcml2src -X --unit "2" <<< "$input"

check 3<<< "$file2"

srcml2src -X --unit "3" - <<< "$input"

check 3<<< "$file3"

srcml2src -X --unit "3" <<< "$input"

check 3<<< "$file3"

srcml -X --unit "1" --show-filename <<< "$input"

check 3<<< "a.cpp"

srcml -X --unit "2" --show-filename <<< "$input"

check 3<<< "b.cpp"

srcml -X --unit "1" --show-directory <<< "$input"

check 3<<< "sub"

srcml -X --unit "2" --show-directory <<< "$input"

check 3<<< "sub"

srcml -X --unit "1" --show-language <<< "$input"

check 3<<< "C"

srcml -X --unit "2" --show-language <<< "$input"

check 3<<< "Java"