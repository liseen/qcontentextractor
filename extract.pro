TEMPLATE = app
SOURCES += test.cpp  vdom_content_extractor.cpp vdom_content_util.cpp
HEADERS += vdom_content_block.h  vdom_content_extractor.h  vdom_content_result.h

LIBS += -lvdom -lprotobuf -lpcre

QT -= gui

