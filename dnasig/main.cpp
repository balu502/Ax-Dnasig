#define QAX_DUMPCPP_DNASIG_NOINLINES

#include "dnasig.h"
#include <QApplication>
#include <QDir>



#ifdef WINBUILD
#include <openssl/applink.c>
#include <QAxFactory>
#endif

#ifndef GUIBUILD
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <iostream>
#endif


QT_USE_NAMESPACE

#ifdef WINBUILD
QAXFACTORY_BEGIN("{2A593805-A956-4a3e-A593-CB81FEB5F16B}", "{434A4F15-72DC-45ad-A4CF-C011B56F5091}")
    QAXCLASS(DnaSig)
QAXFACTORY_END()
#endif

int main(int argc, char *argv[]){

    QApplication::setOrganizationName   ("DNA-Technology");
    QApplication::setOrganizationDomain ("dna-technology.ru");
    QApplication::setApplicationName    ("DNASIG");

    QApplication app( argc, argv );

    if(!app.libraryPaths().contains(
                            QDir::currentPath() ) ){
        app.addLibraryPath( QDir::currentPath() );
    }

    QDir::setCurrent( app.applicationDirPath() );

#ifdef GUIBUILD

    app.setQuitOnLastWindowClosed(false);

#ifdef WINBUILD
    // started by COM - don't do anything
    if (QAxFactory::isServer())
        return app.exec();
#endif

    // started by user
    DnaSig  appobject(0);
            appobject.setObjectName("DNASIG_From_Application");

#ifdef WINBUILD
    QAxFactory::startServer();
    QAxFactory::registerActiveObject(&appobject);
#endif

    QObject::connect( &app, SIGNAL(lastWindowClosed()),
                &appobject, SLOT(quit()));

    return appobject.execute(false);
#else

//    QApplication app(argc, argv);
//    QDir::setCurrent( app.applicationDirPath() );

    DnaSig  appobject(0);
            appobject.setObjectName("DNASIG_From_Application");

    int rc = -1;

    if ( argc>1 ){

        if ( QString(argv[1]).endsWith("help") ){
            qDebug()<< QString("Usage: %s [<xml-file> [-s]]").arg( argv[0] );
            qDebug()<< "\tVerify xml-file or sign if -s key exists, output signed into exportTests.xml.";
            qDebug()<< "\tIf no xml-file parameter specified then reads xml from stdin and verify.";
            qDebug()<< "\tReturns code 0 if successed.";
            return  0;
        }

        QString inFile = argv[1];

        rc = false == appobject.appendFile( inFile, (argc>2) );

    }else{

        qDebug()<< "Reading STDIN...";

        QTextStream stream( stdin );

        rc = false == appobject.appendStream( &stream );
    }

    return rc;

#endif

}
