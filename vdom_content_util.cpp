#include <pcre.h>
#include <stdio.h>

#include "vdom_content_util.h"
namespace vdom {
namespace content {

#define OVECCOUNT 60    /* should be a multiple of 3 */

static pcre *copyright_re = NULL;
static const char *copyright_pat = "(?:copyright|版权|友情链接|免责申明|法律申明|联系我们|注册|详情请看|导航|首页|>|备案|打印本页|下一页|免费服务|文章来源|版权|ICP|©)";

bool Util::contain_copyright_text(const std::string &text) {
    if (copyright_re == NULL) {
        const char *error;
        int erroffset;
        //copyright_re = pcre_compile(copyright_pat, PCRE_CASELESS | PCRE_UTF8, &error, &erroffset, NULL);
        copyright_re = pcre_compile(copyright_pat, PCRE_CASELESS, &error, &erroffset, NULL);
        if (copyright_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }
    int ovector[OVECCOUNT];
    int rc = pcre_exec(copyright_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "Matching error %d\n", rc); break;
        }
    } else {
        return true;
    }

    return false;
}

/*
static void split_text(const std::string &text, std::vector<std::string> &str_vec, unsigned char sep = '\t') {

}

*/

void
Util::normalize_content(const std::string &raw, std::string &normalized) {
    const char *p = raw.c_str();
    int size = raw.size();
    bool pre_is_space = false;
    for (int i = 0; i < size; i++) {
        char c = *(p+i);
        if (isspace(c)) {
            if (pre_is_space) {
                // do nothihng
            } else {
                pre_is_space = true;
                normalized.append(" ");
            }
        } else {
            pre_is_space = false;
            normalized.append(1, c);
        }
    }
}

} // namespace content
} // namespace vdom
