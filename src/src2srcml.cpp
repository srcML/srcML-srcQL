/*
  src2srcml.cpp

  Copyright (C) 2002-2010  SDML (www.sdml.info)

  This file is part of the srcML translator.

  The srcML translator is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  The srcML translator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the srcML translator; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Main program to run the srcML translator.
*/

#include <cstring>
#include <sys/stat.h>
#include "version.h"
#include "srcmlapps.h"
#include "srcmlns.h"
#include "Options.h"
#include "project.h"
#include "Language.h"
#include "srcMLTranslator.h"
#include "URIStream.h"
#include <getopt.h>
//#include "libxml_curl_io.h"

int option_error_status(int optopt);

using namespace LanguageName;

const char* const NAME = "src2srcml";

const char* const DEBUG_FLAG = "debug";
const char DEBUG_FLAG_SHORT = 'g';

const char* const LITERAL_FLAG = "literal";

const char* const OPERATOR_FLAG = "operator";

const char* const MODIFIER_FLAG = "modifier";

const char* const POSITION_FLAG = "position";
const char* const TABS_FLAG = "tabs";
const char* const TABS_FLAG_FULL = "tabs=NUMBER";

const char* const INTERACTIVE_FLAG = "interactive";
const char INTERACTIVE_FLAG_SHORT = 'c';

const char* const CPP_MARKUP_ELSE_FLAG = "cpp_markup_else";

const char* const CPP_TEXTONLY_ELSE_FLAG = "cpp_text_else";

const char* const CPP_MARKUP_IF0_FLAG = "cpp_markup_if0";

const char* const CPP_TEXTONLY_IF0_FLAG = "cpp_text_if0";

const char* const EXPRESSION_MODE_FLAG = "expression";
const char EXPRESSION_MODE_FLAG_SHORT = 'e';

const char* const SELF_VERSION_FLAG = "self-version";

const char* const DEFAULT_XML_ENCODING = "UTF-8";

const char* const FILELIST_FLAG = "files-from";
const char* const FILELIST_FLAG_FULL = "files-from=INPUT";

const char* const XMLNS_FLAG = "xmlns";
const char* const XMLNS_DEFAULT_FLAG_FULL = "xmlns=URI";
const char* const XMLNS_FLAG_FULL = "xmlns:PREFIX=URI";

const char* const REGISTER_EXT_FLAG = "register";

const int DEFAULT_LANGUAGE = srcMLTranslator::LANGUAGE_CXX;

const char* const EXAMPLE_TEXT_FILENAME="foo.cpp";
const char* const EXAMPLE_XML_FILENAME="foo.cpp.xml";

const char FILELIST_COMMENT = '#';

bool process;

struct uridata {
  char const * const uri;
  int option;
};

uridata uris[] = {

  { SRCML_SRC_NS_URI,          0 },
  { SRCML_CPP_NS_URI,          OPTION_CPP },
  { SRCML_ERR_NS_URI,          OPTION_DEBUG },
  { SRCML_EXT_LITERAL_NS_URI,  OPTION_LITERAL },
  { SRCML_EXT_OPERATOR_NS_URI, OPTION_OPERATOR },
  { SRCML_EXT_MODIFIER_NS_URI, OPTION_MODIFIER },
  { SRCML_EXT_POSITION_NS_URI, OPTION_POSITION },
};

const char* urisprefix[] = {

  SRCML_SRC_NS_PREFIX_DEFAULT,
  SRCML_CPP_NS_PREFIX_DEFAULT,
  SRCML_ERR_NS_PREFIX_DEFAULT,
  SRCML_EXT_LITERAL_NS_PREFIX_DEFAULT,
  SRCML_EXT_OPERATOR_NS_PREFIX_DEFAULT,
  SRCML_EXT_MODIFIER_NS_PREFIX_DEFAULT,
  SRCML_EXT_POSITION_NS_PREFIX_DEFAULT,
};

const int num_prefixes = sizeof(uris) / sizeof(uris[0]);

void libxml_error(void *ctx, const char *msg, ...) {}

