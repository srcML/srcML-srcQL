#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# nested file input
define nested <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0">

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>b</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>b</name></expr>;</expr_stmt>
	</unit>

	</unit>
	STDOUT

define nested_unit_out <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" revision="0.8.0">

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>b</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"><expr_stmt><expr><name>b</name></expr>;</expr_stmt>
	</unit>

	</unit>
	STDOUT

define nested_exprstmt_out <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" revision="0.8.0">

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++" item="1" location="/src:expr_stmt[1]"><expr_stmt><expr><name>a</name></expr>;</expr_stmt></unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++" item="1" location="/src:expr_stmt[1]"><expr_stmt><expr><name>b</name></expr>;</expr_stmt></unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++" item="1" location="/src:expr_stmt[1]"><expr_stmt><expr><name>a</name></expr>;</expr_stmt></unit>

	<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++" item="1" location="/src:expr_stmt[1]"><expr_stmt><expr><name>b</name></expr>;</expr_stmt></unit>

	</unit>
	STDOUT

createfile sub/a.cpp.xml "$nested"

# /src:unit
srcml2src --apply-root --xpath=/src:unit sub/a.cpp.xml
check 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=/src:unit < sub/a.cpp.xml
check 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=/src:unit sub/a.cpp.xml -o sub/b.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=/src:unit -o sub/b.cpp.xml sub/a.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=/src:unit -o sub/b.cpp.xml < sub/a.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_unit_out"

# //src:unit
srcml2src --apply-root --xpath=//src:unit sub/a.cpp.xml
check 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=//src:unit < sub/a.cpp.xml
check 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=//src:unit sub/a.cpp.xml -o sub/b.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=//src:unit -o sub/b.cpp.xml sub/a.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_unit_out"

srcml2src --apply-root --xpath=//src:unit -o sub/b.cpp.xml < sub/a.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_unit_out"

# //src:expr_stmt
srcml2src --apply-root --xpath=//src:expr_stmt sub/a.cpp.xml
check 3<<< "$nested_exprstmt_out"

srcml2src --apply-root --xpath=//src:expr_stmt < sub/a.cpp.xml
check 3<<< "$nested_exprstmt_out"

srcml2src --apply-root --xpath=//src:expr_stmt sub/a.cpp.xml -o sub/b.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_exprstmt_out"

srcml2src --apply-root --xpath=//src:expr_stmt -o sub/b.cpp.xml sub/a.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_exprstmt_out"

srcml2src --apply-root --xpath=//src:expr_stmt -o sub/b.cpp.xml < sub/a.cpp.xml
check sub/b.cpp.xml 3<<< "$nested_exprstmt_out"

#
#srcml2src --apply-root --xpath=/src:unit/src:unit[1]/src:expr_stmt srcml_nested xpath_single_expr_stmt)
#
#srcml2src --apply-root --xpath=/src:unit/src:unit[1]/src:expr_stmt sub/a.cpp.xml xpath_single_expr_stmt)
#
#srcml2src --apply-root --xpath=/src:unit/src:unit[1]/src:expr_stmt -o sub/b.cpp.xml srcml_nested
#
#srcml2src --apply-root --xpath=/src:unit/src:unit[1]/src:expr_stmt sub/a.cpp.xml -o sub/b.cpp.xml ""
#
#
