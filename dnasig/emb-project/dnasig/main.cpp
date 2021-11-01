#define QAX_DUMPCPP_DNASIG_NOINLINES

#include "dnasig.h"
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QFile>

//#include <openssl/applink.c>

#ifdef WIN32
#define winBuild
#endif

#ifdef winBuild
#include <QAxFactory>
#endif



QT_USE_NAMESPACE

#ifdef winBuild
QAXFACTORY_BEGIN("{2A593805-A956-4a3e-A593-CB81FEB5F16B}", "{434A4F15-72DC-45ad-A4CF-C011B56F5091}")
    QAXCLASS(DnaSig)
QAXFACTORY_END()
#endif

int main(int argc, char *argv[]){

    QApplication::setOrganizationName   ("DNA-Technology");
    QApplication::setOrganizationDomain ("dna-technology.ru");
    QApplication::setApplicationName    ("DNASIG");

#ifdef winBuild
    QApplication
            app(argc, argv);
            app.setQuitOnLastWindowClosed(false);

    QDir::setCurrent( app.applicationDirPath() );

    // started by COM - don't do anything
    if (QAxFactory::isServer())
        return app.exec();

    // started by user
    DnaSig  appobject(0);
            appobject.setObjectName("DNASIG_From_Application");


    QAxFactory::startServer();
    QAxFactory::registerActiveObject(&appobject);

    QObject::connect( &app, SIGNAL(lastWindowClosed()),
                &appobject, SLOT(quit()));

    return appobject.execute(false);
#else

    QApplication app(argc, argv);

    QDir::setCurrent( app.applicationDirPath() );

    DnaSig  appobject(0);
            appobject.setObjectName("DNASIG_From_Application");

    QString line;
    QString xmlIn= "";
    int     rc   = -1;

    if ( argc>1 ){

        if ( QString(argv[1]).endsWith("help") ){
            qDebug()<< QString("Usage: %s [<xml-file> [-s]]").arg( argv[0] );
            qDebug()<< "\tVerify xml-file or sign if -s key exists.";
//            qDebug()<< "\tIf no xml-file parameter specified then reads xml from stdin and verify.";
            qDebug()<< "\tReturns code 0 if successed.";
            return  0;
        }

        QString inFile = argv[1];

        rc = false == appobject.appendFile( inFile, (argc>2) );

    }else{

        qDebug()<< "Reading STDIN...";

        QTextStream stream( stdin );

        while ( stream.readLineInto(&line) )
            xmlIn += line;
    }

    return rc;

#endif

}
