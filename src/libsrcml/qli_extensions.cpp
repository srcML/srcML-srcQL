// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file qli_extensions.cpp
 *
 * @copyright Copyright (C) 2023 srcML, LLC. (www.srcML.org)
 */

#include "qli_extensions.hpp"
#include "unification_table.hpp"

#include <string>
#include <string_view>
#include <set>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <memory>

// std::string_view literals, e.g., "abc"sv
using namespace ::std::literals::string_view_literals;

thread_local UnificationTable table;

// std::unique_ptr deleter functions for libxml2
// usage: std::unique<xmlFree> p(xmlReadMemory);
// Call p.get() for original pointer
// Will deallocate automatically at end of std::unique_ptr lifetime
namespace std {
    template<>
    struct default_delete<xmlChar> {
        void operator()(xmlChar* s) { xmlFree(s); }
    };
    template<>
    struct default_delete<xmlNodeSet> {
        void operator()(xmlNodeSet* nodes) { xmlXPathFreeNodeSet(nodes); }
    };
};

std::string_view trim_whitespace(std::string_view str) {

    const auto first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";

    const auto last = str.find_last_not_of(" \t\n\r");

    return str.substr(first, (last - first + 1));
}

void get_node_text(const xmlNode* top_node, std::string& text, bool top) {

    for (const xmlNode* node = top_node; node != NULL && (!top || node == top_node); node = node->next) {
        if (node->type == XML_TEXT_NODE) {
            xmlChar* t = xmlNodeGetContent(node);
            text += trim_whitespace((const char*) t);
            text += ' ';
            xmlFree(t);
        }
        if (node->children) {
            get_node_text(node->children, text, false);
        }
    }
}

std::string get_node_text(const xmlNode* top_node) {

    std::string s;
    get_node_text(top_node, s, true);
    return s;
}

void add_element(xmlXPathParserContext* ctxt, int nargs) {

    if (nargs < 3 || nargs > 5) {
        std::cerr << "Arg arity error" << std::endl;
        return;
    }

    // postfix
    std::string_view postfix;
    std::unique_ptr<xmlChar> ctxtPostfix;
    if (nargs == 5) {
        ctxtPostfix.reset(xmlXPathPopString(ctxt));
        postfix = (const char*)(ctxtPostfix.get());
    }

    // prefix
    std::string_view prefix;
    std::unique_ptr<xmlChar> ctxtPrefix;
    if (nargs >= 4) {
        ctxtPrefix.reset(xmlXPathPopString(ctxt));
        prefix = (const char*)(ctxtPrefix.get());
    }

    // order ?
    size_t number = (size_t) xmlXPathPopNumber(ctxt);

    // bucket name
    std::unique_ptr<xmlChar> ctxtBucket(xmlXPathPopString(ctxt));
    const std::string_view bucket = (char*)(ctxtBucket.get());

    // token
    std::unique_ptr<xmlNodeSet> node_set(xmlXPathPopNodeSet(ctxt));
    const xmlNode* node = node_set.get()->nodeTab[0];
    const std::string token(trim_whitespace(get_node_text(node)));
    const auto node_ptr = reinterpret_cast<std::uintptr_t>(node);

    // @NOTE Add comment
    table.add_to_variable_bucket(bucket);
    if (table.size_of_variable_bucket(bucket) < number) {
        table.add_to_number_bucket(bucket, number);
    }

    // clear all but the first bucket
    const size_t last = table.get_last_used(bucket);
    const auto size = table.size_of_variable_bucket(bucket);
    if (number == 1 && last == size) {
        // clear all but the first bucket
        for (size_t i = 1; i < size; ++i) {
            table.clear_token_list(bucket, i);
        }
    }
    table.set_last_used(bucket, number);

    // handle token via std::string_view for efficent trimming
    std::string_view tokenView(token);

    // remove prefix
    // @ASSUMPTION Prefix has to start at the beginning
    if (!prefix.empty() && (token.size() < prefix.size() || token.compare(0, prefix.size(), prefix) != 0)) {
        xmlXPathReturnBoolean(ctxt, false);
        return;
    }
    tokenView.remove_prefix(prefix.length());

    // remove postfix
    // @ASSUMPTION Postfix has to start at the beginning
    if (!postfix.empty() && (token.size() < postfix.size() || token.compare(token.size() - postfix.size(), postfix.size(), postfix) != 0)) {
        xmlXPathReturnBoolean(ctxt, false);
        return;
    }
    tokenView.remove_suffix(postfix.length());

    // @NOTE Add comment
    const bool valid = table.does_element_match_variable(bucket, number, tokenView, node_ptr);
    if (valid) {
        table.add_to_token_list(bucket, number, tokenView, node_ptr);
    }

    xmlXPathReturnBoolean(ctxt, valid);
}

void clear_elements(xmlXPathParserContext* ctxt, int nargs) {

    if (nargs > 1) {
        std::cerr << "Arg arity error" << std::endl;
        return;
    }

    if (nargs == 0) {

        // clear all buckets
        table.empty_buckets();

    } else if (nargs == 1) {

        // clear this bucket
        xmlChar* var = xmlXPathPopString(ctxt);
        table.empty_bucket((const char*) var);
        xmlFree(var);
    }

    xmlXPathReturnBoolean(ctxt, true);
}

void is_valid_element(xmlXPathParserContext* ctxt, int nargs) {

    if (nargs != 1) {
        std::cerr << "Arg arity error" << std::endl;
        return;
    }

    const std::unique_ptr<xmlNodeSet> node_set(xmlXPathPopNodeSet(ctxt));
    // @TODO Might there be more than one node?
    assert(node_set.get()->nodeNr == 1);
    const xmlNode* node = node_set.get()->nodeTab[0];

    // check for invalid elements
    const std::string_view nodeURI((char *) node->ns->href);
    const std::string_view nodeName((char*) node->name);
    const bool invalidElement = "http://www.srcML.org/srcML/src"sv == nodeURI && (
                                "operator"sv == nodeName ||
                                "comment"sv == nodeName ||
                                "modifier"sv == nodeName ||
                                "specifier"sv == nodeName);

    xmlXPathReturnBoolean(ctxt, !invalidElement);
}