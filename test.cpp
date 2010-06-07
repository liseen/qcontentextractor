#include <fstream>
#include <iostream>
#include <vdom.h>

#include <google/protobuf/text_format.h>

#include "vdom_content_extractor.h"

using namespace std;

int main(int argc, char* argv[])
{
    std::string vdom_content;
    if (argc < 2) {
        std::string line;
        while (getline(cin, line)) {
            vdom_content.append(line);
            vdom_content.append("\n");
        }
    } else {
        ifstream in_file;
        in_file.open(argv[1]);
        if (!in_file) {
            cout << "Unable to open file";
            exit(1); // terminate with error
        }
        std::string line;
        while (getline(in_file, line)) {
            vdom_content.append(line);
            vdom_content.append("\n");
        }

        if (vdom_content.size() > 1) {
            vdom_content = vdom_content.substr(0, vdom_content.size() - 2);
        }

        in_file.close();
    }

    vdom::Window win;
    ::google::protobuf::TextFormat::ParseFromString(vdom_content, &win);
    vdom::Document *doc = win.mutable_doc();

    doc->mutable_body()->build_vdom_tree(&win, doc, NULL, 0);

    vdom::content::Extractor extractor;
    vdom::content::Result ret;
    extractor.extract(win, ret, true);
    ret.debug_print();
    return 0;
}
