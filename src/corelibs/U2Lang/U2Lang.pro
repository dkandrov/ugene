include (U2Lang.pri)

# Input
HEADERS += src/library/BaseActorCategories.h \
           src/library/BaseAttributes.h \
           src/library/BasePorts.h \
           src/library/BaseSlots.h \
           src/library/BaseTypes.h \
           src/library/LocalDomain.h \
           src/library/ScriptLibrary.h \
           src/model/ActorModel.h \
           src/model/ActorPrototypeRegistry.h \
           src/model/Attribute.h \
           src/model/Configuration.h \
           src/model/ConfigurationEditor.h \
           src/model/ConfigurationValidator.h \
           src/model/Datatype.h \
           src/model/DatatypeConversion.h \
           src/model/Descriptor.h \
           src/model/IntegralBus.h \
           src/model/IntegralBusModel.h \
           src/model/IntegralBusType.h \
           src/model/Peer.h \
           src/model/Port.h \
           src/model/QDConstraint.h \
           src/model/QDScheme.h \
           src/model/QueryDesignerRegistry.h \
           src/model/Schema.h \
           src/model/WorkflowEnv.h \
           src/model/WorkflowManager.h \
           src/model/WorkflowTransport.h \
           src/support/CoreLibConstants.h \
           src/support/HRSchemaSerializer.h \
           src/support/MapDatatypeEditor.h \
           src/support/RunSchemaForTask.h \
           src/support/SchemaSerializer.h \
           src/support/WorkflowEnvImpl.h \
           src/support/WorkflowIOTasks.h \
           src/support/WorkflowRunTask.h \
           src/support/WorkflowSettings.h \
           src/support/WorkflowUtils.h
SOURCES += src/library/BaseActorCategories.cpp \
           src/library/BaseAttributes.cpp \
           src/library/BasePorts.cpp \
           src/library/BaseSlots.cpp \
           src/library/BaseTypes.cpp \
           src/library/LocalDomain.cpp \
           src/library/ScriptLibrary.cpp \
           src/model/ActorModel.cpp \
           src/model/ActorPrototypeRegistry.cpp \
           src/model/Attribute.cpp \
           src/model/Configuration.cpp \
           src/model/Datatype.cpp \
           src/model/Descriptor.cpp \
           src/model/IntegralBus.cpp \
           src/model/IntegralBusModel.cpp \
           src/model/IntegralBusType.cpp \
           src/model/Port.cpp \
           src/model/QDConstraint.cpp \
           src/model/QDScheme.cpp \
           src/model/QueryDesignerRegistry.cpp \
           src/model/Schema.cpp \
           src/model/WorkflowEnv.cpp \
           src/model/WorkflowTransport.cpp \
           src/support/CoreLibConstants.cpp \
           src/support/HRSchemaSerializer.cpp \
           src/support/MapDatatypeEditor.cpp \
           src/support/RunSchemaForTask.cpp \
           src/support/SchemaSerializer.cpp \
           src/support/WorkflowEnvImpl.cpp \
           src/support/WorkflowIOTasks.cpp \
           src/support/WorkflowRunTask.cpp \
           src/support/WorkflowSettings.cpp \
           src/support/WorkflowUtils.cpp

TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
