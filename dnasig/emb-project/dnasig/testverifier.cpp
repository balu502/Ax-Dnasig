#include "testverifier.h"


#include <QDebug>
#include <QFileInfo>
#include <QDir>

#ifndef XMLSEC_NO_XSLT
    xsltSecurityPrefsPtr xsltSecPrefs = NULL;
#endif /* XMLSEC_NO_XSLT */

#define QC_STR(s)  (s.toStdString().c_str())
#define QSTRXML(s) ((const xmlChar *) QC_STR(s))


TestVerifier::TestVerifier()
    :   CountryCode ("RU")
    ,   Org         ("DNA-Technology Ltd. (www.dna-technology.ru)")
    ,   CommonName  ("DNASIG")
    ,   daysLimit   (365)
{
    _error = 0;

    /* Init libxml and libxslt libraries */
    xmlInitParser();
    LIBXML_TEST_VERSION
    xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
    xmlSubstituteEntitiesDefault(1);
#ifndef XMLSEC_NO_XSLT
    xmlIndentTreeOutput = 1;
#endif /* XMLSEC_NO_XSLT */

    /* Init libxslt */
#ifndef XMLSEC_NO_XSLT
    /* disable everything */
    xsltSecPrefs = xsltNewSecurityPrefs();
    xsltSetSecurityPrefs(xsltSecPrefs,  XSLT_SECPREF_READ_FILE,        xsltSecurityForbid);
    xsltSetSecurityPrefs(xsltSecPrefs,  XSLT_SECPREF_WRITE_FILE,       xsltSecurityForbid);
    xsltSetSecurityPrefs(xsltSecPrefs,  XSLT_SECPREF_CREATE_DIRECTORY, xsltSecurityForbid);
    xsltSetSecurityPrefs(xsltSecPrefs,  XSLT_SECPREF_READ_NETWORK,     xsltSecurityForbid);
    xsltSetSecurityPrefs(xsltSecPrefs,  XSLT_SECPREF_WRITE_NETWORK,    xsltSecurityForbid);
    xsltSetDefaultSecurityPrefs(xsltSecPrefs);
#endif /* XMLSEC_NO_XSLT */

    /* Init xmlsec library */
    if(xmlSecInit() < 0) {
        qWarning("Error: xmlsec initialization failed.");
        _error = -1;
    }

    /* Check loaded library version */
    if(xmlSecCheckVersion() != 1) {
        qWarning("Error: loaded xmlsec library version is not compatible.");
        _error  = -2;
    }

    /* Load default crypto engine if we are supporting dynamic
     * loading for xmlsec-crypto libraries. Use the crypto library
     * name ("openssl", "nss", etc.) to load corresponding
     * xmlsec-crypto library.
     */
#ifdef XMLSEC_CRYPTO_DYNAMIC_LOADING
    if(xmlSecCryptoDLLoadLibrary(NULL) < 0) {
        qWarning(   "Error: unable to load default xmlsec-crypto library. Make sure\n"
                    "that you have it installed and check shared libraries path\n"
                    "(LD_LIBRARY_PATH and/or LTDL_LIBRARY_PATH) envornment variables.\n");
        _error  = -3;
    }
#endif /* XMLSEC_CRYPTO_DYNAMIC_LOADING */

    /* Init crypto library */
    if(xmlSecCryptoAppInit(NULL) < 0) {
        qWarning( "Error: crypto initialization failed.");
        _error = -4;
    }

    /* Init xmlsec-crypto library */
    if(xmlSecCryptoInit() < 0) {
        qWarning("Error: xmlsec-crypto initialization failed.");
        _error = -5;
    }
}

TestVerifier::~TestVerifier(){

    qDebug() << _error << "err code TestVerifier";


    if ( _error ) return;


    qDeleteAll(pubKeys);
    qDeleteAll(privKeys);

     pubKeys.clear();
    privKeys.clear();


    /* Shutdown xmlsec-crypto library */
    qDebug() << xmlSecCryptoShutdown() << "xmlSecCryptoShutdown()";

    /* Shutdown crypto library */
    qDebug() << xmlSecCryptoAppShutdown() << "xmlSecCryptoAppShutdown()";

    /* Shutdown xmlsec library */
    qDebug() << xmlSecShutdown() << "xmlSecShutdown()";

    /* Shutdown libxslt/libxml */
#ifndef XMLSEC_NO_XSLT
    xsltFreeSecurityPrefs(xsltSecPrefs);
    xsltCleanupGlobals();
#endif /* XMLSEC_NO_XSLT */
    xmlCleanupParser();

    qDebug() << "~TestVerifier";
}



