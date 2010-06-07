/*
 *
 *
 */

#ifndef VDOM_CONTENT_RESULT_H

#include <list>
#include <string>
#include <iostream>

namespace vdom {
namespace content {

// TODO image src and around text
struct ImageResult {
    std::string src;
    std::string around_text;
};

struct Result
{
    bool extracted_okay;
    float list_confidence;
    std::string raw_title;
    std::string title;
    std::string content;
    std::list<ImageResult> images;

    void debug_print() {
        std::cout << "okay: " << extracted_okay << std::endl;
        std::cout << "title: " << title << std::endl;
        std::cout << "content: " << content << std::endl;
    }
};

} //namespace vdom
} //namespace content

#endif
