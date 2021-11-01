#-------------------------------------------------
#
# Project created by QtCreator 2017-12-04T14:22:35
#
#-------------------------------------------------

QT += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

contains(CONFIG, static):DEFINES += QT_NODLL


DEFINES += GUIBUILD

windows{
    QT += axserver
    DEFINES += WINBUILD
    message("Windows version of DNASig")
}

linux-g++ {
    message(Linux)
}


CONFIG += warn_off
#CONFIG += qaxserver_no_postlink
#CONFIG += console

RC_FILE  = dnasig.rc

TARGET = dnasig


## OUTPROCESS EXE SERVER
#
TEMPLATE = app


## INPROCESS DLL SERVER
#
#TEMPLATE = lib
#CONFIG  += dll
#DEF_FILE = dnasig.def


windows{

message($${QMAKE_TARGET.arch})

ISX64=$$find(QMAKE_TARGET.arch, "x86_64")

equals(ISX64, ""):{
    ISX86=$$find(QMAKE_TARGET.arch, "x64_86")

    equals(ISX86, ""):{
        ISX64=$$find(QMAKE_TARGET.arch, "x64")
    }
}

#message(isx64 $${ISX64})

!equals(ISX64, ""):{

INCLUDEPATH += ..\xml64bit\build\build_x64\include
INCLUDEPATH += ..\xml64bit\build\build_x64\include\libiconv

win32:LIBS  += ..\xml64bit\build\build_x64\lib\libxslt.lib
win32:LIBS  += ..\xml64bit\build\build_x64\lib\libxml2.lib
win32:LIBS  += ..\xml64bit\build\build_x64\lib\libxmlsec.lib
win32:LIBS  += ..\xml64bit\build\build_x64\lib\libcrypto.lib
win32:LIBS  += ..\xml64bit\build\build_x64\lib\iconv.lib

}else{

INCLUDEPATH += ..\xml\include

win32:LIBS  += ..\xml\lib\libxslt.lib
win32:LIBS  += ..\xml\lib\libxml2.lib
win32:LIBS  += ..\xml\lib\libxmlsec.lib
win32:LIBS  += ..\xml\lib\libcrypto.lib
}

DEFINES += XMLSEC_NO_SIZE_T
DEFINES += XMLSEC_CRYPTO_OPENSSL
DEFINES += XMLSEC_CRYPTO_DYNAMIC_LOADING
}


linux-g++ {
INCLUDEPATH += /usr/include/
INCLUDEPATH += /usr/include/libxml2
INCLUDEPATH += /usr/include/libxslt
INCLUDEPATH += /usr/include/xmlsec1

XS_LIBS    = $$system(xmlsec1-config --libs)
XS_DEFINES = $$system(xmlsec1-config --cflags)

LIBS            += $$XS_LIBS
QMAKE_CXXFLAGS  += $$XS_DEFINES

}



SOURCES += main.cpp\
        dnasig.cpp \
    testverifier.cpp

HEADERS  += dnasig.h \
    testverifier.h


TRANSLATIONS += dnasig_ru.ts

OTHER_FILES += \
    dnasig.ico \
    dnasig.rc \
    dnasig.def

#RESOURCES += \
#    dnasig.qrc

RESOURCES += \
    dnasig.qrc

