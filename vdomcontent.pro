TEMPLATE = lib
SOURCES += vdom_content_extractor.cpp vdom_content_util.cpp
HEADERS += vdom_content_block.h  vdom_content_extractor.h  vdom_content_result.h vdom_content_util.h

INCLUDEPATH  = /opt/qcrawler-thirdparty/include
LIBS = -L /opt/qcrawler-thirdparty/lib -lprotobuf -lvdom -lpcre

QT -= gui

