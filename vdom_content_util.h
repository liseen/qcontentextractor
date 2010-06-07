#ifndef VDOM_CONTENT_UTIL_H

#include <string>
#include <vector>

namespace vdom {
namespace content {

class Util {
public:
    static bool contain_copyright_text(const std::string &text);

    static void split_text(const std::string &text, std::vector<std::string> &str_vec, unsigned char sep = '\t');

    //static void normalize_content(const std::string &raw, std::string &nomalized);

    //static int utf8_to_utf16(const char *str, size_t size, int &char_size);
};

}
}

#endif
