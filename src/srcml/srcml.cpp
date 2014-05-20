/**
 * @file srcml.cpp
 *
 * @copyright Copyright (C) 2014 SDML (www.srcML.org)
 *
 * This file is part of the srcml command-line client.
 *
 * The srcML Toolkit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The srcML Toolkit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the srcml command-line client; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <srcml.h>
#include <srcml_cli.hpp>
#include <srcml_options.hpp>
#include <create_srcml.hpp>
#include <decompress_srcml.hpp>
#include <compress_srcml.hpp>
#include <create_src.hpp>
#include <transform_srcml.hpp>
#include <srcml_display_metadata.hpp>
#include <srcml_execute.hpp>
#include <isxml.hpp>
#include <peek4char.hpp>

#include <archive.h>
#include <iostream>

int main(int argc, char * argv[]) {

    // parse the command line
    srcml_request_t srcml_request = parseCLI(argc, argv);

    // version
    if (srcml_request.command & SRCML_COMMAND_VERSION) {
        std::cout <<  srcml_version_string() << "\n";
        std::cout << "srcml Version Number " << srcml_version_number() << "\n";
        std::cout << "libarchive Version " << ARCHIVE_VERSION_NUMBER << "\n";
        return 0;
    }

    // setup global access to options
    SRCMLOptions::set(srcml_request.command);

    // convert the list of input filenames to input sources
    srcml_input_t input_sources(srcml_request.input.begin(), srcml_request.input.end());

    // for single input src archives (e.g., .tar), filename attribute is the source filename
    if (input_sources.size() == 1 && input_sources[0].archives.size() > 0) {
        srcml_request.att_filename = input_sources[0].filename;
    }

    // standard input handled as FILE* to be able to peek
    if (srcml_request.stdindex) {

        srcml_input_src* pstdin = &input_sources[*srcml_request.stdindex];

        // FILE* becomes part of stdin input source
        pstdin->fileptr = fdopen(STDIN_FILENO, "r");
        pstdin->fd = boost::none;

        // peek at the first 4 bytes
        unsigned char data[4];
        ssize_t size = 0;
        peek4char(*(pstdin->fileptr), data, &size);

        // determine if the input is srcML or src from the first up-to 4 bytes
        pstdin->state = isxml(data, size) ? SRCML : SRC;

        // language is required when standard input is used for source
        if ((pstdin->state == SRC) && !srcml_request.att_language) {
            std::cerr << "Using stdin requires a declared language\n";
            exit(1);
        }
    }

    // output destination setup just like an input source
    srcml_output_dest destination(srcml_request.output_filename ? *srcml_request.output_filename : "");

    /*
        Determine what processing needs to occur based on the inputs, outputs, and commands
    */

    // setup the commands in the pipeline
    processing_steps_t pipeline;

    // src->srcml when there is any src input, or multiple srcml input with output to srcml (merge)
    if (std::find_if(input_sources.begin(), input_sources.end(), is_src) != input_sources.end() ||
        (input_sources.size() > 1 && destination.state == SRCML)) {

        pipeline.push_back(create_srcml);
    }

    // XPath and XSLT processing
    if (!srcml_request.transformations.empty()) {

        pipeline.push_back(transform_srcml);
    }

    // output stage of metadata on srcML
    if ((srcml_request.command & SRCML_COMMAND_INSRCML) || srcml_request.unit > 0) {

        pipeline.push_back(srcml_display_metadata);
    }
    // output stage of src, based on the destination
    else if (destination.state != SRCML) {

        pipeline.push_back(create_src);
    }

    // libsrcml can apply gz compression
    // all other compressions require an additional compression stage
    if ((destination.compressions.size() > 1) ||
        (destination.compressions.size() == 1 && destination.compressions.front() != ".gz")) {

#if ARCHIVE_VERSION_NUMBER > 3001002
        pipeline.push_back(compress_srcml);
#else
        std::cerr << "Unsupported output compression\n";
        exit(1);
#endif
    }

    assert(!pipeline.empty());

    // execute the steps in order
    srcml_execute(srcml_request, pipeline, input_sources, destination);

    srcml_cleanup_globals();

    return 0;
}
