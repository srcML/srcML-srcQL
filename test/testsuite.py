#!/usr/bin/env python
#
# update.py
#
# Michael L. Collard

import sys
import os.path
import re
import subprocess
import difflib
import string
from datetime import datetime, time

sys.path.append("../src/libsrcml/python")
sys.path.append("../src")
from libsrcml import *
from srcML import *

maxcount = 700
error_filename = "srcMLTestReport"
error_filename_extension = ".txt"

FIELD_WIDTH_LANGUAGE = 5
FIELD_WIDTH_DIRECTORY = 27
MAX_COUNT = 29
sperrorlist = []
xml_filename = ""

srcmltranslator = os.environ.get("SRC2SRCML")
if srcmltranslator == "" or srcmltranslator == None:
	srcmltranslator = "../bin/src2srcml"

srcmlutility = os.environ.get("SRCML2SRC")
if srcmlutility == "" or srcmlutility == None:
	srcmlutility = "../bin/srcml2src"

# Walk into directories in filesystem
# Ripped from os module and slightly modified
# for alphabetical sorting
#
def sortedWalk(top, topdown=True, onerror=None):
    from os.path import join, isdir, islink
 
    names = os.listdir(top)
    names.sort()
    dirs, nondirs = [], []
 
    for name in names:
        if isdir(os.path.join(top, name)):
            dirs.append(name)
        else:
            nondirs.append(name)
 
    if topdown:
        yield top, dirs, nondirs
    for name in dirs:
        path = join(top, name)
        if not os.path.islink(path):
            for x in sortedWalk(path, topdown, onerror):
                yield x
    if not topdown:
        yield top, dirs, nondirs

# srcExpr translator
srcexprtranslator = '/home/collard/srcML/trunk/srcexpr/src/srcexpr'

# extracts a particular unit from a srcML file
def safe_communicate(command, inp):

	try:
		return subprocess.Popen(command, stdout=subprocess.PIPE, stdin=subprocess.PIPE).communicate(inp)[0]
	except OSError, (errornum, strerror):
		try:
			return subprocess.Popen(command, stdout=subprocess.PIPE, stdin=subprocess.PIPE).communicate(inp)[0]
		except OSError, (errornum, strerror):
			sperrorlist.append((command, xml_filename, errornum, strerror))
			raise

# extracts a particular unit from a srcML file
def safe_communicate_file(command, filename):

	newcommand = command[:]
	newcommand.append(filename)
	try:
		return subprocess.Popen(newcommand, stdout=subprocess.PIPE, stdin=subprocess.PIPE).communicate()[0]
	except OSError, (errornum, strerror):
		try:
			return subprocess.Popen(newcommand, stdout=subprocess.PIPE, stdin=subprocess.PIPE).communicate()[0]
		except OSError, (errornum, strerror):
			sperrorlist.append((command, xml_filename, errornum, strerror))
			raise

# extracts a particular unit from a srcML file
def extract_unit(src, count):

	command = [srcmlutility, "--unit=" + str(count), "--xml"]

	return safe_communicate(command, src)

# extracts a particular unit from a srcML file
def extract_all_executable(src):

	command = [srcmlutility, "-0", "--xml"]

	return safe_communicate(command, src)

def extract_one(archive, list) :

        unit = archive.read_unit()
        if unit.get_xml() != None :
                list.append(unit.get_xml())
                extract_one(archive, list)

# extracts a particular unit from a srcML file
def extract_all(src):
        print "HERE"
        all = []

        archive = srcml_archive()
        archive.read_open_memory(src)
        extract_one(archive, all)
        archive.close()

        all.append(0)

	return all

def name2filestr(src_filename):
	file = open(src_filename).read()
	
	return file

# converts a srcML file back to text
def srcml2src_executable(srctext, encoding):

	# run the srcml processor
	command = [srcmlutility]
	command.append("--src-encoding=" + encoding)

	return safe_communicate(command, srctext)

# converts a srcML file back to text
def srcml2src(srctext, encoding):

	# run the srcml processor
        utility = srcMLUtility(srctext, len(srctext) + 1, encoding, 0, "")
        source = utility.extract_text(0)
        utility.delete()

	return source

# converts from unix to dos line endings
def unix2dos(srctext):

	# run the srcml processor
	command = ['unix2dos']

	return safe_communicate(command, srctext)

# find differences of two files
def xmldiff(xml_filename1, xml_filename2):

	if xml_filename1 != xml_filename2:
		return list(difflib.unified_diff(xml_filename1.splitlines(1), xml_filename2.splitlines(1)))
	else:
		return ""

