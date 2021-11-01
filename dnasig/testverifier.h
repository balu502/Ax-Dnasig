#ifndef TESTVERIFIER_H
#define TESTVERIFIER_H

#include <cstdio>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#ifndef XMLSEC_NO_XSLT
#include <libxslt/xslt.h>
#include <libxslt/security.h>
#endif /* XMLSEC_NO_XSLT */

#include <openssl/pem.h>
#include <openssl/x509.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/templates.h>
#include <xmlsec/crypto.h>

#include <QHash>
#include <QString>
#include <QStringList>



class TestVerifier{

    QHash< QString, xmlSecKeyPtr >  pubKeys;
    QHash< QString, xmlSecKeyPtr > privKeys;

    int _error;

    EVP_PKEY *generateKey();
    X509     *generateX509(EVP_PKEY *pkey);

    bool    writeKeys(EVP_PKEY *pkey, QString k, QString p, X509 *x509, QString c);

public:
    TestVerifier();
    ~TestVerifier();

    QString CountryCode;
    QString Org;
    QString CommonName;
    long    daysLimit;

    int isInvalid(){ return _error; }

    bool generateKeys(QString keyN="key.pem", QString pubN="pub.pem", QString crtN="cert.cer" );

    bool insertPublic( QString pathP );
    bool insertPrivate(QString pathP );


    bool removePublic( QString pathP );

    bool validateNodeByKey(xmlNodePtr, xmlSecKeyPtr);

    bool isValidated(QString s, QString pubK = QString() );
    QString sign(    QString s, QString privK= QString() );

    QStringList pubKeyList(){ return QStringList().fromStdList(  pubKeys.keys().toStdList() ); }
    QStringList privKeyList(){return QStringList().fromStdList( privKeys.keys().toStdList() ); }

};

#endif // TESTVERIFIER_H
