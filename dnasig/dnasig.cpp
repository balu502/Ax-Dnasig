#include "dnasig.h"
#include "testverifier.h"

#include <QTimer>
#include <QDebug>
#include <QTranslator>
#include <QSettings>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QListWidget>
#include <QFileDialog>
#include <QDomDocument>
#include <QMessageBox>
#include <QProcess>
#include <QThread>
#include <QTextStream>

#include <iostream>

#ifdef WINBUILD
#include <Windows.h>
#endif

#ifndef GUIBUILD
    #include <iostream>
#endif

Qt::WindowFlags WNORMFLAGS
            = Qt::Window;
//              Qt::Dialog
//            | Qt::CustomizeWindowHint
//            | Qt::WindowTitleHint
//            | Qt::WindowStaysOnTopHint;

Qt::WindowFlags WTOPFLAGS
            =   Qt::Dialog
            | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowStaysOnTopHint;

QString nodeText( QDomNode tNode );
bool    loadXmlFile( QDomDocument &doc, QString filePath );
int     removeSignatures( QDomDocument &docTest );
void    openFileExplorer(QString action, QString filePath);
void    messageBoxTimeout(QString, int);


int DnaSig::appCounter;

DnaSig::DnaSig(QObject *parent)
    : QObject   (parent)
    , dialog    (0)
    , sDialog   (0)
    , testList  (0)
    , translator(new QTranslator())
    , verifier  (new TestVerifier())
{

    appCounter++;


    setObjectName( "DNASIG_from_QAxFactory" );


    QDir::setCurrent(
        QFileInfo( qApp->applicationFilePath() )
                            .absoluteDir().absolutePath() );

    qApp->addLibraryPath(   qApp->applicationDirPath() );

    settings= new QSettings(QSettings::UserScope,
                            qApp->organizationName(),
                            qApp->applicationName() );

    int vic = verifier->isInvalid();

    qDebug()<< "appCounter -"       << appCounter
            << "\nVerifier status -"<< (!vic)
            << "\nCurrent path -"   << QDir::currentPath()
            << "\nLib paths -"      << qApp->libraryPaths()
            << "\nApp path -"       << qApp->applicationFilePath();


    if ( vic ){        
        messageBoxTimeout(tr(   "Error %1, verification "
                                "context is not ready.")
                                .arg(vic), 5000 );        
        return;
    }

    QString k;

    k = settings->value("main/privatekey", "key.pem").toString();
    qDebug() << k << "private key loading - "
             << verifier->insertPrivate( k );

    foreach( k, settings->value("main/publickeys", "pub.pem").toString().split(";") )
        qDebug()<< k << "public key loading - "
                << verifier->insertPublic( k );


    if (verifier->pubKeyList().empty()
    &&  QFile::exists(k = "pub.pem"))
        qDebug()<< k << "public key loading - "
                << verifier->insertPublic( k );


    if (verifier->pubKeyList().empty())
        messageBoxTimeout(
            tr( "You have not any keys for verification,\n"
                "please open settings and specify public keys."), 5000 );
}

DnaSig::~DnaSig(){

    if (!--appCounter){

        settings->setValue("main/publickeys",
                verifier->pubKeyList().join(";"));

        if (verifier->privKeyList().count())
            settings->setValue("main/privatekey",
                verifier->privKeyList().first() );

        settings->sync();

        if (verifier){  delete  verifier;
                                verifier = 0;
        }
    }


    if( sDialog ){
        sDialog->deleteLater();
        sDialog = 0;
    }

      settings->deleteLater();
    translator->deleteLater();

//    if (!appCounter){
//        if (verifier)
//            delete verifier;
//        verifier = 0;
//    }
}


void DnaSig::quit(){

    if (verifier){  delete  verifier;
                            verifier = 0;
    }

    QTimer::singleShot(0, qApp , SLOT(quit()));
}


int DnaSig::execute(bool isAx){

    if (!dialog){

        dialog = new QDialog(0, WNORMFLAGS);
        sDialog= new QDialog(0, WNORMFLAGS);

        translator->isEmpty()
        &&  setLocale( settings->value( "main/locale", "en" ).toString() );

        buildLayout();
        buildSettingsLayout();
    }

    return dialog->exec();
}