bool TestVerifier::insertPublic(QString pathP){


    xmlSecKeyPtr ky = xmlSecCryptoAppKeyLoad( QC_STR(pathP),
                            xmlSecKeyDataFormatPem, NULL, NULL, NULL);

    if( ky == NULL) {
        qWarning() << "Error: failed to load public pem key from" << pathP;
        return false;
    }

    /* set key name to the file name, this is just an example! */
    QString key_file = QFileInfo(pathP).fileName();

    if(xmlSecKeySetName(ky, QSTRXML(key_file)) < 0) {
        qWarning() << "Error: failed to set key name for key from" << key_file;
    //        return false;
    }

    pubKeys[ key_file ] = ky;

    qDebug() << pubKeys.count() << " pub keys ready";

    return true;
}



bool TestVerifier::insertPrivate(QString pathP){

    /* set key name to the file name, this is just an example! */
    QFileInfo inFile(pathP);
    QString key_file = inFile.fileName();
    QString crt_file = QString("%1/%2.cer")
                        .arg(inFile.absolutePath())
                        .arg( inFile.baseName() );

    if( !QFile::exists( crt_file ) ){
        qWarning() << "Warning: cannot find certificate"
                   << crt_file;
        return false;
    }

    QString pp = privKeys.empty() ? "" : privKeys.keys().first();

    xmlSecKeyPtr ky = privKeys[ key_file ] =
            xmlSecCryptoAppKeyLoad( QC_STR(pathP),
                    xmlSecKeyDataFormatPem, NULL, NULL, NULL);

    if( ky == NULL) {
        privKeys.remove( key_file );
        qWarning() << "Error: failed to load private pem key from"
                   << pathP;
        return false;
    }

    /* load certificate and add to the key */
    if( 0 > xmlSecCryptoAppKeyCertLoad(ky, QC_STR(crt_file), xmlSecKeyDataFormatPem )) {
        privKeys.remove( key_file );
        qWarning() << "Warning: failed to load pem certificate "
                   << crt_file;
        return false;
    }

    if( 0 > xmlSecKeySetName(ky, QSTRXML(key_file)) ) {
        privKeys.remove( key_file );
        qWarning() << "Warning: failed to set key name for key from"
                   << key_file;
        return false;
    }

//    privKeys.clear();
//    privKeys[ key_file ] = ky;

    // remove previous private key
    privKeys.remove( pp );

    return true;
}

bool TestVerifier::removePublic(QString pathP)
{
    if (pubKeys.contains(pathP)){
        pubKeys.remove(pathP);

        return  true;
    }

    return false;
}



bool TestVerifier::isValidated(QString s, QString pubK){

    if ( pubKeys.empty() )
        return false;

    if ( pubK.isEmpty() )
         pubK = pubKeys.keys().first();
    else
        if ( !pubKeys.contains(pubK) )
            return false;

    xmlDocPtr   doc = NULL;
    xmlNodePtr node = NULL;

    doc = xmlParseDoc( QSTRXML(s) );

    if ((doc == NULL) || (xmlDocGetRootElement(doc) == NULL)){
        qWarning( "Error: unable to parse string" );
        return false;
    }


    /* find start node */
    node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
    if(node == NULL) {
        qWarning( "Warning: signature node not found");
        xmlFreeDoc(doc);
        return false;
    }

    qDebug() << "... parsed";

    return  validateNodeByKey( node, pubKeys[pubK] );
}


bool TestVerifier::validateNodeByKey(xmlNodePtr n, xmlSecKeyPtr k){

    bool    stOk    = false;
    xmlSecDSigCtxPtr
            dsigCtx = xmlSecDSigCtxCreate(NULL);

    if(dsigCtx == NULL) {
        qWarning("Error: failed to create signature context");
        return  stOk;
    }

    dsigCtx->signKey = k;

    /* Verify signature */
    if( 0 > xmlSecDSigCtxVerify( dsigCtx, n ) ) {
        qWarning("Error: cannot verify");
//        xmlFreeDoc(doc);
//        return false;
    } else {

//    bool
        stOk = (dsigCtx->status == xmlSecDSigStatusSucceeded);
        stOk? qDebug("Signature is OK")
            : qDebug("Signature is INVALID");
    }
//    xmlFreeDoc(doc);

    dsigCtx->signKey = NULL;
    xmlSecDSigCtxDestroy(dsigCtx);

    return stOk;
}



