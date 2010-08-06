#include <fstream>
#include <iostream>
#include <vdom.h>
#include <tcrdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <QTime>

#include <google/protobuf/text_format.h>

#include "vdom_content_extractor.h"

using namespace std;

int main()
{
    TCRDB *rdb;
    int ecode;
    char *value;

    /* create the object */
    rdb = tcrdbnew();

    /* connect to the server */
    if(!tcrdbopen(rdb, "l-crwl2", 9860)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "open error: %s\n", tcrdberrmsg(ecode));
    }

    vdom::content::Extractor extractor;

    std::string vdom_key;
    std::string vdom_content;
    std::string line;
    while (getline(cin, line)) {
        if (line.size() < 10) {
            continue;
        }
        vdom_key.clear();
        vdom_content.clear();
        vdom_key = line;
        vdom_key.append(".v");
        int vdom_content_size;
        value = (char*)tcrdbget(rdb, vdom_key.c_str(), vdom_key.size(), &vdom_content_size);
        if(value){
            vdom_content.append(value, vdom_content_size);
            free(value);
        } else {
            ecode = tcrdbecode(rdb);
            fprintf(stderr, "get error: %s\n", tcrdberrmsg(ecode));
        }

        vdom::Window win;
        vdom::content::Result ret;
        win.ParseFromString(vdom_content);
        extractor.extract(&win, ret, false);
        int list_confidence = ret.list_confidence;
        if (list_confidence > 100) {
            list_confidence = 100;
        }

        std::cout << line << "\t" <<  list_confidence << "\t" <<  win.location() << "\n";
    }

    /* close the connection */
    if(!tcrdbclose(rdb)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "close error: %s\n", tcrdberrmsg(ecode));
    }

    return 0;
}