// output help
void output_help(const char* name) {
  printf( "Usage: %s [options] <src_infile>... [-o <srcML_outfile>]\n\n"

	  "Translates source-code files in C, C++, and Java into the XML source-code representation srcML.\n"
	  "Both single and multiple files can be stored in a single srcML document.\n\n"

	  "By default, output is to stdout.  You can specify a file for output using the --%s or -%c option.\n"
	  "When no filenames are given input is from stdin and output is to stdout.\n"
	  "An input filename of '-' also reads from stdin.\n\n"

    	  "Any input file can be a local filename (FILE) or a URI with the protocols http:, ftp:, or file:\n\n"

	  "Language is based on the file extension, with a default of C++.  It can also be directly\n"
	  "set using the --language option.\n"
	  "\n"
	  "Options:\n", name, OUTPUT_FLAG, OUTPUT_FLAG_SHORT);

  printf("  -%c, --%-14s display this help and exit\n",      HELP_FLAG_SHORT, HELP_FLAG);
  printf("  -%c, --%-14s display version number and exit\n\n", VERSION_FLAG_SHORT, VERSION_FLAG);

  printf("  -%c, --%-14s write result to OUTPUT which is a FILE or URI\n", OUTPUT_FLAG_SHORT, OUTPUT_FLAG_FULL);
  printf("  --%-18s read list of source file names from INPUT which is a FILE or URI,\n"
	 "                       to form a compound srcML document\n\n",
	  FILELIST_FLAG_FULL);

  printf("  -%c, --%-14s store output in a compound srcML document, default for multiple input files\n",
	  COMPOUND_FLAG_SHORT, COMPOUND_FLAG);

  printf("  -%c, --%-14s expression mode for translating a single expression not in a statement\n",
	  EXPRESSION_MODE_FLAG_SHORT, EXPRESSION_MODE_FLAG);

  printf("  -%c, --%-14s set the output XML encoding to ENC (default:  %s)\n",
	  ENCODING_FLAG_SHORT, ENCODING_FLAG, DEFAULT_XML_ENCODING);

  printf("  -%c, --%-14s set the input source encoding to ENC (default:  %s)\n\n",
	  TEXTENCODING_FLAG_SHORT, TEXTENCODING_FLAG, DEFAULT_TEXT_ENCODING);

  printf("  -%c, --%-14s output in gzip format\n", COMPRESSED_FLAG_SHORT, COMPRESSED_FLAG);

  printf("  -%c, --%-14s immediate output while parsing, default for keyboard input\n", INTERACTIVE_FLAG_SHORT, INTERACTIVE_FLAG);

  printf("  -%c, --%-14s markup translation errors in namespace %s\n",
	  DEBUG_FLAG_SHORT, DEBUG_FLAG, SRCML_ERR_NS_URI);

  printf("  -%c, --%-14s conversion and status information to stderr\n\n",
	  VERBOSE_FLAG_SHORT, VERBOSE_FLAG);

  printf("  --%-18s do not output the default XML declaration\n", NO_XML_DECLARATION_FLAG);
  printf("  --%-18s do not output any namespace declarations\n\n", NO_NAMESPACE_DECLARATION_FLAG);

  printf("Metadata Options:\n\n"
	 "  -%c, --%-14s set the language to %s, %s (default), %s, or %s\n",
	 LANGUAGE_FLAG_SHORT, LANGUAGE_FLAG_FULL,
	 LANGUAGE_C, LANGUAGE_CXX, LANGUAGE_JAVA, LANGUAGE_ASPECTJ);

  printf("  -%c, --%-14s set the directory attribute to DIR\n",
	 DIRECTORY_FLAG_SHORT, DIRECTORY_FLAG_FULL);

  printf("  -%c, --%-14s set the filename attribute to FILE\n",
	 FILENAME_FLAG_SHORT, FILENAME_FLAG_FULL);

  printf("  -%c, --%-14s set the version attribute to VER\n\n",
	 SRCVERSION_FLAG_SHORT, SRCVERSION_FLAG_FULL);

      /*
	      << "  " << SELF_VERSION_FLAG <<  "  " << "        "
	      << "set the version attribute to the src2srcml version\n"
      */

  printf("Markup Extensions:\n\n");

  printf("  --%-18s markup literal values in namespace \"%s\"\n",
	 LITERAL_FLAG, SRCML_EXT_LITERAL_NS_URI);

  printf("  --%-18s markup operators in namespace \"%s\"\n",
	 OPERATOR_FLAG, SRCML_EXT_OPERATOR_NS_URI);

  printf("  --%-18s markup type modifiers in namespace \"%s\"\n",
	 MODIFIER_FLAG, SRCML_EXT_MODIFIER_NS_URI);

  printf("\nLine/Column Position:\n\n");

  printf("  --%-18s markup literal values in namespace \"%s\"\n",
	 POSITION_FLAG, SRCML_EXT_POSITION_NS_URI);

  printf("  --%-18s have tabs NUMBER characters apart.  Default is %d\n",
	 TABS_FLAG_FULL, 8);

  printf("\nPrefix Options:\n\n");

  printf("  --%-18s set the default namespace URI\n", XMLNS_DEFAULT_FLAG_FULL);

  printf("  --%-18s set the namespace PREFIX for the namespace URI\n\n", XMLNS_FLAG_FULL);

  printf("                       Predefined URIs and Prefixes:\n");
  printf("                         xmlns=\"%s\"\n", SRCML_SRC_NS_URI);
  printf("                         xmlns:%s=\"%s\"\n", SRCML_CPP_NS_PREFIX_DEFAULT, SRCML_CPP_NS_URI);
  printf("                         xmlns:%s=\"%s\"\n", SRCML_ERR_NS_PREFIX_DEFAULT, SRCML_ERR_NS_URI);

  printf("\nCPP Markup Options:\n\n");

  printf("  --%-18s markup cpp #else regions (default)\n", CPP_MARKUP_ELSE_FLAG);
  printf("  --%-18s leave cpp #else regions as text\n\n", CPP_TEXTONLY_ELSE_FLAG);

  printf("  --%-18s markup cpp #if 0 regions\n", CPP_MARKUP_IF0_FLAG);
  printf("  --%-18s leave cpp #if 0 regions as text (default)\n\n", CPP_TEXTONLY_IF0_FLAG);

  printf("Examples:  \n"
	 "  %1$s                (read from standard input, write to standard output)\n"
	 "  %1$s m.cpp          (read from file m.cpp, write to standard output)\n"
	 "  %1$s m.cpp -o m.cpp.xml (read from file m.cpp, write to file m.cpp.xml)\n"
	 "\n"
	 "  %1$s http://www.sdml.info/projects/srcml/ex/main.cpp (read from URI)\n"
	 "\n"
	 "  %1$s --directory=src --filename=m.cpp m.cpp -o m.cpp.xml "
	 "(element unit attributes dir \"src\", filename \"m.cpp\")\n"
	 "  %1$s --src-encoding=UTF-8 m.cpp m.cpp.xml         "
	 "(encoding of input text file is UTF-8)\n"
	 "  %1$s --xml-encoding=ISO-8859-1 m.cpp m.cpp.xml    "
	 "(set encoding of srcML file to ISO-8859-1)\n\n", name);

  printf("www.sdml.info\n"
	 "Report bugs to %s\n", EMAIL_ADDRESS);
}

