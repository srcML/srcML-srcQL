/*
  srcMLReader.cpp

  Copyright (C) 2013  SDML (www.sdml.info)

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
*/

#include <srcMLReader.hpp>
#include <../srcmlns.hpp>
#include <srcml.h>

#include <stdlib.h>
#include <string.h>

void output_node_srcml(const xmlNode & node, xmlTextWriterPtr writer, bool is_root);

/**
 * getNode
 * @reader an XML text reader
 *
 * Get a copy of the current node with attributes set.
 */
xmlNodePtr getNode(xmlTextReaderPtr reader) {

  xmlNodePtr node = xmlCopyNode(xmlTextReaderCurrentNode(reader), 2);
  node->extra = xmlTextReaderIsEmptyElement(reader);
  node->type = (xmlElementType)xmlTextReaderNodeType(reader);

  return node;
}

/**
 * freeNode
 * @node an XML node
 * 
 * Free the current node.  Text is not allocated and do not free
 */
void freeNode(xmlNodePtr node) {

  if(node && (xmlReaderTypes)node->type != XML_READER_TYPE_TEXT
     && (xmlReaderTypes)node->type != XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
    xmlFreeNode(node);
}


/**
 * srcMLReader
 * @filename name of a file
 *
 * Construct a srcMLReader using a filename
 */
srcMLReader::srcMLReader(const char * filename)
  : is_archive(false), is_single(false), done(false) {

  if(filename == NULL) throw std::string();

  reader = xmlNewTextReaderFilename(filename);
  if(reader == NULL) throw std::string();
    xmlTextReaderRead(reader);
  node = getNode(reader);
}

/**
 * srcMLReader
 * @input parser input buffer
 *
 * Construct a srcMLReader using a parser input buffer
 */
srcMLReader::srcMLReader(xmlParserInputBufferPtr input)
  : is_archive(false), is_single(false), done(false) {

  if(input == NULL) throw std::string();
  reader = xmlNewTextReader(input, 0);
  if(reader == NULL) throw std::string();
  xmlTextReaderRead(reader);
  node = getNode(reader);
}

/**
 * ~srcMLReader
 *
 * Destructor a srcMLReader
 */
srcMLReader::~srcMLReader() {

  xmlFreeTextReader(reader);
  reader = 0;

}

/**
 * readAttributesInternal
 * @language a location to store the language attribute
 * @filename a location to store the filename attribute
 * @directory a location to store the directory attribute
 * @version a location to store the version attribute
 *
 * Internal function to read unit attributes.
 */
int srcMLReader::readUnitAttributesInternal(std::string ** language, std::string ** filename,
                                            std::string ** directory, std::string ** version) {

  if(language == 0 || filename == 0 || directory == 0 || version == 0) return 1;

  xmlAttrPtr attribute = node->properties;
  while (attribute) {

    std::string name = (const char *)attribute->name;
    try {

      if(name == "language")
        (*language) = new std::string((const char *)attribute->children->content);
      else if(name == "filename")
        (*filename) = new std::string((const char *)attribute->children->content);
      else if(name == "dir")
        (*directory) = new std::string((const char *)attribute->children->content);
      else if(name == "version")
        (*version) = new std::string((const char *)attribute->children->content);

    } catch(...) {

      if(*language) delete *language, (*language) = 0;
      if(*filename) delete *filename, (*filename) = 0;
      if(*directory) delete *directory, (*directory) = 0;
      if(*version) delete *version, (*version) = 0;
      return 1;

    }

    attribute = attribute->next;
  }

  return 0;

}

/**
 * readRootUnitAttributes
 * @language a location to store the language attribute
 * @filename a location to store the filename attribute
 * @directory a location to store the directory attribute
 * @version a location to store the version attribute
 * @attributes array to store other attributes gathered
 * @prefixes an array to store gathered XML namespace prefixes
 * @namepaces an array to store gathered XML naamespaces
 * @options a variable to set used options
 * @tabstop a variable to set the tabstop
 *
 * Read attributes and namespace information fromt the root unit,
 * setting the necessary options.
 */
int srcMLReader::readRootUnitAttributes(std::string ** language, std::string ** filename,
                                        std::string ** directory, std::string ** version,
                                        std::vector<std::string> & attributes,
                                        std::vector<std::string> & prefixes,
                                        std::vector<std::string> & namespaces,
                                        OPTION_TYPE & options,
                                        int & tabstop) {

  if(language == 0 || filename == 0 || directory == 0 || version == 0) return 0;

  if(done) return 0;

  // forward to start unit
  while(true) {
    if(node && (xmlReaderTypes)node->type == XML_READER_TYPE_ELEMENT && strcmp((const char *)node->name, "unit") == 0)
      break;

    if(xmlTextReaderRead(reader) != 1) { done = true; return 0; }
    freeNode(node);
    node = getNode(reader);
  }

  xmlAttrPtr attribute = node->properties;
  while (attribute) {
    std::string name = (const char *)attribute->name;

    try {

      if(name == "language")
        (*language) = new std::string((const char *)attribute->children->content);
      else if(name == "filename")
        (*filename) = new std::string((const char *)attribute->children->content);
      else if(name == "dir")
        (*directory) = new std::string((const char *)attribute->children->content);
      else if(name == "version")
        (*version) = new std::string((const char *)attribute->children->content);
      else if(name == "tabs")
        tabstop = atoi((const char *)attribute->children->content);
      else {

        attributes.push_back(name);
        attributes.push_back((const char *)attribute->children->content);

      }

    } catch(...) {

      if(*language) delete *language, (*language) = 0;
      if(*filename) delete *filename, (*filename) = 0;
      if(*directory) delete *directory, (*directory) = 0;
      if(*version) delete *version, (*version) = 0;
      return 0;

    }

    attribute = attribute->next;
  }

  xmlNsPtr xmlns = node->nsDef;

  while(xmlns) {

    std::string prefix = xmlns->prefix ? (const char *)xmlns->prefix : "";
    std::string ns = xmlns->href ? (const char *)xmlns->href : "";

    if(ns == SRCML_CPP_NS_URI)
      options |= SRCML_OPTION_CPP;
    else if(ns == SRCML_ERR_NS_URI)
      options |= SRCML_OPTION_DEBUG;
    else if(ns == SRCML_EXT_LITERAL_NS_URI)
      options |= SRCML_OPTION_LITERAL;
    else if(ns == SRCML_EXT_OPERATOR_NS_URI)
      options |= SRCML_OPTION_OPERATOR;
    else if(ns == SRCML_EXT_MODIFIER_NS_URI)
      options |= SRCML_OPTION_MODIFIER;
    else if(ns == SRCML_EXT_POSITION_NS_URI)
      options |= SRCML_OPTION_POSITION;

    int i;
    try {

      for(i = 0; i < prefixes.size(); ++i)

        if(namespaces.at(i) == ns) {

          prefixes.at(i) = prefix;
          break;
        }

    } catch(...) {}

    if(i == prefixes.size()) {
      prefixes.push_back(prefix);
      namespaces.push_back(ns);
    }

    xmlns = xmlns->next;
  }

  return 1;
}

/**
 * readUnitAttributes
 * @language a location to store the language attribute
 * @filename a location to store the filename attribute
 * @directory a location to store the directory attribute
 * @version a location to store the version attribute
 *
 */
int srcMLReader::readUnitAttributes(std::string ** language, std::string ** filename,
                                    std::string ** directory, std::string ** version) {

  if(language == 0 || filename == 0 || directory == 0 || version == 0) return 0;

  bool read_unit_start = false;

  if(!save_nodes.empty()) {

    try {

      for(int i = 0; i < save_nodes.size(); ++i)
        freeNode(save_nodes.at(i));

    } catch(...) {}

    save_nodes.clear();

  }

  if(done) return 0;

  // forward to start unit
  while(true) {
    if(node && (xmlReaderTypes)node->type == XML_READER_TYPE_ELEMENT && strcmp((const char *)node->name, "unit") == 0)
      break;

    if(xmlTextReaderRead(reader) != 1) { done = true; return 0; }
    freeNode(node);
    node = getNode(reader);
  }

  if(readUnitAttributesInternal(language, filename, directory, version)) return 0;
  if(xmlTextReaderRead(reader) != 1) { freeNode(node); done = true; return 0; }

  save_nodes.push_back(node);
  node = getNode(reader);

  if(is_archive) return 1;

  while(true) {

    save_nodes.push_back(node);

    if(node->type == (xmlElementType)XML_READER_TYPE_ELEMENT && strcmp((const char *)node->name, "unit") == 0) {

      is_archive = true;
      try {
        for(int i = 0; i < save_nodes.size() - 1; ++i)
          freeNode(save_nodes.at(i));

      } catch(...) {}
      save_nodes.clear();
      save_nodes.push_back(node);
      if(*language) delete *language, (*language) = 0;
      if(*filename) delete *filename, (*filename) = 0;
      if(*directory) delete *directory, (*directory) = 0;
      if(*version) delete *version, (*version) = 0;
      if(readUnitAttributesInternal(language, filename, directory, version)) return 0;
      break;
    }

    if(node->type == (xmlElementType)XML_READER_TYPE_ELEMENT
       && strcmp((const char *)node->name, "unit") != 0) {
      is_archive = true;
      is_single = true;
      break;
    }

    if(xmlTextReaderRead(reader) != 1) {done = true; return 0; }
    node = getNode(reader);

  }

  readUnitAttributesInternal(language, filename, directory, version);
  if(xmlTextReaderRead(reader) != 1) { done = true; return 0; }
  node = getNode(reader);

  return 1;

}

/**
 * readsrcML
 * @writer an XML text writer
 *
 * Read the next unit of a srcML Archive.
 * and write it to the writer.
 *
 * Return 0 when finished and 1 otherwize.
 */
int srcMLReader::readsrcML(xmlTextWriterPtr writer) {

  if(done) return 0;

  //xmlTextWriterStartDocument(writer, XML_VERSION, xml_encoding, XML_DECLARATION_STANDALONE);
  bool read_unit_start = false;

  if(!save_nodes.empty()) {

    try {

      for(int i = 0; i < save_nodes.size(); ++i)
        output_node_srcml(*save_nodes.at(i), writer, is_single);

      for(int i = 0; i < save_nodes.size(); ++i)
        freeNode(save_nodes.at(i));

    } catch(...) {}

    save_nodes.clear();

  } else {

    // forward to start unit
    while(true) {
      if(node && (xmlReaderTypes)node->type == XML_READER_TYPE_ELEMENT && strcmp((const char *)node->name, "unit") == 0)
        break;

      if(xmlTextReaderRead(reader) != 1) {done = true; return 0; }
      freeNode(node);
      node = getNode(reader);
    }

  }

  while(true) {

    if(is_archive) output_node_srcml(*node, writer, is_single);
    else save_nodes.push_back(node);

    if(strcmp((const char *)node->name, "unit") == 0) {

      if(node->type == (xmlElementType)XML_READER_TYPE_ELEMENT) {

        if(read_unit_start) {

          is_archive = true;
          try {

            for(int i = 0; i < save_nodes.size() - 1; ++i)
              freeNode(save_nodes.at(i));

          } catch(...) {}

          save_nodes.clear();
          output_node_srcml(*node, writer, is_single);

        }

        read_unit_start = true;
      }

      if(node->type == (xmlElementType)XML_READER_TYPE_END_ELEMENT) {
        break;
      }

    }

    if(!save_nodes.empty() && node->type == (xmlElementType)XML_READER_TYPE_ELEMENT
       && strcmp((const char *)node->name, "unit") != 0) {

      is_archive = true;
      is_single = true;

      try {

        for(int i = 0; i < save_nodes.size(); ++i)
          output_node_srcml(*save_nodes.at(i), writer, is_single);

        for(int i = 0; i < save_nodes.size() - 1; ++i)
          freeNode(save_nodes.at(i));

      } catch(...) {}

      save_nodes.clear();

    }

    if(is_archive) freeNode(node);
    if(xmlTextReaderRead(reader) != 1) {done = true; return 0; }
    node = getNode(reader);

  }

  if(is_archive) freeNode(node);
  node = 0;

  xmlTextWriterEndDocument(writer);

  return 1;

}

/**
 * readsrcML
 * 
 * Read the next unit from a srcML Archive
 * and return it as a std::string. Uses
 * readsrcML(xmlTextWriterPtr writer).
 *
 * When finished return a 0.
 */
std::string * srcMLReader::readsrcML() {

  if(done) return 0;

  xmlBufferPtr buffer = xmlBufferCreate();
  xmlTextWriterPtr writer = xmlNewTextWriterMemory(buffer, 0);
  int status = readsrcML(writer);

  if(!status) return 0;

  int length = buffer->use;

  while(length > 0 && buffer->content[length - 1] == '\n')
    --length;

  std::string * unit = 0;
  try {

    unit = new std::string((const char *)buffer->content, length);

  } catch(...) {}

  xmlFreeTextWriter(writer);
  xmlBufferFree(buffer);

  return unit;

}

/**
 * output_node_srcml
 * @node an XML node
 * @writer an XML text writer
 * @is_root boolean indicating if parsing root unit
 *
 * Output node as srcML.
 */
void output_node_srcml(const xmlNode & node, xmlTextWriterPtr writer, bool is_root) {

  bool isemptyelement = false;

  switch (node.type) {
  case XML_READER_TYPE_ELEMENT:

    // record if this is an empty element since it will be erased by the attribute copying
    isemptyelement = node.extra & 0x1;

    // start the element
    {

      std::string s = "";
      if(node.ns && node.ns->prefix) {

        s += ((char*) node.ns->prefix);
        s += ":";
      }
      s += (char*) node.name;

      xmlTextWriterStartElement(writer, BAD_CAST s.c_str());

    }

    if(strcmp((const char *)node.name, "unit") == 0) {

      xmlNsPtr xmlns = node.nsDef;
      while(xmlns) {

        std::string ns = xmlns->href ? (const char *)xmlns->href : "";
        if(ns != SRCML_CPP_NS_URI) {

          xmlns = xmlns->next;
          continue;

        }

        std::string prefix = "xmlns";
        if(xmlns->prefix) {

          prefix += ":";
          prefix += (const char *)xmlns->prefix;

        }

        xmlTextWriterWriteAttribute(writer, (const xmlChar *)prefix.c_str(), (const xmlChar *)ns.c_str());

        xmlns = xmlns->next;
      }
    }

    // copy all the attributes
    {
      xmlAttrPtr attribute = node.properties;
      while (attribute) {

        std::string s;
        if(attribute->ns && attribute->ns->prefix) {
          s = (const char *)attribute->ns->prefix;
          s += ":";

        }
        s += (const char *)attribute->name;

        xmlTextWriterWriteAttribute(writer, (const xmlChar *)s.c_str(), (const xmlChar *)attribute->children->content);
        attribute = attribute->next;
      }
    }

    // end now if this is an empty element
    if (isemptyelement) {

      xmlTextWriterEndElement(writer);
    }

    break;

  case XML_READER_TYPE_END_ELEMENT:
    xmlTextWriterEndElement(writer);
    break;

  case XML_READER_TYPE_COMMENT:
    xmlTextWriterWriteComment(writer, (const xmlChar *)node.content);
    break;

  case XML_READER_TYPE_TEXT:
  case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:

    // output the UTF-8 buffer escaping the characters.  Note that the output encoding
    // is handled by libxml
    for (unsigned char* p = (unsigned char*) node.content; *p != 0; ++p) {
      if (*p == '&')
        xmlTextWriterWriteRawLen(writer, BAD_CAST (unsigned char*) "&amp;", 5);
      else if (*p == '<')
        xmlTextWriterWriteRawLen(writer, BAD_CAST (unsigned char*) "&lt;", 4);
      else if (*p == '>')
        xmlTextWriterWriteRawLen(writer, BAD_CAST (unsigned char*) "&gt;", 4);
      else
        xmlTextWriterWriteRawLen(writer, BAD_CAST (unsigned char*) p, 1);
    }
    break;

  default:
    break;
  }
}
