/*
 *
 *
 */

#ifndef VDOM_CONTENT_RESULT_H

#include <list>
#include <string>
#include <iostream>

#include "vdom_content_util.h"

namespace vdom {
namespace content {

// TODO image src and around text
struct ImageResult {
    std::string src;
    std::string alt;
    std::string around_text;
};

struct Result
{
    Result() {
        extracted_okay = false;
        list_confidence = 0; // %100
        content_confidence = 0; // %100
    }

    bool extracted_okay;
    int list_confidence; // %100
    int content_confidence; // %100
    std::string raw_title;
    std::string title;
    std::string keywords;
    std::string description;
    std::string content;
    std::list<std::string> urls;
    std::list<ImageResult> images;

    void debug_print() {
        std::cout << "okay: " << extracted_okay << std::endl;
        std::cout << "list_confidence: " << list_confidence << std::endl;
        std::cout << "content_confidence: " << content_confidence << std::endl;
        std::cout << "title: " << title << std::endl;
        std::cout << "keywords: " << keywords << std::endl;
        std::cout << "description: " << description << std::endl;
        std::cout << "content: " << content << std::endl;
        std::string normal_content;
        Util::normalize_content(content, normal_content);
        std::cout << "normal_content: " << normal_content << std::endl;
    }
};

} //namespace vdom
} //namespace content

#endif