QString TestVerifier::sign(QString s, QString privK){

    if ( privKeys.empty() )
        return false;

    if ( privK.isEmpty() )
         privK = privKeys.keys().first();
    else
        if ( !privKeys.contains(privK) )
            return false;

    xmlDocPtr          doc = NULL;
    xmlNodePtr    signNode = NULL;
    xmlNodePtr     refNode = NULL;
    xmlNodePtr keyInfoNode = NULL;


    /* load doc file */
    doc = xmlParseDoc( QSTRXML(s) );

    if ((doc == NULL) || (xmlDocGetRootElement(doc) == NULL)){
        qWarning( "Error: unable to parse string" );
        return false;
    }

//    qDebug() << s;

    /* find start node */
    signNode = xmlSecFindNode(xmlDocGetRootElement(doc),
                              xmlSecNodeSignature, xmlSecDSigNs);

    if(signNode != NULL) {
        qWarning( "Info: signature node presents");

//        xmlNodePtr newSig = xmlSecTmplSignatureCreate(
//                                    doc, xmlSecTransformExclC14NId,
//                                    xmlSecTransformRsaSha1Id, NULL);

//        xmlReplaceNode( signNode, newSig );
//        signNode = newSig;

    }else{
//    /* create signature template for RSA-SHA1 enveloped signature */
//        signNode = xmlSecTmplSignatureCreate(
//                                doc, xmlSecTransformExclC14NId,
//                                xmlSecTransformRsaSha1Id, NULL);

//        if(signNode == NULL) {
//            qWarning("Error: failed to create signature template");
//            xmlFreeDoc(doc);
//            return "";
//        }

//        /* add <dsig:Signature/> node to the doc */
//        xmlAddChild(xmlDocGetRootElement(doc), signNode);
    }


//    /* add reference */
//    refNode = xmlSecTmplSignatureAddReference(signNode,
//                    xmlSecTransformSha1Id, NULL, NULL, NULL);

//    if(refNode == NULL) {
//        qWarning("Error: failed to add reference to signature template");
//        xmlFreeDoc(doc);
//        return "";
//    }

//    /* add enveloped transform */
//    if(xmlSecTmplReferenceAddTransform(refNode, xmlSecTransformEnvelopedId) == NULL) {
//        qWarning("Error: failed to add enveloped transform to reference");
//        xmlFreeDoc(doc);
//        return "";
//    }

//    /* add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to put key name in the signed document */
//    keyInfoNode = xmlSecTmplSignatureEnsureKeyInfo(signNode, NULL);
//    if(keyInfoNode == NULL) {
//        qWarning("Error: failed to add key info");
////        return false;
//    }

//    if(xmlSecTmplKeyInfoAddKeyName(keyInfoNode, NULL) == NULL) {
//        qWarning("Error: failed to add key name");
////        return false;
//    }


//    xmlSecDSigCtxPtr dsigCtx = privKeys[privK];
    xmlSecDSigCtxPtr dsigCtx = xmlSecDSigCtxCreate(NULL);

    if(dsigCtx == NULL) {
        qWarning("Error: failed to create signature context");
        return false;
    }

    dsigCtx->signKey = privKeys[privK];

    int rsc = xmlSecDSigCtxSign(dsigCtx, signNode);
    /* sign the template */
    if(rsc < 0) {
        qWarning(QC_STR(QString("Error: signature failed with key %1, code:%2")
                        .arg(privK).arg(rsc)));

        xmlFreeDoc(doc);
        dsigCtx->signKey = NULL;
        xmlSecDSigCtxDestroy(dsigCtx);
        return "";
    }

    /* return signed doc */
    xmlChar   *xmlbuff;
    int     buffersize;

    xmlDocDumpMemory(doc, &xmlbuff, &buffersize);
    QString retXml( (const char *) xmlbuff );

//    xmlSaveFile( QString("%1_docsig.xml").arg( (int)xmlbuff ).toStdString().c_str(), doc );

    xmlFree(xmlbuff);
    xmlFreeDoc(doc);
    dsigCtx->signKey = NULL;
    xmlSecDSigCtxDestroy(dsigCtx);


    return retXml;
}




