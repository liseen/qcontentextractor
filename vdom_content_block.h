/*
 *
 *
 */

#ifndef VDOM_CONTENT_BLOCK_H

#include <string>
#include <vdom.h>

#include "ddebug.h"

namespace vdom {
namespace content {

struct TextBlock {
    TextBlock() {
        _node = NULL;

        _has_anchor_length = false;
        _has_anchor_ratio = false;
        _has_tag_density = false;
        _has_punct_count = false;
        _has_punct_ratio = false;
        _has_space_ratio = false;
        _has_contain_outside_link = false;
        _has_contain_advertise_link = false;

        _contain_outside_link = false;
        _contain_advertise_link = false;
        _is_good = false;
        _is_bad = false;
        _is_content = false;
        _prev_is_noise = false;
        _next_is_noise = false;
    }

public:
    inline bool has_anchor_length() {
        return _has_anchor_length;
    }

    int compute_anchor_length(Node *cnode) {
        if (cnode->tag_name() == "A") {
            return cnode->content().size();
        }

        int al = 0;
        int child_size = cnode->child_nodes_size();
        for (int i = 0; i < child_size; ++i) {
            Node *child_node = cnode->mutable_child_nodes(i);
            if (child_node->tag_name() == "A") {
                al += child_node->content().size();
            } else if (child_node->type() == Node::ELEMENT) {
                al += compute_anchor_length(child_node);
            }
        }

        return al;
    }

    int anchor_length() {
        if (has_anchor_length()) {
            return _anchor_length;
        } else {
            _has_anchor_length = true;
            _anchor_length = compute_anchor_length(_node);
            return _anchor_length;
        }
    }

    inline bool has_anchor_ratio() {
        return _has_anchor_ratio;
    }

    inline int anchor_ratio() {
        if (has_anchor_ratio()) {
            return _anchor_ratio;
        } else {
            _has_anchor_ratio = true;
            size_t total = _node->content().size();
            if ( total == 0) {
                _anchor_ratio = 0;
            }  else {
                _anchor_ratio = 100 * anchor_length() / total;
            }
            return _anchor_ratio;
        }
    }

    inline bool has_tag_density() {
        return _has_tag_density;
    }

    int compute_tag_count(Node* cnode) {
        int tc = 0;
        int child_size = cnode->child_nodes_size();
        tc += child_size;
        for (int i = 0; i < child_size; ++i) {
            if (cnode->child_nodes(i).type() == Node::ELEMENT) {
                tc += compute_tag_count(cnode->mutable_child_nodes(i));
            }
        }
        return tc;
    }

    float tag_density() {
        if (has_tag_density()) {
            return _tag_density;
        } else {
            _has_tag_density = true;
            int tag_cnt = compute_tag_count(_node);
            size_t total = _node->content().size();
            if ( total == 0) {
                _tag_density = 0;
            }  else {
                _tag_density = (float)tag_cnt / total;
            }
            return _tag_density;
        }
    }

    inline bool has_punct_count() {
        return _has_punct_count;
    }

    int punct_count() {
        if (has_punct_count()) {
            return _punct_count;
        } else {
            _has_punct_count = true;
            // TODO
            _punct_count = 1;
        }
    }

    inline bool has_punct_ratio() {
        return _has_punct_ratio;
    }

    int punct_ratio() {
        if (has_punct_ratio()) {
            return _punct_ratio;
        } else {
            _has_punct_ratio = true;

            if (_node->content().size() == 0) {
                _punct_ratio = 0;
            } else {
                _punct_ratio = punct_count() / _node->content().size();
            }

            return _punct_ratio;
        }
    }

    inline int content_size() {
        return _node->content().size();
    }

    inline bool has_space_ratio() {
        return _has_space_ratio;
    }

    int space_count() {
        int cnt = 0;
        const char *p = _node->content().c_str();
        int size = _node->content().size();
        for (int i = 0; i < size; i++) {
            switch (*(p + i)) {
                case ' ':
                case '\t':
                case '\n':
                    ++cnt;
                    break;
            }
        }

        return cnt;
    }

    int space_ratio() {
        if (has_space_ratio()) {
            return _space_ratio;
        } else {
            _has_space_ratio = true;

            if (content_size() == 0) {
                _space_ratio = 0;
            } else {
                _space_ratio = 100 * space_count() / content_size();
            }

            return _space_ratio;
        }
    }

    inline bool has_contain_outside_link() const {
        return _has_contain_outside_link;
    }

    inline bool contain_outside_link() const {
        if (has_contain_outside_link()) {
            return _contain_outside_link;
        } else {
            if (_node->tag_name() ==  "A") {
            } else {

            }
        }
    }

    inline bool is_good() {
        return _is_good;
    }
    inline void set_is_good(bool is) {
        _is_good = is;
    }

    inline bool is_content() {
        return _is_content;
    }
    inline void set_is_content(bool is) {
        _is_content = is;
    }

    inline bool is_bad() {
        return _is_bad;
    }

    inline void set_is_bad(bool is) {
        _is_bad = is;
    }

    inline bool prev_is_noise() {
        return _prev_is_noise;
    }

    inline void set_prev_is_noise(bool is) {
        _prev_is_noise = is;
    }

    inline bool next_is_noise() {
        return _next_is_noise;
    }

    inline void set_next_is_noise(bool is) {
        _next_is_noise = is;
    }

    inline Node* node() {
        return _node;
    }

    inline void set_node(Node *n) {
        _node = n;
    }

    void debug_print() {
        if (_node == NULL) {
            std::cout << "content: " << "node is null" << std::endl;
        } else {
            std::cout << "type: " << _node->type() << std::endl;
            std::cout << "tagname: " << _node->tag_name() << std::endl;
            std::cout << "allchildreninline: " << _node->all_children_inline() << std::endl;
            std::cout << "x: " << _node->x() << std::endl;
            std::cout << "y: " << _node->y() << std::endl;
            std::cout << "w: " << _node->w() << std::endl;
            std::cout << "h: " << _node->h() << std::endl;
            std::cout << "is_good: " << is_good() << std::endl;
            std::cout << "is_bad: " << is_bad() << std::endl;
            std::cout << "prev_is_noise: " << prev_is_noise() << std::endl;
            std::cout << "next_is_noise: " << next_is_noise() << std::endl;
            std::cout << "is_content: " << is_content() << std::endl;
            std::cout << "content_size: " << content_size() << std::endl;
            std::cout << "content: " << _node->content() << std::endl;
            std::cout << "tag_desity: " << tag_density() << std::endl;
            std::cout << "space_ratio " << space_ratio() << std::endl;
            std::cout << "anchor_length: " << anchor_length() << std::endl;
            std::cout << "anchor_ratio: " << anchor_ratio() << std::endl;
            std::cout << "repeat_sig: " << _node->repeat_sig() << std::endl;
        }
    }
private:
    // ratio ~[0-100]
    bool _has_anchor_ratio;
    int _anchor_ratio;
    bool _has_anchor_length;
    int _anchor_length;
    bool _has_tag_density;
    float _tag_density;
    bool _has_punct_count;
    int _punct_count;
    bool _has_punct_ratio;
    int _punct_ratio;
    bool _has_space_ratio;
    int _space_ratio;

    bool _has_contain_outside_link;
    bool _contain_outside_link;
    bool _has_contain_advertise_link;
    bool _contain_advertise_link;
    bool _prev_is_noise;
    bool _next_is_noise;
    bool _is_good;
    bool _is_bad;
    bool _is_content;
    std::string text;
    Node *_node;
};

} //namespace vdom
} //namespace content

#endif