# find differences of two files
def src2srcML_executable(text_file, encoding, language, directory, filename, prefixlist):

        command = [srcmltranslator, "-l", language, "--encoding=" + encoding]

        if directory != "":
                command.extend(["--directory", directory])

        if filename != "":
                command.extend(["--filename", filename])

        command.extend(prefixlist)

        #print command                                                                                                                                 

        # run the srcml processor                                                                                                                      
        command.append("--src-encoding=" + encoding)

        command.append("--quiet")

        return safe_communicate(command, text_file)

# find differences of two files
def src2srcML(text_file, encoding, language, directory, filename, prefixlist):

        options = SRCML_OPTION_CPP

        if filename == "" :
                filename = None;

        if prefixlist.count("--xmlns:op=http://www.sdml.info/srcML/operator") :
                options = options | SRCML_OPTION_OPERATOR
        if prefixlist.count("--xmlns:lit=http://www.sdml.info/srcML/literal") :
                options = options | SRCML_OPTION_LITERAL
        if prefixlist.count("--xmlns:type=http://www.sdml.info/srcML/modifier") :
                options = options | SRCML_OPTION_MODIFER

        archive = srcml_archive()

        archive.write_open_memory()
        archive.set_all_options(options)
        unit = srcml_unit(archive)
        unit.set_language(language)
        is_all =  directory.find(".all") 
        if is_all != -1 :
                unit.set_filename(filename)
                unit.set_directory(directory)

        unit.parse_memory(text_file)
        archive.close()
        srcml = unit.get_xml()
        if is_all == -1 :
                srcml = "<unit>" + srcml[srcml.find(">") + 1:]

        return srcml

# additional processing stages
def srcML2srcMLStages(srcmlfile, otherxmlns):

	newfile = srcmlfile
	for a in otherxmlns:
		url = a.split('=')[1]
		if url == 'http://www.sdml.info/srcExpr':
			command = [srcexprtranslator]
			newfile = safe_communicate(command, newfile)
		
	return newfile

#
def getsrcmlattribute(xml_file, command):

	last_line = safe_communicate([srcmlutility, command], xml_file)

	return last_line.strip()

def getsrcmlattributefile(xml_file, command):

	last_line = safe_communicate_file([srcmlutility, command], xml_file)

	return last_line.strip()

def getsrcmlattributeraw(srctext, command):

	# run the srcml processor
	command = [srcmlutility]
	command.append("--info")

	return safe_communicate(command, srctext)

# directory attribute
def getdirectory(xml_file):

	return getsrcmlattribute(xml_file, "-d")

# language attribute
def getlanguage(xml_file):
	
	return getsrcmlattribute(xml_file, "-l")

# xml encoding
def getencoding(xml_file):

	return getsrcmlattribute(xml_file, "-x")

# version attribute
def getversion(xml_file):

	return getsrcmlattribute(xml_file, "-x")

# filename attribute
def getfilename(xml_file):

	return getsrcmlattribute(xml_file, "-f")

# xmlns attribute
def getfullxmlns_executable(xml_file):

	l = []
	for a in getsrcmlattribute(xml_file, "--info").split():
		if a[0:5] == "xmlns":
			l.append("--" + a.replace('"', ""))
	
	return l

# xmlns attribute
def getfullxmlns(xml_file):

	l = []
        utility = srcMLUtility(xml_file, len(xml_file) + 1, encoding, 0, "")
	for a in utility.long_info().split():
		if a[0:5] == "xmlns":
			l.append("--" + a.replace('"', ""))
        utility.delete()

	return l

# xmlns attribute
def defaultxmlns(l):

	newl = []
	for a in l:
		url = a.split('=')[1]
		if url == 'http://www.sdml.info/srcML/src' or url == 'http://www.sdml.info/srcML/cpp' or url == 'http://www.sdml.info/srcML/srcerr' or url == 'http://www.sdml.info/srcML/literal' or url == 'http://www.sdml.info/srcML/operator':
			newl.append(a)
	return newl

def nondefaultxmlns(l):

	newl = []
	for a in l:
		url = a.split('=')[1]
		if not(url == 'http://www.sdml.info/srcML/src' or url == 'http://www.sdml.info/srcML/cpp' or url == 'http://www.sdml.info/srcML/srcerr' or url == 'http://www.sdml.info/srcML/literal' or url == 'http://www.sdml.info/srcML/operator'):
			newl.append(a)
	return newl

# version of src2srcml
def src2srcmlversion():
	last_line = safe_communicate([srcmltranslator, "-V"], "")

	return last_line.splitlines()[0].strip()