int DnaSig::setLocale(QString appLang) const
{
    int retVal = 0;

    QString rtT = "dnasig_" + appLang;

    if (!translator->load( rtT, qApp->applicationDirPath() )){
        qDebug() << QString("Error while loading translator %1").arg(rtT) ;

    }else
        retVal = qApp->installTranslator( translator );

    settings->setValue("main/locale", appLang);

    return  retVal;
}



void DnaSig::buildLayout(){

    QDialogButtonBox
                *dBx    = new QDialogButtonBox(Qt::Horizontal, dialog);
    QVBoxLayout *dLayout= new QVBoxLayout(dialog);
                testList= new QListWidget(dialog);

    QLabel  *picDecor= new QLabel(dialog);
             picDecor->setPixmap( QPixmap(":/img/dnasig.png") );
             picDecor->setScaledContents(true);
             picDecor->setFixedHeight( 84 );


    dLayout->addWidget( picDecor, 0, Qt::AlignLeft );
    dLayout->addWidget( testList);
    dLayout->addWidget( dBx, 0, Qt::AlignHCenter);
//    dLayout->addWidget(sbInfo);

    dialog->setLayout( dLayout );
    //dialog->setWindowTitle(tr("DNA-Technology Signature Manager"));
    dialog->setObjectName("QDialog_dialog");
//    dialog->setFixedSize(310, 200);
    dialog->setWindowTitle(
        QString("%1, %2").arg(qApp->applicationName())
                         .arg(qApp->organizationName()));

//    sbInfo->setSizeGripEnabled(false);

    QPushButton *btFile = dBx->addButton( tr("Load.."),     QDialogButtonBox::ActionRole );
    QPushButton *btSign = dBx->addButton( tr("Sign"),       QDialogButtonBox::ActionRole );
    QPushButton *btEdit = dBx->addButton( tr("Settings"),   QDialogButtonBox::ActionRole );
    QPushButton *btExp  = dBx->addButton( tr("Save"),       QDialogButtonBox::AcceptRole );
    QPushButton *btClear= dBx->addButton( tr("Clear"),      QDialogButtonBox::ActionRole );
    QPushButton *btCancel=dBx->addButton( tr("Close"),      QDialogButtonBox::RejectRole );

    btSign->setEnabled( verifier->privKeyList().count() );
     btExp->setEnabled( false );

    connect( this,      SIGNAL(testsLoaded(bool)),  btExp, SLOT(setEnabled(bool)) );
    connect( this,      SIGNAL(privateLoaded(bool)),btSign,SLOT(setEnabled(bool)) );

    connect( btExp,     SIGNAL(clicked()),  this,    SLOT(exportSigned()));
    connect( btSign,    SIGNAL(clicked()),  this,    SLOT(signTests()));
    connect( btEdit,    SIGNAL(clicked()),  sDialog, SLOT(exec()));
    connect( btFile,    SIGNAL(clicked()),  this,    SLOT(appendFile()));
    connect( btClear,   SIGNAL(clicked()),  testList,SLOT(clear()));
    connect( btCancel,  SIGNAL(clicked()),  dialog,  SLOT(reject()));
}


