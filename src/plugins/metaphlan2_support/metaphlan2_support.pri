# include (metaphlan2_support.pri)
include (../../ugene_version.pri)

PLUGIN_ID=metaphlan2_support
PLUGIN_NAME=MetaPhLan2 external tool support
PLUGIN_VENDOR=Unipro
PLUGIN_DEPENDS=ngs_reads_classification:$${UGENE_VERSION};external_tool_support:$${UGENE_VERSION}

LIBS += -L../../_release/plugins -lngs_reads_classification

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        PLUGIN_DEPENDS -= ngs_reads_classification:$${UGENE_VERSION};external_tool_support:$${UGENE_VERSION}
        PLUGIN_DEPENDS += ngs_reads_classificationd:$${UGENE_VERSION};external_tool_supportd:$${UGENE_VERSION}

        LIBS -= -L../../_release/plugins -lngs_reads_classification
        LIBS += -L../../_debug/plugins -lngs_reads_classificationd
    }
}

include( ../../ugene_plugin_common.pri )