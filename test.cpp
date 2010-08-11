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

int main(int argc, char* argv[])
{
    std::string vdom_key;
    std::string vdom_content;
    if (argc < 2) {
        fprintf(stderr, "not vdom key gived\n");
        exit(1);
    }

    vdom_key.append(argv[1]);
    vdom_key.append(".v");

    TCRDB *rdb;
    int ecode;
    char *value;

    /* create the object */
    rdb = tcrdbnew();

    /* connect to the server */
    if(!tcrdbopen(rdb, "crwl2", 9860)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "open error: %s\n", tcrdberrmsg(ecode));
    }

    int vdom_content_size;
    value = (char*)tcrdbget(rdb, vdom_key.c_str(), vdom_key.size(), &vdom_content_size);
    if(value){
        vdom_content.append(value, vdom_content_size);
        free(value);
    } else {
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "get error: %s\n", tcrdberrmsg(ecode));
    }

    /* close the connection */
    if(!tcrdbclose(rdb)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "close error: %s\n", tcrdberrmsg(ecode));
    }

    /* delete the object */
    tcrdbdel(rdb);

    vdom::Window win;
    win.ParseFromString(vdom_content);

    std::cout << "vdom: " << std::endl;
    std::cout << win.Utf8DebugString() << std::endl;

    std::cout << "extractor:" << std::endl;
    vdom::content::Extractor extractor(false, true);
    vdom::content::Result ret;
    QTime begin_time;
    begin_time.start();
    extractor.extract(&win, ret);
    std::cout<<"elapsed: "<< begin_time.elapsed() << endl;
    ret.debug_print();
    return 0;
}