void DnaSig::buildSettingsLayout(){

    QGridLayout *dLayout = new QGridLayout(sDialog);
    QDialogButtonBox *dBx= new QDialogButtonBox(Qt::Horizontal, sDialog);
    QDialogButtonBox *dBx2=new QDialogButtonBox(Qt::Vertical,   sDialog);
    QDialogButtonBox *dBx3=new QDialogButtonBox(Qt::Vertical,   sDialog);

    pubFList    = new QListWidget(sDialog);
    editPrivate = new QLineEdit(sDialog);
    editPrivate->setReadOnly(true);
    editPrivate->setPlaceholderText( tr("Private key file path..") );

    QLabel      *vL;
    QPushButton *bt;

//    QLabel      *picDecor= new QLabel(sDialog);
//                 picDecor->setPixmap( QPixmap(":/img/dnasig.png") );

    QDialogButtonBox::ButtonRole
            acR = QDialogButtonBox::ActionRole;

    (bt = dBx2->addButton(tr("*"),   acR))->setToolTip(tr("Generate private key"));
        connect(bt, SIGNAL(clicked()), this, SLOT(generateKeys()));

    (bt = dBx2->addButton(tr("..."), acR))->setToolTip(tr("Load private key"));
        connect(bt, SIGNAL(clicked()), this, SLOT(selectPrivKey()));

    (bt = dBx3->addButton(tr("+"), acR))->setToolTip(tr("Append public key"));;
        connect(bt, SIGNAL(clicked()), this, SLOT(selectPubKey()));

    (bt = dBx3->addButton(tr("-"), acR))->setToolTip(tr("Remove selected public key"));;
        connect(bt, SIGNAL(clicked()), this, SLOT(removePubKey()));


    dLayout->addWidget( (new QLabel(tr("Private key (for make signature)"))),
                                                                0, 0, 1, 3 );
    dLayout->addWidget( editPrivate,                               1, 0, 1, 2 );
    dLayout->addWidget( dBx2,                                   1, 2, 4, 1 );

    dLayout->addWidget( vL = new QLabel(tr("C (country code)")),2, 0 );
    dLayout->addWidget( edC= (new QLineEdit()),                 2, 1 );
                        edC->setToolTip(vL->text());

    dLayout->addWidget( vL = new QLabel(tr("CN (common name)")),3, 0 );
    dLayout->addWidget( edCN=(new QLineEdit()),                 3, 1 );
                        edCN->setToolTip(vL->text());

    dLayout->addWidget( vL = new QLabel(tr("O (org. name)")),   4, 0 );
    dLayout->addWidget( edO= (new QLineEdit()),                 4, 1 );
                        edO->setToolTip(vL->text());

    dLayout->addWidget( vL = new QLabel(tr("Duration (days)")), 5, 0 );
    dLayout->addWidget( edD= (new QLineEdit()),                 5, 1 );
                        edD->setToolTip(vL->text());
                        edD->setText("365");

    dLayout->addWidget( (new QLabel(tr("Public keys (for verify data)")) ),
                                                                6, 0, 1, 3 );
    dLayout->addWidget( pubFList,                               7, 0, 1, 2 );
    dLayout->addWidget( dBx3,                                   7, 2 );

    dLayout->addWidget( dBx,                                    8, 0, 1, 3 );



//    dLayout->addWidget( picDecor);
//    dLayout->addWidget( testList);
//    dLayout->addWidget( dBx, 0, Qt::AlignHCenter);
//    dLayout->addWidget(sbInfo);

    sDialog->setLayout( dLayout );
    //dialog->setWindowTitle(tr("DNA-Technology Signature Manager"));
    sDialog->setObjectName("QDialog_sDialog");
//    dialog->setFixedSize(310, 200);
    sDialog->setWindowTitle(
        QString("%1, %2").arg(tr("Settings"))
                         .arg(qApp->applicationName()));

    QPushButton *btCancel=dBx->addButton( tr("Close"), QDialogButtonBox::RejectRole );

    connect(btCancel, SIGNAL(clicked()),    sDialog, SLOT(hide()));
    connect(dialog, SIGNAL(finished(int)),  sDialog, SLOT(done(int)));

    loadDialogSettings();
}

bool DnaSig::checkTextInput(QLineEdit *ei){

    if (ei->text().trimmed().isEmpty()){
        QMessageBox::warning(sDialog, sDialog->windowTitle(),
                tr("Please input correct value for %1").arg(ei->toolTip()));
        return false;
    }

    return true;
}

void DnaSig::generateKeys(){

    if (verifier->privKeyList().count()
    && (QMessageBox::Cancel ==
            QMessageBox::warning(sDialog, sDialog->windowTitle(),
                tr("Existing private key %1 will no longer be used?")
                        .arg(verifier->privKeyList().first()),
                QMessageBox::Ok|QMessageBox::Cancel)))
        return;

    if (!checkTextInput(edD)
    ||  !checkTextInput(edC)
    ||  !checkTextInput(edCN)
    ||  !checkTextInput(edO))
        return;

    if (!edD->text().toInt()){
        QMessageBox::warning(sDialog, sDialog->windowTitle(),
                tr("Please input correct value for %1").arg(edD->toolTip()));
        return;
    }

    int v = qAbs( *((int*)&verifier) );
    QString pubName = QString("%1_pubkey.pem"   ).arg(v),
            privName= QString("%1_privkey.pem"  ).arg(v),
            crtName = QString("%1_privkey.cer"  ).arg(v);

    verifier->daysLimit     = edD->text().toInt();
    verifier->CountryCode   = edC->text().trimmed();
    verifier->Org           = edO->text().trimmed();
    verifier->CommonName    = edCN->text().trimmed();


    if (!verifier->generateKeys( privName, pubName, crtName )){
        QMessageBox::warning(sDialog, sDialog->windowTitle(),
                tr("Error while generating keys"));
        return;
    }

    editPrivate->setText( verifier->privKeyList().first() );

    pubFList->clear();
    pubFList->addItems( verifier->pubKeyList() );

    edD->setText("365");
    edO->clear();   edC->clear();   edCN->clear();

    openFileExplorer("select", qApp->applicationDirPath() +"/"+ crtName);

    emit privateLoaded(true);
}