/* Generates a 2048-bit RSA key. */
EVP_PKEY * TestVerifier::generateKey()
{
    /* Allocate memory for the EVP_PKEY structure. */
    EVP_PKEY * pkey = EVP_PKEY_new();
    if(!pkey)
    {
        qWarning("Unable to create EVP_PKEY structure.");
        return NULL;
    }

    /* Generate the RSA key and assign it to pkey. */
    RSA * rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
    if(!EVP_PKEY_assign_RSA(pkey, rsa))
    {
        qWarning( "Unable to generate 2048-bit RSA key." );
        EVP_PKEY_free(pkey);
        return NULL;
    }

    /* The key has been generated, return it. */
    return pkey;
}


/* Generates a self-signed x509 certificate. */
X509 * TestVerifier::generateX509(EVP_PKEY * pkey)
{
    /* Allocate memory for the X509 structure. */
    X509 * x509 = X509_new();
    if(!x509)
    {
        qWarning("Unable to create X509 structure.");
        return NULL;
    }

    /* Set the serial number. */
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

    /* This certificate is valid from now until exactly one year from now. */
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), daysLimit * 24L * 3600L );
//    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);


    /* Set the public key for our certificate. */
    X509_set_pubkey(x509, pkey);

    /* We want to copy the subject name to the issuer name. */
    X509_NAME * name = X509_get_subject_name(x509);

    /* Set the country code and common name. */
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *) QC_STR(CountryCode), -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *) QC_STR(Org),         -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *) QC_STR(CommonName),  -1, -1, 0);

    /* Now set the issuer name. */
    X509_set_issuer_name(x509, name);

    /* Actually sign the certificate with our key. */
    if(!X509_sign(x509, pkey, EVP_sha1()))
    {
        qWarning("Error signing certificate.");
        X509_free(x509);
        return NULL;
    }

    return x509;
}


bool TestVerifier::writeKeys(EVP_PKEY *pkey, QString k, QString p, X509 *x509, QString c)
{
    /* Open the PEM file for writing the key to disk. */
    FILE * pkey_file = fopen(QC_STR(k), "wb");
    if(!pkey_file)
    {
        qWarning("Unable to open \"%s\" for writing.", k);
        return false;
    }

    /* Write the key to disk. */
    bool ret = PEM_write_PrivateKey(pkey_file, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(pkey_file);

    if(!ret)
    {
        qWarning( "Unable to write private key to disk." );
        return false;
    }

    /* Open the PEM file for writing the certificate to disk. */
    FILE * x509_file = fopen(QC_STR(c), "wb");
    if(!x509_file)
    {
        qWarning( "Unable to open \"%s\" for writing.", c );
        return false;
    }

    /* Write the certificate to disk. */
    ret = PEM_write_X509(x509_file, x509);
    fclose(x509_file);

    if(!ret)
    {
        qWarning( "Unable to write certificate to disk." );
        return false;
    }


    /* Open the PEM file for writing the public key to disk. */
    FILE * pub_file = fopen(QC_STR(p), "wb");
    if(!pub_file)
    {
        qWarning("Unable to open \"%s\" for writing.", p);
        return false;
    }

    /* Write the public key to disk. */
    ret = PEM_write_PUBKEY(pub_file, pkey);
    fclose(pub_file);

    if(!ret)
    {
        qWarning( "Unable to write public key to disk." );
        return false;
    }


    return true;
}


bool TestVerifier::generateKeys( QString keyN, QString pubN, QString crtN ){

    qDebug() << "Generating RSA key...";

    EVP_PKEY * pkey = generateKey();
    if(!pkey)
        return false;

    qDebug() << "Generating x509 certificate...";

    X509 * x509 = generateX509(pkey);
    if(!x509)
    {
        EVP_PKEY_free(pkey);
        return false;
    }

    qDebug() << "Writing key and certificate to disk..." ;

    bool ret = writeKeys(pkey, keyN, pubN, x509, crtN);
    EVP_PKEY_free(pkey);
    X509_free(x509);

    if(ret){

        qDebug() << "Success!" ;
        qDebug() << keyN << "private key loading - "
                 << insertPrivate( keyN );

        qDebug() << pubN << "public key loading - "
                 << insertPublic(  pubN );

        return true;
    }

    return false;
}