# version of srcml2src
def srcml2srcversion():

	last_line = safe_communicate([srcmlutility, "-V"], "")

	return last_line.splitlines()[0].strip()

# number of nested units
def getnested(xml_file):

	snumber = safe_communicate([srcmlutility, "-n"], xml_file)

	if snumber != "":
		return int(snumber)
	else:
		return 0

class Tee(object):
    def __init__(self, name):
        self.file = open(name, "w")
        self.stdout = sys.stdout
        sys.stdout = self

    def __del__(self):
        sys.stdout = self.stdout
        self.file.close()

    def write(self, data):
        self.file.write(data)
        self.stdout.write(data)

Tee(error_filename)

print "Testing:"
print 


# Handle optional dos line endings
doseol = False
use_exec = False
while len(sys.argv) > 1 and ( sys.argv[1] == "--dos" or sys.argv[1] == "--exec" ) :
        if sys.argv[1] == "--dos" :
                doseol = True
        else :
                use_exec = True
        sys.argv.pop(0)

if use_exec :
        print src2srcmlversion()
        print srcml2srcversion()
else :
        print srcml_version_string()
print

specname = ""
if len(sys.argv) > 1:
	specname = sys.argv[1]

if specname != "":
	print specname

specnum = 0
speclang = ""
if len(sys.argv) == 3:
	result = sys.argv[2]
	if result == "C++" or result == "C" or result == "C#" or result == "Java" or result == "C++11" or result == "AspectJ":
		speclang = result
	else:
		specnum = int(sys.argv[2])
elif len(sys.argv) == 2:
	result = sys.argv[1]
	if result == "C++" or result == "C" or result == "C#" or result == "Java" or result == "C++11" or result == "AspectJ":
		speclang = result
		specname = ""
	else:
		specname = result
elif len(sys.argv) > 2:
	specnum = int(sys.argv[2])

	if len(sys.argv) > 3:
		speclang = sys.argv[3]

# base directory
base_dir = "../suite"

errorlist = []

#if not(os.path.isfile(srcmltranslator)):
#	print srcmltranslator + " does not exist."
#	exit

m = re.compile(specname + "$")

# source directory
source_dir = base_dir

# total number of errors
error_count = 0

# total test cases
total_count = 0

try:
			
	# process all files		
	for root, dirs, files in sortedWalk(source_dir, topdown=True):		

		# process all files
		for name in files:
			try: 
	
				# only process xml files
				if os.path.splitext(name)[1] != ".xml":
					continue

				# full path of the file
				xml_filename = os.path.join(root, name)

                                f = open(xml_filename, "r")

                                line1 = f.readline()
                                filexml = line1
                                s = f.readline()
                                filexml += s

                                language = s.split("language=\"")[1].split('"')[0]

                                part = s.split("dir=\"")
                                directory = part[1].split('"')[0] if len(part) > 1 else None

                                part = s.split("filename=\"")
                                filename = part[1].split('"')[0] if len(part) > 1 else None

				# only process if directory name matches or is not given
				if specname != "" and m.match(directory) == None and (filename == None or m.match(filename) == None):
                                        f.close()
					continue

				# only process if language matches or is not given
				if speclang != "" and language != speclang:
                                        f.close()
					continue
			
                                filexml += f.read()
                                f.close()
                                        
				# output language and directory
				print
				print language.ljust(FIELD_WIDTH_LANGUAGE), " ", directory.ljust(FIELD_WIDTH_DIRECTORY), " ",

				# encoding of the outer unit
                                part = line1.split("encoding=\"")
                                encoding = part[1].split('"')[0]
		
				if specnum == 0:
					count = 0
				else:
					count = specnum - 1

                                if use_exec :
                                        all = string.split(extract_all_executable(filexml), '\0')
                                else :
                                        all = extract_all(filexml)

                                number = len(all) - 1
                                if use_exec :
                                        xmlns = defaultxmlns(getfullxmlns_executable(filexml))
                                else :
                                        xmlns = defaultxmlns(getfullxmlns(filexml))
				while len(xmlns) == 0 :
					if use_exec :
						xmlns = defaultxmlns(getfullxmlns_executable(filexml))
					else :
						xmlns = defaultxmlns(getfullxmlns(filexml))
				while count == 0 or count < number:

					try: 

						count = count + 1

						if specnum!= 0 and count > specnum:
							break

						if count > maxcount:
							break

                                                #print all[count]

						# total count of test cases
						total_count = total_count + 1

						# save the particular nested unit
						if number == 0:
							unitxml = filexml
						else:
							unitxml = all[count - 1]

						# convert the unit in xml to text
                                                if use_exec :
                                                        unittext = srcml2src_executable(unitxml, encoding)
                                                else :
                                                        unittext = srcml2src(unitxml, encoding)

						# convert the unit in xml to text (if needed)
                                                if doseol:
                                                        unittext = unix2dos(unittext)

						# convert the text to srcML
                                                if use_exec :
                                                        unitsrcmlraw = src2srcML_executable(unittext, encoding, language, directory, getfilename(unitxml), xmlns)
                                                else :
                                                        unitsrcmlraw = src2srcML(unittext, encoding, language, directory, filename, xmlns)

						# additional, later stage processing
						unitsrcml = unitsrcmlraw # srcML2srcMLStages(unitsrcmlraw, nondefaultxmlns(getfullxmlns(unitxml)))
						
						# find the difference
						result = xmldiff(unitxml, unitsrcml)
						if count > 0 and (count % MAX_COUNT) == 0:
							print "\n", "".rjust(FIELD_WIDTH_LANGUAGE), " ", "...".ljust(FIELD_WIDTH_DIRECTORY), " ",
						if result != "":
							error_count += 1
							
							errorlist.append((directory + " " + language, count, result, name))

							# part of list of nested unit number in output
							print "\033[0;31m" + str(count) + "\033[0m",
						elif number != 0:
							# part of list of nested unit number in output
							print "\033[0;33m" + str(count) + "\033[0m",
	
					except OSError, (errornum, strerror):
						continue

			except OSError, (errornum, strerror):
				continue

	ki = False