void DnaSig::selectPrivKey(){

    QString fileName = QFileDialog::getOpenFileName(
                dialog,  tr("Open Private key"), QString(),
                tr("OpenSSL key files (*.pem *.key)"));

    if (QFile::exists( fileName )
    &&  verifier->insertPrivate( fileName )){
        editPrivate->setText( QFileInfo( fileName ).fileName() );
        editPrivate->setToolTip( QFileInfo( fileName ).filePath() );

    }else{
        ( !fileName.isEmpty() )
        && QMessageBox::warning(sDialog, sDialog->windowTitle(),
                tr("Selected file %1 is incorrect or not supported")
                             .arg(fileName));
    }

    edO->clear();   edC->clear();   edCN->clear();

    emit privateLoaded(true);
}


void DnaSig::selectPubKey(){

    QString fileName = QFileDialog::getOpenFileName(
                            dialog,  tr("Open Public key"), QString(),
                            tr("OpenSSL key files (*.pem *.key)"));

    if (!QFile::exists( fileName ))
        return;

    if (verifier->insertPublic( fileName )){

        pubFList->clear();
        pubFList->addItems( verifier->pubKeyList() );

    }else{
        ( !fileName.isEmpty() )
        && QMessageBox::warning(sDialog, sDialog->windowTitle(),
                tr("Selected file %1 is incorrect or not supported")
                             .arg(fileName));
    }
}


void DnaSig::removePubKey(){

    QList<QListWidgetItem*> remL;

    if ((remL = pubFList->selectedItems()).empty()
    || (QMessageBox::No ==
            QMessageBox::warning(sDialog, sDialog->windowTitle(),
                tr("Selected public key %1 will no longer used. Proceed?")
                        .arg(remL.first()->text()),
                QMessageBox::Yes|QMessageBox::No)))
        return;

    if (verifier->removePublic( remL.first()->text() ))
        delete pubFList->takeItem( pubFList->row( remL.first() ) );
    else
        QMessageBox::warning(sDialog, sDialog->windowTitle(),
                             tr("Cannot delete key %1").arg( remL.first()->text() ));
}


void DnaSig::loadDialogSettings()
{
    pubFList->clear();

    if (verifier->privKeyList().count())
        editPrivate->setText( verifier->privKeyList().first() );

    pubFList->addItems( verifier->pubKeyList() );
}


//QString nodeText( QDomNode tNode ){

//    QString *tString = &(QString());
//    QTextStream tStream(tString);

//    tNode.save(tStream, -1);

//    return  (*tString);
//}


QString nodeText( QDomNode tNode ){

    QString     tString;
    QTextStream tStream( &tString );

    tNode.save(tStream, -1);

    return  tString;
}



bool loadXmlFile( QDomDocument &doc, QString filePath ){

    if (!QFile::exists( filePath ))
        return false;

    QFile file( filePath );

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}


bool DnaSig::appendStream(QTextStream *textStrm, bool applySign){

    QDomDocument doc("TESTs");

    if (!doc.setContent(textStrm->readAll())){
        qDebug() << "Cannot parse File";
        return  false;
    }

    QDomElement
        docElem = doc.documentElement();

    return  appendDomElement( &docElem );
}


bool DnaSig::appendFile(QString fileName, bool applySign){

    QDomDocument doc("TESTs");

    if (fileName.isEmpty()){
        fileName = QFileDialog::getOpenFileName(
                    dialog,  tr("Open File"), QString(),
                    tr("DNA-Tech. tests (*.xrt *.xml)"));
    }

    if (!loadXmlFile( doc, fileName )){
        qDebug() << "Cannot parse File";
        return  false;
    }

    QDomElement
        docElem = doc.documentElement();

    return  appendDomElement( &docElem );
}


