#ifndef DNASIG_H
#define DNASIG_H

#include <QWidget>
#include <QCoreApplication>

class QDialog;
class QLabel;
class QComboBox;
class QLineEdit;
class QListWidget;
class QCheckBox;
class QPushButton;
class QSettings;
class QListWidgetItem;
class QTranslator;
class QSettings;
class QDomNode;
class QDomDocument;
class TestVerifier;

class DnaSig : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("ClassID",      "{6DCA6D52-6D57-44ae-AFDC-2C4332680093}")
    Q_CLASSINFO("InterfaceID",  "{5275EDF3-B653-4c85-BE19-3D90B157F0BD}")
    Q_CLASSINFO("RegisterObject", "yes")


    QDialog     *dialog;
    QDialog     *sDialog;
    QTranslator *translator;
    QSettings   *settings;
    TestVerifier *verifier;

    QListWidget *testList, *pubFList;

    QLineEdit   *edC, *edCN, *edO, *edD;
    QLineEdit   *editPrivate;

    static int appCounter;

public:
    DnaSig(QObject *parent = 0);
    ~DnaSig();

public slots:
    void    quit();
    int     execute(bool isAx=true);
    int     setLocale(QString) const;
    bool    loadXmlTest( QString xmlTest );
    QString signXmlTest( QString xmlTest );
    bool    appendFile(QString fileName=QString(), bool applySign=false);

    QString privateKeys();
    QString  publicKeys();

private slots:
    void    buildLayout();
    void    buildSettingsLayout();
//    void    editSettings();
    bool    loadTest (QDomNode *tNode);
    QString signTests(QDomNode *tNode=0);
    void    exportSigned(QStringList xmlTests=QStringList());

    void    generateKeys();
    void    selectPrivKey();
//    void    setDefaultKey();
    void    selectPubKey();
    void    removePubKey();
    void    loadDialogSettings();

    QString signNodeText(QString tXml, QDomDocument *templateSig);
signals:
    void    testsLoaded(bool);
    void    privateLoaded(bool);
};

#endif // DNASIG_H
