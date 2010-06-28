#include "vdom_content_extractor.h"
#include "vdom_content_util.h"

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

    result.extracted_okay = true;
    return true;
}

/* top down first, get all text block */
bool Extractor::extract_block_list(Node* node, std::list<TextBlock> &block_list) {
    if (node->tag_name() == "FORM" ||
            (node->type() == Node::ELEMENT && node->render_type() != Node::INLINE) ) {
        /* filters */
        if (check_is_noise(node)) {
            prev_is_noise = true;
            if (!block_list.empty()) {
                block_list.back().set_next_is_noise(true);
            }
        }

        if (node->render_type() == Node::BLOCK && node->all_children_inline()) {
            TextBlock block;
            block.set_node(node);
            block.set_prev_is_noise(prev_is_noise);
            tag_block(block);
            block_list.push_back(block);
        } else {
            int child_size = node->child_nodes_size();
            for (int i = 0; i < child_size; i++) {
                extract_block_list(node->mutable_child_nodes(i), block_list);
            }
        }
    } else {
        TextBlock block;
        block.set_node(node);
        block.set_prev_is_noise(prev_is_noise);
        tag_block(block);
        block_list.push_back(block);
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
    if (x < 0.3 * doc_width && y < 300 &&  w > 0.9 * doc_width && h < 100 &&  (float)h/w < 0.1) {
        //DD("nav or menu: " << node->content());
        return true;
    }

    /* left and right advertise banner */
    if (w > 0  && h > 200 && w < 400 &&  (float)h/w > 0.8 && (x > 0.6 * doc_width || x < 0.3 * doc_width) ) {
        //DD("left or right adver or menu: " << node->content);
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

    if (h < 400 && y > 0.5 * doc_height && Util::contain_copyright_text(node->content())) { /* bottom copyright .. */
        block.set_is_bad(true);
        return;
    }

    int good_block_max_height = (int)(0.7 * doc_height);
    if (good_block_max_height < doc_height - 500) {
        good_block_max_height = doc_height - 500;
    }

    if (x + w > 0.3 * doc_width && \
        y > 50 && \
        y < good_block_max_height && \
        block.content_size() > 50 && \
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
            TextBlockIter ei = ++it; --it;
            TextBlockIter last_content = ei;
            int space_blocks = 0;
            // forward expand
            for (; ei != end_it; ++ei) {
                Node* node = ei->node();
                if (ei->space_ratio() > 90 && node->w() == 0) {
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
                        abs(node->y() - it->node()->y()) < 200 &&\
                        node->x() + node->w() > 0.5 * doc_width && node->w() > 0.3 * doc_width && \
                        ei->content_size() > 20 && \
                        ei->anchor_ratio() < 40 && ei->tag_density() < 0.2 && ei->space_ratio() < 50 ) {
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
            ei = --it; ++it;
            last_content = ei;
            for (; ei != end_it; --ei) {
                Node* node = ei->node();
                if (ei->space_ratio() > 90 && node->w() == 0) {
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
                        abs(node->y() - it->node()->y()) < 200 &&\
                        node->x() + node->w() > 0.5 * doc_width && node->w() > 0.3 * doc_width && \
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
            }
        }
    }

    return true;
}
} //namespace vdom
} //namespace content