bool DnaSig::appendDomElement(QDomElement *docElem, bool applySign){

    if ( "tests" != docElem->tagName().toLower() ){
        qDebug() << "Unsupported XML format";
        return  false;
    }

//    QDomNodeList tItems = docElem.elementsByTagName("item");
    QDomNodeList tItems = docElem->childNodes();
    QDomNode     iTmp;
    QStringList  tsList;
    int vcnt = 0, scnt = 0, icnt=0;

#ifndef GUIBUILD
    std::wcout << "<TESTs>\n";
#endif

    for( int i=0; i < tItems.count(); i++)
        if ("item" == tItems.at(i).nodeName()){
            iTmp = tItems.at(i);
            vcnt += loadTest( &iTmp );
            icnt++;

            if( applySign ){
                QString st = signTests( &iTmp );
                bool    est= st.isEmpty();

                if (!est) tsList << st;

                qDebug() << "Apply signature result is - " << !est;
                scnt += est;
            }
        }

#ifndef GUIBUILD
    std::wcout << "</TESTs>\n";
#endif

    qDebug() << "Total" << icnt
             << "tests accepted,"
             << vcnt << "verificated successfuly";

#ifdef GUIBUILD
    emit testsLoaded(testList->count());
#endif

    if (applySign)
        exportSigned( tsList );

    return  applySign
            ? (scnt == icnt)
            : (vcnt == icnt);
}


bool DnaSig::loadXmlTest( QString xmlTest ){

    QDomDocument    doc("item");
    QDomElement     tmpElement;

    if (!doc.setContent(xmlTest))
        return false;

    tmpElement = doc.documentElement();

    return loadTest( &tmpElement );
}

QString DnaSig::signXmlTest( QString xmlTest ){

    QDomDocument    doc("item");
    QDomElement     tmpElement;

    if (!doc.setContent(xmlTest))
        return QString();

    tmpElement = doc.documentElement();

    return signTests( &tmpElement );
}


QString DnaSig::privateKeys(){

    return verifier->privKeyList().join(";");
}

QString DnaSig::publicKeys(){

    return verifier->pubKeyList().join(";");
}



bool DnaSig::loadTest( QDomNode * tNode ){

    QString      tName, tId;
    QString      tString  = nodeText(*tNode);
    QDomNodeList nameItems= tNode->toElement().elementsByTagName("nameTest");
    QDomNodeList   idItems= tNode->toElement().elementsByTagName("IDTest");
    bool         verified = false;

    qDebug() << verifier->pubKeyList().count()
             << "pub key(s) available";

    if (nameItems.isEmpty()
    ||  verifier->pubKeyList().empty() )
        return verified;

    tName= nameItems.at(0).toElement().text();
    tId  =   idItems.count()
              ? idItems.at(0).toElement().text()
              : "no-id-item";

    QString  kf, ks;

    foreach( kf, verifier->pubKeyList() )
        if ( verified = verifier->isValidated(tString, kf) ){
            ks = kf;
            qDebug() << tName << "Signed with key" << ks;
            break;
        }


    if (testList){  // QListWidget

        QString verK = verified ? "[signed]" : "[------]";

        QListWidgetItem *item =
                new QListWidgetItem( QString("%1 %2")
                                     .arg( verK ).arg( tName ));

        item->setData(Qt::UserRole,  tName );
        item->setData(Qt::UserRole+1,tString );

        if (verified) item->setToolTip(
                        tr("Signed with %1 key").arg(kf));

        testList->addItem( item );
    }

    qDebug() << tName << "item processed";

#ifndef GUIBUILD
    std::wcout
        << "<item>"
        << "<IDTest>"   << tId.toStdWString()   << "</IDTest>"
        << "<nameTest>" << tName.toStdWString() << "</nameTest>"
        << "<sigk>"     << ks.toStdWString()    << "</sigk>"
        << "</item>\n";
#endif


    return verified;
}


void    messageBoxTimeout(QString m, int ms){
#ifdef GUIBUILD

    if (QThread::currentThread()
            != qApp->thread()){
        qDebug() << m ;
        return;
    }

    QMessageBox *mbox = new QMessageBox;
    mbox->setWindowTitle(qApp->applicationName());
    mbox->setText(m);
    mbox->setWindowFlags( WTOPFLAGS );
    QTimer::singleShot(ms, mbox, SLOT(hide()));
    mbox->exec();
#else
    qDebug() << m;
#endif
}


