//
//  ?????????? ?????? DnaSig
//
////////////////////////////////////////////////
/*

1. ????????? ??????? ??????
	.. ????? ????????????? ??? ? ?????? ?????????? ??????? ???????????? 
????? dnasig.exe, ??? ? ??? ?????? COM-?????????? ?? ?????? ??????????. 
?????? ????. ? ?????????? ????????? ?????? ???? ??????? ???????? ?????, ?? 
??????? ???????????? ???????? ??????????? ??????????? ??????? ???????????? 
??????.

2. ???????????? ??????
	???????????? ?????? ?????????????? ??? ???????????????? ??????? 
???????????? ????? dnasig.exe. ? ?????????? ????????? ?????? ???? ?????? 
????????? ???? ???????????, ??????? ????? ????????????? ?????. ????? ????,
??? ????????? ???? ????????, ? ???? ???????? ????? ????? ?????????? ????????
??????????? ???????, ?? ??????? ????? ? ?????????? ?????????? ???????????
???????????? ????. ??????? ????????????? ?????? ? ??????????? ??? ????????????:
	- ??????? ????(?) ??????;
	- ? ?????? ???????? ????? ?????? ? ?????, ???????????? ??????? ???????;
	- ????????? ???????????? (Sign), ????? ?????? ??????????;
	- ????????? ?????? ??????????? ?????? (Save). 
	C?????????? ? ???? ?????? exportTests.xml.
*/



//  Register in system
//
// dnasig.exe -regserver
//
////////////////////////////////////////////////


	if ( FAILED( CoInitialize( NULL ))){
	
      		qDebug() << "Unable to initialize COM";
		return -1;
	}

//....

	QAxObject axgp;

	// class dnasig
	//
	axgp.setControl( "{6DCA6D52-6D57-44ae-AFDC-2C4332680093}" );   


	if (axgp.control().isNull()){

        	qDebug() << "Unable to set COM control";
	        return -2;
	}


	// Check keys
	//
	qDebug() << axgp.dynamicCall( "publicKeys()");
	qDebug() << axgp.dynamicCall( "privateKeys()");


	QString xmlItem = "<item>........</item>";

	bool isValid = axgp.dynamicCall( "loadXmlTest(QString)", 
						xmlItem ).toBool();


//....
	CoUninitialize();
