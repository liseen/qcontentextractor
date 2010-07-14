#include "vdom_content_extractor.h"
#include "vdom_content_util.h"

#include <map>
#include <set>

#include <google/protobuf/text_format.h>

#define VD_MIN(a,b) ((a)>(b)?(b):(a))
#define VD_MAX(a,b) ((a)>(b)?(a):(b))

namespace vdom {
namespace content {

static void debug_print_block_list(TextBlockList &list)
{
    int count = 0;
    for (TextBlockIter it = list.begin(); it != list.end(); it++) {
        std::cout << count++ <<  ": -------------------" << std::endl;
        it->debug_print();
    }
}

Extractor::Extractor()
{
}

Extractor::~Extractor()
{
}


bool Extractor::extract(vdom::Window *window, Result &result, bool debug)
{
    // init
    vdom::Document *doc = window->mutable_doc();

    result.raw_title = doc->title();
    result.title = doc->title();
    if (! doc->has_body()) {
        result.extracted_okay = false;
        return false;
    }

    doc->mutable_body()->build_vdom_tree(window, doc, NULL, 0);

    result.keywords = doc->keywords();
    result.description = doc->description();

    vdom::Node* body = doc->mutable_body();
    std::list<TextBlock> block_list;
    extract_block_list(body, block_list);

    select_good_block(block_list);
    expand_good_block(block_list);

    if (debug) {
        debug_print_block_list(block_list);
    }

    merge_content_block(block_list, result);
    //merge_content(block_list);


    // list page confidence
    body->build_repeat_sig();
    RepeatGroupList group_list;
    extract_repeat_groups(group_list, body);

    result.list_confidence = compute_list_confidence(doc, group_list);

    if (debug) {
        std::cout << "body repeat sig: " << body->repeat_sig() << std::endl;
        std::cout << "extract_repats_size " << group_list.size() << std::endl;
        for (RepeatGroupListIter lit = group_list.begin(); lit != group_list.end(); lit++) {
            if (is_link_group(*lit)) {
                std::cout << "group: =========================" << std::endl;
                for (RepeatGroupIter it = lit->begin(); it != lit->end(); it++) {
                    vdom::Node *node = *it;
                    std::cout << "node: =========================" << std::endl;
                    //std::cout << (*it)->content();
                    if (node->repeat_sig().find("#A") != std::string::npos) {
                        std::list<vdom::Node*> links;
                        node->get_elements_by_tag_name("A", links);
                        std::cout << "x: " << (*it)->x() << std::endl;
                        std::cout << "y: " << (*it)->y() << std::endl;
                        std::cout << "w: " << (*it)->w() << std::endl;
                        std::cout << "h: " << (*it)->h() << std::endl;
                        std::string normal;
                        Util::normalize_content((*it)->content(), normal);
                        std::cout << "content: " << normal << std::endl;
                    }
                }
            }
        }
    }

    // extract images
    if (debug) {
        std::list<vdom::Node*> imgs;
        body->get_elements_by_tag_name("IMG", imgs);
        std::cout << "images: " << std::endl;
        for (std::list<vdom::Node*>::iterator it = imgs.begin(); it != imgs.end(); it++) {
            if ((*it)->w() > 200) {
                std::cout << (*it)->Utf8DebugString() << std::endl;
                std::cout << "area: " << (*it)->w() * (*it)->h()<< std::endl;
            }
        }
    }
    result.extracted_okay = true;

    return true;
}

/* top down first, get all text block */
bool Extractor::extract_block_list(Node* node, std::list<TextBlock> &block_list) {
    if (node->type() == Node::ELEMENT) {
        /* filters */
        if (check_is_noise(node)) {
            prev_is_noise = true;
            if (!block_list.empty()) {
                block_list.back().set_next_is_noise(true);
            }
        } else {
            if (node->all_children_inline()) {
                TextBlock block;
                block.set_node(node);
                block.set_prev_is_noise(prev_is_noise);
                tag_block(block);
                block_list.push_back(block);
                prev_is_noise = false;
            } else {
                int child_size = node->child_nodes_size();
                for (int i = 0; i < child_size; i++) {
                    extract_block_list(node->mutable_child_nodes(i), block_list);
                }
            }
        }
    } else {
        TextBlock block;
        block.set_node(node);
        block.set_prev_is_noise(prev_is_noise);
        tag_block(block);
        block_list.push_back(block);
        prev_is_noise = false;
    }

    return true;
}

bool Extractor::check_is_noise(Node *node) {
    int doc_width = node->owner_document()->width();

    int x = node->x();
    int y = node->y();
    int w = node->w();
    int h = node->h();

    /* navigator or menu */
    if (x < 0.3 * doc_width && y < 300 &&  w > 0.9 * doc_width && h < 100 &&  (float)h/w < 0.1 && node->content().size() < 250) {
        //DD("nav or menu: " << node->content());
        //DD("nav or menu tag_name: " << node->tag_name());
        return true;
    }

    /* left and right advertise banner */
    if (w > 0  && h > 200 && w < 350 &&  (float)h/w > 0.8 && (x > 0.6 * doc_width || x + w  < 0.4 * doc_width) ) {
        //DD("left or right adver or menu: " << node->content());
        return true;
    }

    return false;
}


void Extractor::tag_block(TextBlock &block) {
    assert(block.node() != NULL);
    Node *node = block.node();
    int doc_width = node->owner_document()->width();
    int doc_height = node->owner_document()->height();

    int x = node->x();
    int y = node->y();
    int w = node->w();
    int h = node->h();

    //if (h < 400 && (y < 0.3 * doc_height || y > 0.5 * doc_height) && Util::contain_copyright_text(node->content())) { /* bottom copyright .. */
    if (h < 200 && Util::contain_copyright_text(node->content())) { /* bottom copyright .. */
        block.set_is_bad(true);
        return;
    }

    int good_block_max_height = (int)(0.84 * doc_height);
    if (good_block_max_height < doc_height - 200) {
        good_block_max_height = doc_height - 200;
    }

    if (x + w > 0.3 * doc_width && \
        x < 0.6 * doc_width && \
        y > 100 && \
        y < good_block_max_height && \
        block.content_size() > 35 && \
        block.anchor_ratio() < 20 && block.tag_density() < 0.1 && block.space_ratio() < 50 ) {

        block.set_is_good(true);

        return;
    }
}

bool Extractor::select_good_block(std::list<TextBlock> &list) {
    for (TextBlockIter it = list.begin(); it != list.end(); it++) {
        tag_block(*it);
    }

    return true;
}

/* Bottom-up merge expand good block_list*/
bool Extractor::expand_good_block(std::list<TextBlock> &block_list) {
    if (block_list.size() < 1) {
        return true;
    }

    TextBlockIter begin_it = block_list.begin();
    TextBlockIter end_it = block_list.end();
    int doc_width = begin_it->node()->owner_document()->width();
    for (TextBlockIter it = begin_it; it != end_it; it++) {
        if (it->is_good()) {
            it->set_is_content(true);
            TextBlockIter ei = it;
            ++ei;
            TextBlockIter last_content = ei;
            int space_blocks = 0;
            // forward expand
            for (; ei != end_it; ++ei) {
                Node* node = ei->node();
                if ((ei->space_ratio() > 90 || ei->content_size() == 0) && node->w() == 0) {
                    ++space_blocks;
                } else if ((abs(node->x() - it->node()->x()) > 100)) {
                    ++space_blocks;
                }

                if (space_blocks > 5) {
                    break;
                } else if (ei->is_bad()) {
                    break;
                } else if ((ei->is_good() || ei->is_content()) && abs(node->y() - it->node()->y()) < 400 ) {
                    for (; last_content != ei; ++last_content) {
                        last_content->set_is_content(true);
                    }
                    break;
                } else if (space_blocks <= 5 && \
                        abs(node->y() - it->node()->y() - it->node()->h()) < 200 &&\
                        node->x() + node->w() > 0.11 * doc_width && \
                        ei->content_size() > 20 && \
                        ei->anchor_ratio() < 50 && ei->tag_density() < 0.2 && ei->space_ratio() < 50 ) {
                    space_blocks  = 0;
                    ei->set_is_content(true);
                    for (; last_content != ei; ++last_content) {
                        last_content->set_is_content(true);
                    }
                }

                if (ei->next_is_noise()) {
                    break;
                }
            }

            space_blocks = 0;
            ei = it;
            --ei;
            last_content = ei;
            // backward
            for (; ei != end_it; --ei) {
                //std::cout << "ei: " <<  space_blocks << " " << ei->node()->content() << std::endl;
                Node* node = ei->node();
                if ((ei->space_ratio() > 90 || ei->content_size() == 0) && node->w() == 0) {
                    ++space_blocks;
                    continue;
                } else if ((abs(node->x() - it->node()->x()) > 100)) {
                    ++space_blocks;
                }

                if (space_blocks > 5) {
                    break;
                } else if (ei->is_bad()) {
                    break;
                } else if ((ei->is_good() || ei->is_content()) && abs(node->y() - it->node()->y()) < 400 ) {
                    for (; last_content != ei; --last_content) {
                        last_content->set_is_content(true);
                    }
                    break;
                } else if (space_blocks <= 5 && \
                        abs(node->y() - it->node()->y() - node->h()) < 200 &&\
                        node->x() + node->w() > 0.11 * doc_width && \
                        ei->content_size() > 20 && \
                        ei->anchor_ratio() < 50 && ei->tag_density() < 0.2 && ei->space_ratio() < 50 ) {
                    space_blocks  = 0;
                    ei->set_is_content(true);
                    for (; last_content != ei; --last_content) {
                        last_content->set_is_content(true);
                    }
                }

                if (ei->prev_is_noise()) {
                    break;
                }
            }
        }
    }

    return true;
}

bool Extractor::merge_content_block(TextBlockList &block_list, Result &result) {
    TextBlockIter end_it = block_list.end();
    bool prev_is_content = true;
    bool prev_is_space = true;
    bool has_content = false;
    result.content.reserve(50 * 1024);
    for (TextBlockIter it = block_list.begin(); it != end_it; ++it) {
        if (it->is_content()) {
            has_content = true;
            if (!prev_is_content) {
                result.content.append("\n");
            }
            prev_is_content = true;

            if (!prev_is_space) {
                result.content.append(" ");
            }
            if (it->space_ratio() == 100) {
                prev_is_space = true;
            } else {
                prev_is_space = false;
            }

            result.content.append(it->node()->content());
        } else {
            prev_is_content = false;
        }
    }

    if (has_content) {
        result.content_confidence = 100;
    } else {
        result.content_confidence = 50;
        for (TextBlockIter it = block_list.begin(); it != end_it; ++it) {
            if (!it->is_bad()) {
                result.content.append(it->node()->content());
                result.content.append(" ");
            }
        }
    }

    return true;
}

bool Extractor::extract_repeat_groups(RepeatGroupList &groups, Node* node)
{
    std::map<std::string, RepeatGroup> group_map;
    std::map<std::string, RepeatGroup>::iterator end_it = group_map.end();
    std::map<std::string, RepeatGroup>::iterator it;

    int child_size = node->child_nodes_size();
    for (int i = 0; i < child_size; i++) {
        Node* child = node->mutable_child_nodes(i);
        if (child->type() == Node::ELEMENT && child->w() > 200 && child->repeat_sig().size() > 0 && child->repeat_sig().find("#A") != std::string::npos) {
            it = group_map.find(child->repeat_sig());
            if (it == end_it) {
                RepeatGroup group;
                group.push_back(child);
                group_map.insert(std::pair<std::string, RepeatGroup>(child->repeat_sig(), group));
            } else {
                it->second.push_back(child);
            }
        }
    }

    for (it = group_map.begin() ; it != end_it; it++ ) {
        if (it->second.size() >= 3) {
            groups.push_back(it->second);
        } else {
            if (!check_is_noise(it->second.front())) {
                extract_repeat_groups(groups, it->second.front());
            }
        }
    }

    return true;
}

bool Extractor::is_link_group(RepeatGroup &group)
{
    if (group.size() <= 0) {
        return false;
    }

    int times = 0;
    int avr_w =  -1;
    int avr_h =  -1;
    std::set<std::string> url_set;
    for (RepeatGroupIter it = group.begin(); it != group.end(); it++) {
        times++;

        vdom::Node *node = *it;
        if (times == 1) {
            avr_w = node->w();
        } else if (node->w() > 1.1 * avr_w || node->w() < 0.9 * avr_w ) {
            return false;
        }

        avr_h += node->h();

        std::list<vdom::Node*> links;
        node->get_elements_by_tag_name("A", links);
        bool contain_good_link = false;
        bool all_same_links = false;

        for (std::list<vdom::Node*>::iterator link_it = links.begin(); link_it != links.end(); link_it++) {
            vdom::Node *link_node = *link_it;
            if (link_node->w() > 100 && link_node->normalized_content().size() > 10) {
                contain_good_link = true;
                if (times == 1) {
                    url_set.insert(link_node->normalized_content());
                } else {
                    if (url_set.find(link_node->normalized_content()) == url_set.end()) {
                        all_same_links = false;
                    }
                }
            }
        }

        if (!contain_good_link) {
            return false;
        }

        if (all_same_links) {
            return false;
        }
    }

    if (avr_h > 0 && avr_h / group.size()  > 250) {
        return false;
    }

    return true;
}

int Extractor::compute_list_confidence(vdom::Document *doc, RepeatGroupList &group_list)
{
    int doc_width = doc->width();
    int doc_height = doc->height();
    if (doc_height > 1.5 * 847) {
        doc_height = 1.5 * 847;
    }

    int central_w = 0.618 * doc_width;
    int central_h = 0.618 * doc_height;
    int central_x = 0.5 * (1 - 0.618) * doc_width;
    int central_y = 0.5 * (1 - 0.618) * doc_height;

    int  confidence = 0;

    for (RepeatGroupListIter lit = group_list.begin(); lit != group_list.end(); lit++) {
        if (is_link_group(*lit)) {
            for (RepeatGroupIter it = lit->begin(); it != lit->end(); it++) {
                vdom::Node *node = *it;
                int x = node->x();
                int y = node->y();
                int w = node->w();
                int h = node->h();
                int x1 = VD_MAX(x, central_x);
                int x2 = VD_MIN(x + w, central_x + central_w);
                int y1 = VD_MAX(y, central_y);
                int y2 = VD_MIN(y + h, central_y + central_h);
                if (x2 > x1 && y2 > y1) {
                    int area = (x2 - x1) * (y2 - y1);
                    if (area * 2 > w * h) {
                        confidence += area;
                    }
                }
            }
        }
    }

    return confidence * 100 / (central_w * central_h + 1);

}

} //namespace vdom
} //namespace content