void openFileExplorer(QString action, QString filePath){

    QString program = "explorer.exe";
    QStringList arguments;
    arguments << QString("/%1,").arg(action)
              << filePath.replace('/',"\\") ;

    QProcess *myProcess = new QProcess();
    myProcess->start(program, arguments);
}


int removeSignatures( QDomDocument &docTest ){

    QDomNodeList
        sigNodes= docTest.documentElement().
                    elementsByTagName("Signature");

    int     cnt = sigNodes.count();

    for (int n=0; n < cnt; n++)
        docTest.documentElement().removeChild( sigNodes.at(n) );

    return cnt;
}


QString DnaSig::signTests(QDomNode *tNode){

    qDebug()<< "----------------------------\n"
            << verifier->privKeyList().count()
            << "priv keys available";

    if ( verifier->privKeyList().empty() ){
        QString wm = tr("Warning! You have not any keys for signing,\n"
                        "please open settings and define keys.");
        if ( testList )
             QMessageBox::warning(sDialog, sDialog->windowTitle(), wm);
        else qWarning( wm.toStdString().c_str() );

        return QString();
    }

    QListWidgetItem *item;
    QString         sigXml, s;
    QDomDocument    templateSig;

    if (!loadXmlFile( templateSig, ":/data/templateSig.xml" )){
        qWarning("Error: cannot load signature template!");
        return QString();
    }

    if ( tNode ){

        s       = nodeText( *tNode );
        sigXml  = signNodeText(s, &templateSig);

    }else
    for ( int i=0; i < testList->count(); i++ ){

        item  = testList->item( i );
        s     = item->data(Qt::UserRole+1).toString();
        sigXml= signNodeText(s, &templateSig);

        item->setData( Qt::UserRole+2, sigXml );
        item->setText( QString("%1 %2")
                            .arg( sigXml.length() ? "[signed]" : "[------]" )
                            .arg( item->data(Qt::UserRole).toString() )
                        );

        qDebug() << sigXml.length() << " signed length of "
                 << item->data(Qt::UserRole).toString()
                 << "\n";
    }

    return  sigXml;
}


QString DnaSig::signNodeText(QString tXml, QDomDocument *templateSig){

    QDomDocument docTest("item");
    if (!docTest.setContent( tXml ))
        return  "";

    removeSignatures( docTest );

    docTest.documentElement().appendChild(
        templateSig->documentElement().cloneNode());


    return  verifier->sign( nodeText( docTest.documentElement() ) );
}


void DnaSig::exportSigned(QStringList xmlTests){

    QDomDocument doc("TESTs");
    QDomElement root = doc.createElement("TESTs");

    doc.appendChild(root);

    QString         sigXml;
    bool needWrite = false;

    if ( testList ){

        QListWidgetItem  *item;

        for ( int i=0; i < testList->count(); i++ ){

            item  = testList->item( i );
            sigXml= item->data( Qt::UserRole+2 ).toString();

            if (!sigXml.isEmpty())
                xmlTests << sigXml;
        }
    }

    foreach ( QString t, xmlTests ){

        QDomDocument docTest("item");

        if (docTest.setContent( t )){
            root.appendChild(
                docTest.documentElement().cloneNode() );
            needWrite = true;
        }
    }


//    for ( int i=0; i < testList->count(); i++ ){

//        item  = testList->item( i );
//        sigXml= item->data( Qt::UserRole+2 ).toString();

//        if (sigXml.isEmpty()) continue;

//        QDomDocument docTest("item");

////        qDebug() << "~~~" << sigXml << "***";

//        if (docTest.setContent( sigXml )){
//            root.appendChild(
//                docTest.documentElement().cloneNode() );
//            needWrite = true;
//        }
//    }

    if (!needWrite){
        QMessageBox::warning(sDialog,
            sDialog->windowTitle(), tr("Nothing to save"));
        return;
    }


    qDebug() << "Saving results";

    QFile file("exportTests.xml");
    if (!file.open(QIODevice::WriteOnly)){
        qDebug() << "...faulted";

        QMessageBox::warning(sDialog,
            sDialog->windowTitle(),
            tr("Cannot write file %1").arg(file.fileName()));
        return;
    }

//    doc.save( QTextStream(&file), -1 );
    QTextStream ts( &file );
    doc.save(   ts, -1 );
    file.close();

    qDebug() << "...Complete";

    openFileExplorer( "select",
        qApp->applicationDirPath() +"/"+ file.fileName());
}

