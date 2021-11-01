#-------------------------------------------------
#
# Project created by QtCreator 2017-12-04T14:22:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

contains(CONFIG, static):DEFINES += QT_NODLL

windows{
QT += axserver
message("Windows version of DNASig")
}


QT += xml

CONFIG += warn_off
#CONFIG += qaxserver_no_postlink

RC_FILE  = dnasig.rc



## OUTPROCESS EXE SERVER
#
TEMPLATE = app


## INPROCESS DLL SERVER
#
#TEMPLATE = lib
#CONFIG  += dll
#DEF_FILE = dnasig.def


#INCLUDEPATH += D:\agusev\gusev\Devel\xml\include

#win32:LIBS += D:\agusev\gusev\Devel\xml\lib\libxslt.lib
#win32:LIBS += D:\agusev\gusev\Devel\xml\lib\libxml2.lib
#win32:LIBS += D:\agusev\gusev\Devel\xml\lib\libxmlsec.lib
#win32:LIBS += D:\agusev\gusev\Devel\xml\lib\libcrypto.lib


#INCLUDEPATH += ..\xml\include

#win32:LIBS  += ..\xml\lib\libxslt.lib
#win32:LIBS  += ..\xml\lib\libxml2.lib
#win32:LIBS  += ..\xml\lib\libxmlsec.lib
#win32:LIBS  += ..\xml\lib\libcrypto.lib

#out
TARGET = dnasig
target.files = TARGET
target.path = /home/root/
INSTALLS += target

LIBS += -lxslt
LIBS += -lxml2
LIBS += -lcrypto
LIBS += -lxmlsec1

DEFINES += XMLSEC_CRYPTO_OPENSSL
DEFINES += XMLSEC_CRYPTO_DYNAMIC_LOADING
DEFINES += XMLSEC_NO_SIZE_T

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