// output version message
void output_version(const char* name) {
  printf("%s Version %s\n%s\n", name, VERSION,COPYRIGHT);
}

int options = OPTION_CPP_MARKUP_ELSE;

#ifdef __GNUG__
extern "C" void verbose_handler(int);

extern "C" void terminate_handler(int);
#endif

struct process_options
{
  // options
  // output filename
  const char* srcml_filename;
  const char* fname;
  int language;
  const char* src_encoding;
  const char* xml_encoding;
  const char* given_directory;
  const char* given_filename;
  const char* given_version;
  int tabsize;
  bool prefixchange[num_prefixes];
};

// setup options and collect info from arguments
int process_args(int argc, char* argv[], process_options & poptions);

int main(int argc, char* argv[]) {

  int exit_status = EXIT_SUCCESS;

  LIBXML_TEST_VERSION

  xmlGenericErrorFunc handler = (xmlGenericErrorFunc) libxml_error;
  initGenericErrorDefaultFunc(&handler);
  /*
  if (xmlRegisterInputCallbacks(curlMatch, curlOpen, curlRead, curlClose) < 0) {
    fprintf(stderr, "failed to register curl handler\n");
    exit(1);
  }
  */

  /* signal handling */

#ifdef __GNUG__
  // signal to toggle verbose flag
  pstd::signal(SIGUSR1, verbose_handler);
#endif

  process_options poptions =
    {
      0,
      "-",
      0,
      DEFAULT_TEXT_ENCODING,
      DEFAULT_XML_ENCODING,
      0,
      0,
      0,
      8,
      {
	false,
	false,
	false,
	false,
	false,
	false,
      }
    };

  // process command-line arguments
  int curarg = process_args(argc, argv, poptions);

  /* Special checks for illegal combinations */

  // first command line parameter after options are the input filenames
  int input_arg_start = curarg;
  int input_arg_end = argc - 1;
  int input_arg_count = input_arg_end - input_arg_start + 1;

  // no output specified, so use stdout
  if (!poptions.srcml_filename)
    poptions.srcml_filename = "-";

  // if more than one input filename assume nested
  if (input_arg_count > 1)
    options |= OPTION_NESTED;

  // verify that the output filename is not the same as any of the input filenames
#ifdef __GNUG__
  struct stat outstat;
  stat(poptions.srcml_filename, &outstat);
  for (int i = input_arg_start; i <= input_arg_end; ++i) {

    struct stat instat;
    stat(argv[i], &instat);
    if (instat.st_ino == outstat.st_ino && instat.st_dev == outstat.st_dev) {
      fprintf(stderr, "%s: Input file '%s' is the same as the output file '%s'\n",
	      argv[0], argv[i], poptions.srcml_filename);
      exit(STATUS_INPUTFILE_PROBLEM);
    }
  }
#endif

  // make sure user did not specify duplicate prefixes as an option
  for (int i = 0; i < num_prefixes - 1; ++i) {
    for (int j = i + 1; j < num_prefixes; ++j)
      if(strcmp(urisprefix[i], urisprefix[j]) == 0) {

	fprintf(stderr, "%s: Namespace conflict for ", NAME);
	if (urisprefix[i] == '\0') {
	  fprintf(stderr, "default prefix\n");
	} else {
	  fprintf(stderr, "prefix \'%s\'\n", urisprefix[i]);
	}
	fprintf(stderr, "Prefix URI conflicts:\n  %s\n  %s\n", uris[i].uri, uris[j].uri);

	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
  }

  // automatic interactive use from stdin (not on redirect or pipe)
#ifdef __GNUG__
  if (input_arg_count == 0 || strcmp(argv[input_arg_start], STDIN) == 0) {

    if (isatty(STDIN_FILENO))
      options |= OPTION_INTERACTIVE;
  }
#endif

  try {

    // for single file specified on command line, grab the language from the extension
    if (poptions.language == 0 && input_arg_count == 1) {

      char* path = argv[input_arg_start];
      poptions.language = Language::getLanguageFromFilename(path);
    }

    // turnon cpp namespace for non Java-based languages
    if (!(poptions.language == srcMLTranslator::LANGUAGE_JAVA || poptions.language == srcMLTranslator::LANGUAGE_ASPECTJ))
	options |= OPTION_CPP;

    // translator from input to output using determined language
    //    if (language == 0)
    //	language = DEFAULT_LANGUAGE;
    srcMLTranslator translator(poptions.language == 0 ? DEFAULT_LANGUAGE : poptions.language, poptions.src_encoding, poptions.xml_encoding, poptions.srcml_filename, options, poptions.given_directory, poptions.given_filename, poptions.given_version, urisprefix, poptions.tabsize);

  // output source encoding
  if (isoption(options, OPTION_VERBOSE)) {
    fprintf(stderr, "Source encoding:  %s\n", poptions.src_encoding);
    fprintf(stderr, "XML encoding:  %s\n", poptions.xml_encoding);
  }

  // translate input filenames from list in file
  if (isoption(options, OPTION_FILELIST)) {

#ifdef __GNUG__
    // setup so we can gracefully stop after a file at a time
    pstd::signal(SIGINT, terminate_handler);
#endif
      
    try {

      // translate all the filenames listed in the named file
      // Use libxml2 routines so that we can handle http:, file:, and gzipped files automagically
      URIStream uriinput(poptions.fname);
      int count = 0;
      char* line;
      while ((line = uriinput.getline())) {

	// skip blank lines or comment lines
	if (line[0] == '\0' || line[0] == FILELIST_COMMENT)
	  continue;

	// another file
	++count;

	// in verbose mode output the currently processed filename
	if (isoption(options, OPTION_VERBOSE))
	  fprintf(stderr, "%d\t%s", count, line);

	// turnon cpp namespace for non Java-based languages
	if (!(poptions.language == srcMLTranslator::LANGUAGE_JAVA || poptions.language == srcMLTranslator::LANGUAGE_ASPECTJ))
	  options |= OPTION_CPP;

	// translate the file listed in the input file using the directory and filename extracted from the path
	char* dir = 0;
	char* filename = 0;
	try {
	  translator.setupInput(line);

	  // language based on extension
	  if (poptions.language == 0)
	    poptions.language = Language::getLanguageFromFilename(line);

	  filename_split(line, dir, filename);
	  translator.translate(dir,
			       filename,
			       poptions.given_version,
			       poptions.language,
			       poptions.tabsize);
	} catch (FileError) {

	  if (dir)
	    fprintf(stderr, isoption(options, OPTION_VERBOSE) ? "\t\terror: file \'%s/%s\' does not exist.\n" :
				   " error: file \'%s/%s\' does not exist.\n", dir, filename);
	  else
	    fprintf(stderr, isoption(options, OPTION_VERBOSE) ? "\t\terror: file \'%s\' does not exist.\n" :
				   " error: file \'%s\' does not exist.\n", filename);
	}

	if (isoption(options, OPTION_VERBOSE)) {
	  fprintf(stderr, "\n");
	}

	// compound documents are interrupted gracefully
	if (isoption(options, OPTION_TERMINATE))
	  return STATUS_TERMINATED;
      }

    } catch (URIStreamFileError) {
      fprintf(stderr, "%s error: file/URI \'%s\' does not exist.\n", argv[0], poptions.fname);
      exit(STATUS_INPUTFILE_PROBLEM);
    }

    // translate from standard input
  } else if (input_arg_count == 0 || strcmp(argv[input_arg_start], STDIN) == 0) {

    // translate from standard input using any directory, filename and version given on the command line
    translator.setupInput(STDIN);
    translator.translate(poptions.given_directory, poptions.given_filename, poptions.given_version, 
			 poptions.language ? poptions.language : DEFAULT_LANGUAGE,
			 poptions.tabsize);

  // translate single input filename from command line
  }  else if (input_arg_count == 1) {

    // translate from path given on command line using directory given on the command line or extracted
    // from full path
    char* path = argv[input_arg_start];
    translator.setupInput(path);
    char* path_s = 0;
    char* filename_s = 0;
    filename_split(path, path_s, filename_s);

    // hack to fix where directory, but no filename
    if (path_s && !filename_s[0]) {
      char* p = path_s;
      path_s = filename_s;
      filename_s = p;
    }

    try {
      translator.translate(isoption(options, OPTION_DIRECTORY) ? poptions.given_directory : path_s,
			   isoption(options, OPTION_FILENAME)  ? poptions.given_filename  : filename_s,
			   poptions.given_version,
			   poptions.language ? poptions.language : DEFAULT_LANGUAGE,
			   poptions.tabsize);

    } catch (FileError) {

      if (path_s)
	fprintf(stderr, "%s error: file \'%s/%s\' does not exist.\n", argv[0], path_s, filename_s);
      else
	fprintf(stderr, "%s error: file \'%s\' does not exist.\n", argv[0], filename_s);

      exit(STATUS_INPUTFILE_PROBLEM);
    }

  // translate multiple input filenames on command line
  } else {

    int count = 0;    // keep count for verbose mode

#ifdef __GNUG__
    // setup so we can gracefully stop after a file at a time
    pstd::signal(SIGINT, terminate_handler);
#endif
      
    // translate in batch the input files on the command line extracting the directory and filename attributes
    // from the full path
    for (int i = input_arg_start; i <= input_arg_end; ++i) {

      char* path = argv[i];

      // another file
      ++count;

      // in verbose mode output the currently processed filename
      if (isoption(options, OPTION_VERBOSE)) {
	fprintf(stderr, "%d\t%s", count, path);
      }
      char* path_s = 0;
      char* filename_s = 0;
      try {
	translator.setupInput(path);
	filename_split(path, path_s, filename_s);
	translator.translate(path_s, filename_s, 0, poptions.language ? poptions.language : DEFAULT_LANGUAGE, poptions.tabsize);
      } catch (FileError) {
	if (path_s)
	  fprintf(stderr, "%s error: file \'%s/%s\' does not exist.\n", argv[0], path_s, filename_s);
	else
	  fprintf(stderr, "%s error: file \'%s\' does not exist.\n", argv[0], filename_s);
      }

      if (isoption(options, OPTION_VERBOSE)) {
	fprintf(stderr, "\n");
      }

      // compound documents are interrupted gracefully
      if (isoption(options, OPTION_TERMINATE))
	return STATUS_TERMINATED;
    }
  }
  } catch (srcEncodingException) {
    fprintf(stderr, "Translation encoding problem\n");
    exit(STATUS_UNKNOWN_ENCODING);
  }

  return exit_status;
}

// setup options and collect info from arguments
int process_args(int argc, char* argv[], process_options & poptions) {

  bool cpp_if0 = false;
  bool cpp_else = false;

  int curoption = 0;
  struct option cliargs[] = {
    { HELP_FLAG, no_argument, NULL, HELP_FLAG_SHORT },
    { VERSION_FLAG, no_argument, NULL, VERSION_FLAG_SHORT },
    { OUTPUT_FLAG, required_argument, NULL, OUTPUT_FLAG_SHORT },
    { COMPOUND_FLAG, no_argument, NULL, COMPOUND_FLAG_SHORT },
    { EXPRESSION_MODE_FLAG, no_argument, NULL, EXPRESSION_MODE_FLAG_SHORT },
    { ENCODING_FLAG, required_argument, NULL, ENCODING_FLAG_SHORT },
    { TEXTENCODING_FLAG, required_argument, NULL, TEXTENCODING_FLAG_SHORT },
    { COMPRESSED_FLAG, no_argument, NULL, COMPRESSED_FLAG_SHORT },
    { INTERACTIVE_FLAG, no_argument, NULL, INTERACTIVE_FLAG_SHORT },
    { DEBUG_FLAG, no_argument, NULL, DEBUG_FLAG_SHORT },
    { VERBOSE_FLAG, no_argument, NULL, VERBOSE_FLAG_SHORT },
    { LANGUAGE_FLAG, required_argument, NULL, LANGUAGE_FLAG_SHORT },
    { DIRECTORY_FLAG, required_argument, NULL, DIRECTORY_FLAG_SHORT },
    { FILENAME_FLAG, required_argument, NULL, FILENAME_FLAG_SHORT },
    { SRCVERSION_FLAG, required_argument, NULL, SRCVERSION_FLAG_SHORT },
    { FILELIST_FLAG, required_argument, NULL, 'F' },
    { REGISTER_EXT_FLAG, required_argument, NULL, 'R' },
    { XMLNS_FLAG, required_argument, NULL, 'X' },
    { NO_XML_DECLARATION_FLAG, no_argument, &curoption, OPTION_XMLDECL | OPTION_XML },
    { NO_NAMESPACE_DECLARATION_FLAG, no_argument, &curoption, OPTION_NAMESPACEDECL | OPTION_XML },
    { TABS_FLAG, required_argument, NULL, 'T' },
    { POSITION_FLAG, no_argument, &curoption, OPTION_POSITION },
    { LITERAL_FLAG, no_argument, &curoption, OPTION_LITERAL },
    { OPERATOR_FLAG, no_argument, &curoption, OPTION_OPERATOR },
    { MODIFIER_FLAG, no_argument, &curoption, OPTION_MODIFIER },
    { CPP_MARKUP_ELSE_FLAG, no_argument, NULL, 'm' },
    { CPP_TEXTONLY_ELSE_FLAG, no_argument, NULL, 'E' },
    { CPP_MARKUP_IF0_FLAG, no_argument, NULL, '0' },
    { CPP_TEXTONLY_IF0_FLAG, no_argument, NULL, 'p' },
    { 0, 0, 0, 0 }
  };

  // process all command line options
  char* embedded;
  const char* ns_prefix;
  const char* ns_uri;

  while (1) {
    int lastoptind = optind;
    curoption = 0;
    int option_index = 0;
    bool special = optind < argc && !strncmp(argv[optind], "--xmlns:", 8);
    opterr = !special ? 1 : 0;
    int c = getopt_long(argc, argv, "hVo:nex:t:zcgvl:d:f:s:", cliargs, &option_index);

    if (c == -1)
      break;

    if (curoption) {
      options |= curoption;
      continue;
    }

    // treat --xmlns:prefix=url as --xmlns=url for processing
    if (special && c == '?') {
      c = 'X';
    }

    // missing or extra option argument
    if (c == '?') {
      fprintf(stderr, "Try '%s %s' for more information.\n", argv[0], HELP_FLAG);
      exit(option_error_status(optopt));
    }

    char* end = 0;
    switch(c) {

    case HELP_FLAG_SHORT: 
      output_help(argv[0]);
      exit(STATUS_SUCCESS);

      break;

    case VERSION_FLAG_SHORT: 
      output_version(argv[0]);
      exit(STATUS_SUCCESS);
      break;

    case OUTPUT_FLAG_SHORT: 
      poptions.srcml_filename = optarg;
      break;

    case 'F': 
      options |= OPTION_FILELIST;

      // filelist mode is default nested mode
      options |= OPTION_NESTED;

      poptions.fname = optarg;
      break;

    case 'R': 

      end = asg_split(optarg);

      Language::registerUserExt(optarg, end);
      break;

    case NESTED_FLAG_SHORT: 
      options |= OPTION_NESTED;
      break;

    case EXPRESSION_MODE_FLAG_SHORT: 
      options |= OPTION_EXPRESSION;
      break;

    case ENCODING_FLAG_SHORT: 
      options |= OPTION_XML_ENCODING;

      poptions.xml_encoding = optarg;

      // validate xml encoding
      if (!srcMLOutput::checkEncoding(poptions.xml_encoding)) {
	fprintf(stderr, "%s: xml encoding \"%s\" is not supported.\n", argv[0], poptions.xml_encoding);
	fprintf(stderr, "Try '%s %s' for more information.\n", argv[0], HELP_FLAG);
	exit(STATUS_UNKNOWN_ENCODING);
      }
      break;

    case TEXTENCODING_FLAG_SHORT: 
      options |= OPTION_TEXT_ENCODING;

      poptions.src_encoding = optarg;

      // validate source encoding
      if (!srcMLOutput::checkEncoding(poptions.src_encoding)) {
	fprintf(stderr, "%s: text encoding \"%s\" is not supported.\n", argv[0], poptions.src_encoding);
	fprintf(stderr, "Try '%s %s' for more information.\n", argv[0], HELP_FLAG);
	exit(STATUS_UNKNOWN_ENCODING);
      }
      break;

    case 'X': 
      ns_prefix = 0;
      ns_uri = 0;

      // find the ':' or '=' or end of string
      for (embedded = argv[optind - 1]; *embedded; ++embedded)
	if (*embedded == ':' || *embedded == '=')
	  break;

      // found prefix for sure
      if (*embedded == '=') {

	ns_prefix = "";
	ns_uri = embedded + 1;

      } else if (*embedded == ':') {

	ns_prefix = embedded + 1;

      } else {

	ns_prefix = "";
      }

      // look for uri in rest of this argument
      if (!ns_uri)
	for (; *embedded; ++embedded)
	  if (*embedded == '=') {
	    ns_uri = embedded + 1;
	    *embedded = '\0';
	    break;
	  }

      // look for uri in next argument
      if (!ns_uri) {
	if (!(optind < argc && argv[optind][0] != '-')) {
	  fprintf(stderr, "%s: xmlns option selected but not specified.\n", NAME);
	  exit(STATUS_LANGUAGE_MISSING);
	}

	ns_uri = argv[optind++];
      }

      // check uri to turn on specific option
      process = false;
      for (int i = 0; i < num_prefixes; ++i)
	if (strcmp(ns_uri, uris[i].uri) == 0) {

	  options |= uris[i].option;

	  urisprefix[i] = ns_prefix;
	  poptions.prefixchange[i] = true;
	  process = true;
	  break;
	}

      if (!process) {
	fprintf(stderr, "%s: invalid namespace \"%s\"\n\n"
		"Namespace URI must be on of the following:  \n"
		"  %-35s primary srcML namespace\n"
		"  %-35s namespace for cpreprocessing elements\n"
		"  %-35s namespace for srcML debugging elements\n"
		"  %-35s namespace for optional literal elements\n"
		"  %-35s namespace for optional operator element\n"
		"  %-35s namespace for optional modifier element\n"
		"  %-35s namespace for optional position element\n",
		argv[0], ns_uri,
		SRCML_SRC_NS_URI, SRCML_CPP_NS_URI, SRCML_ERR_NS_URI,
		SRCML_EXT_LITERAL_NS_URI, SRCML_EXT_OPERATOR_NS_URI, SRCML_EXT_MODIFIER_NS_URI,
		SRCML_EXT_POSITION_NS_URI
		);
	exit(STATUS_INVALID_LANGUAGE);
      }
      break;

    case COMPRESSED_FLAG_SHORT: 
      options |= OPTION_COMPRESSED;
      break;

    case INTERACTIVE_FLAG_SHORT: 
      options |= OPTION_INTERACTIVE;
      break;

    case DEBUG_FLAG_SHORT: 
      options |= OPTION_DEBUG;
      break;

    case VERBOSE_FLAG_SHORT: 
      options |= OPTION_VERBOSE;
      break;

    case LANGUAGE_FLAG_SHORT: 
      options |= OPTION_LANGUAGE;

      // validate language selected
      poptions.language = Language::getLanguage(optarg);
      if (poptions.language == 0) {
	fprintf(stderr, "%s: invalid option -- Language flag must one of the following values:  "
		"%s %s %s %s\n", argv[0], LANGUAGE_C, LANGUAGE_CXX, LANGUAGE_JAVA, LANGUAGE_ASPECTJ);

	exit(STATUS_INVALID_LANGUAGE);
      }
      break;

    case DIRECTORY_FLAG_SHORT: 

      // check for missing argument confused by an argument that looks like an option
      checkargisoption(argv[0], argv[lastoptind], optarg, optind, lastoptind);

      options |= OPTION_DIRECTORY;

      poptions.given_directory = optarg;
      break;

    case FILENAME_FLAG_SHORT: 

      // check for missing argument confused by an argument that looks like an option
      checkargisoption(argv[0], argv[lastoptind], optarg, optind, lastoptind);

      options |= OPTION_FILENAME;

      poptions.given_filename = optarg;
      break;

    case SRCVERSION_FLAG_SHORT: 
      options |= OPTION_VERSION;

      poptions.given_version = optarg;
      break;

    case 'T' :
      options |= OPTION_POSITION;
      
      char * end;
      poptions.tabsize = pstd::strtol(optarg, &end, 10);
      
      // validate type of tabsize number
      if (errno == EINVAL || strlen(end) == strlen(optarg)) {
        fprintf(stderr, "%s: unit option value \"%s\" must be numeric.\n", argv[0], optarg);
        exit(STATUS_UNIT_INVALID);
      }

      // validate range of unit number
      if (poptions.tabsize <= 0) {
        fprintf(stderr, "%s: unit option value \"%d\" must be > 0.\n", argv[0], poptions.tabsize);
        exit(STATUS_UNIT_INVALID);
      }

      break;


    case 'm': 
      if (!cpp_else) {
	options |= OPTION_CPP_MARKUP_ELSE;

	cpp_else = true;

      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_ELSE_FLAG, CPP_TEXTONLY_ELSE_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
      break;

    case 'E': 
      if (!cpp_else) {
	options &= ~OPTION_CPP_MARKUP_ELSE;

	cpp_else = true;
      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_ELSE_FLAG, CPP_TEXTONLY_ELSE_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
      break;

    case '0': 
      if (!cpp_if0) {
	options |= OPTION_CPP_MARKUP_IF0;

	cpp_if0 = true;

      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_IF0_FLAG, CPP_TEXTONLY_IF0_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
      break;

    case 'p': 
      if (!cpp_if0) {
	// clear if previously marked
	options &= ~OPTION_CPP_MARKUP_IF0;

	cpp_if0 = true;

      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_IF0_FLAG, CPP_TEXTONLY_IF0_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
      break;

    default:
      exit(STATUS_UNKNOWN_OPTION);
      break;
    };
  }

  return optind;
}

#ifdef __GNUG__
extern "C" void verbose_handler(int) {

  if ((options &= OPTION_VERBOSE) == 0)
    options |= OPTION_VERBOSE;
  else
    options &= ~OPTION_VERBOSE;
}

extern "C" void terminate_handler(int) {

  // setup to terminate after current file
  options |= OPTION_TERMINATE;

  // turn off handler for this signal
  pstd::signal(SIGINT, SIG_DFL);
}
#endif

int option_error_status(int optopt) {

  switch (optopt) {

  case FILENAME_FLAG_SHORT:
    return STATUS_FILENAME_MISSING;
    break;

  case LANGUAGE_FLAG_SHORT:
    return STATUS_LANGUAGE_MISSING;
    break;

  case DIRECTORY_FLAG_SHORT:
    return STATUS_DIRECTORY_MISSING;
    break;

  case SRCVERSION_FLAG_SHORT:
    return STATUS_VERSION_MISSING;
    break;

  case ENCODING_FLAG_SHORT:
    return STATUS_XMLENCODING_MISSING;
    break;

  case TEXTENCODING_FLAG_SHORT:
    return STATUS_SRCENCODING_MISSING;
    break;

  case 'T':
    return STATUS_ERROR;
    break;

  case '\0':
    return STATUS_UNKNOWN_OPTION;
    break;
  };

  return 0;
}