except KeyboardInterrupt:
	ki = True
	
print
print
print "Report:"
if ki:
	print
	print "Testing stopped by keyboard"
	
# output error counts
# and delete error file
print

#for oldfile in os.listdir("."):
#	if re.match(error_filename + ".+", oldfile):
#		os.remove(oldfile)



f = open(error_filename, "w")

if error_count == 0:
	print "No errors out of " + str(total_count) + " cases" 
else:
	# break errorlist into two, one with original name, one with a name with a dot in it
	oerrorlist = []
	xerrorlist = []
	for e in errorlist:
		if str(e[0]).count(".") == 0:
			oerrorlist.append(e);
		else:
			xerrorlist.append(e);

	print "Errors:  " + str(error_count) + " out of " + str(total_count),
	if str(total_count) == "1":
		print " case", 
	else:
		print " cases", 
	print "\n"
	print "Errorlist:"
	nxerrorlist = xerrorlist[:]
	for e in oerrorlist:
		othererror = ""
		for x in xerrorlist[:]:
			if str(e[0]).split(' ')[1] != str(x[0]).split(' ')[1]:
				continue

			if str(e[1]) != str(x[1]):
				continue

			if str(x[0]).split('.')[0] == str(e[0]).split(' ')[0]:
				othererror = othererror + " " + str(x[0]).split(' ')[0].split('.')[1]

			xerrorlist.remove(x)

		print e[0], e[1], e[3], othererror, "\n", "".join(e[2][3:])

	for e in xerrorlist:
		print e[0], e[1], "\n", "".join(e[2][3:])

# Output totals again
if error_count != 0:
	print "Errors:  " + str(error_count) + " out of " + str(total_count),
	if str(total_count) == "1":
		print " case", 
	else:
		print " cases", 
        print

# Output totals by language
if error_count != 0:
        list = []
        for e in errorlist:
                list.append(str.split(e[0])[1])

        # TODO:  Replace with Counter from collections
        n = list.count("C++")
        if n != 0:
                print "Errors:  C++  " + str(n)
        
        n = list.count("C#")
        if n != 0:
                print "Errors:  C#   " + str(n)
        
        n = list.count("C")
        if n != 0:
                print "Errors:  C    " + str(n)
        
        n = list.count("Java")
        if n != 0:
                print "Errors:  Java " + str(n)
        
# output tool errors counts
print
if len(sperrorlist) == 0:
	print "No tool errors"
else:
	print "Tool errors:  " + str(len(sperrorlist))
	print "Tool Errorlist:"
	for e in sperrorlist:
		f.write(str(e[0]) + " " + str(e[1]) + " " + str(e[2]) + " " + str(e[3]) + "\n")
		print e[0], e[1], e[2], e[3]

current_time = datetime.now()
os.rename(error_filename, error_filename + "_" + current_time.isoformat().replace(":", "-") + error_filename_extension)
f.close()


# output tool version
print
if use_exec :
        print src2srcmlversion(), srcmltranslator
        print srcml2srcversion(), srcmlutility
else :
        print srcml_version_string()

exit
