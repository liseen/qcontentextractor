/*
 *
 *
 */

#ifndef VDOM_CONTENT_EXTRACTOR_H

#include <list>
#include <vdom.h>

#include "vdom_content_block.h"
#include "vdom_content_result.h"

#include "ddebug.h"

namespace vdom {
namespace content {

typedef std::list<TextBlock> TextBlockList;
typedef std::list<TextBlock>::iterator TextBlockIter;

typedef std::list<Node*> RepeatGroup;
typedef std::list<Node*>::iterator RepeatGroupIter;
typedef std::list<RepeatGroup> RepeatGroupList;
typedef std::list<RepeatGroup>::iterator RepeatGroupListIter;

class Extractor
{
public:
    Extractor(bool need_urls = false, bool debug = false) : m_debug(debug), m_need_urls(need_urls) {
    }
    ~Extractor() {
    }

    void set_need_urls(bool need_urls);
    void extract_urls(vdom::Node *body, std::list<std::string> &urls);
    bool extract(vdom::Window *window, Result &result);

private:
    bool extract_block_list(Node* node, TextBlockList  &block_list);
    void tag_block(TextBlock &block);
    bool check_is_noise(Node *node);
    bool select_good_block(TextBlockList &block_list);
    bool expand_good_block(TextBlockList &block_list);
    bool merge_content_block(TextBlockList &block_list, Result &result);

    bool extract_repeat_groups(RepeatGroupList &groups, Node* node);
    int compute_list_confidence(vdom::Document *doc, RepeatGroupList &groups);
    bool is_link_group(RepeatGroup &group);
private:
    bool prev_is_noise;

private:
    bool m_debug;
    bool m_need_urls;

};

} //namespace vdom
} //namespace content

#endif
