#-------------------------------------------------
#
# Project created by QtCreator 2013-04-16T14:39:23
#
#-------------------------------------------------

QT       -= core gui

unix:TARGET = $$PWD/../bin/NVSSDKEngine
win32:TARGET = NVSSDKEngine

TEMPLATE = lib

DEFINES += NVSSDKENGINE_LIBRARY

DEFINES -= UNICODE

SOURCES += \
    stdafx.cpp \
    NVSSDKFunctions.cpp \
    NVSSDKEngine.cpp \
    NVSHandlerMgr.cpp \
    writelog.cpp

HEADERS += \
    stdafx.h \
    NVSSDKFunctions.h \
    NVSSDKDef.h \
    NVSHandlerMgr.h \
    IDVRHandler.h \
    NVSTypeDefine.h \
    WriteLog.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE5334B09
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = NVSSDKEngine.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}


