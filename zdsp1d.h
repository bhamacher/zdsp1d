// header datei zdsp1d.h
// globale definitionen


#ifndef ZDSP1D_H
#define ZDSP1D_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QHash>
#include <QVector>
#include <protonetserver.h>

#include "dsp1scpi.h"
#include "zhserver.h"
#include "dsp.h"
#include "zdspdprotobufwrapper.h"


typedef QVector<float> tDspMemArray;

class QByteArray;
class QStateMachine;
class QState;
class cZDSP1Server; // forward
class cDebugSettings;
class cETHSettings;
class cDSPSettings;
class cRMConnection;

class cZDSP1Client
{
public:
    cZDSP1Client(){}
    cZDSP1Client(int socket, ProtoNetPeer *netclient, cZDSP1Server *server);
    ~cZDSP1Client(){} //  allokierten speicher ggf. freigeben
    
    
    QString& SetRavList(QString&);
    QString& GetRavList();
    QString& SetCmdListDef(QString& );
    QString& GetCmdListDef();
    QString& SetCmdIntListDef(QString&);
    QString& GetCmdIntListDef();
    void SetEncryption(int);
    int GetEncryption();
    bool GenCmdLists(QString&); // baut die cmdlisten  für den dsp zusammen wenn fehler -> false 
    cDspCmd GenDspCmd(QString&,bool*); // generiert ein dsp kommando aus einem string
    bool InitiateActValues(QString&); // liess die messergebnisse (liste)
    bool isActive(); 
    QString& FetchActValues(QString&); // gibt die messergebnisse aus (liste)
    void SetActive(bool); // merkt sich in m_bActive ob diese liste aktiv ist
    void SetStartAdr(ulong); // zum relokalisieren der userdaten
    QString& DspVarListRead(QString&); // lesen dsp daten ganze Liste
    bool DspVar(QString&,int&); // einen int (32bit) wert lesen
    bool DspVar(QString&,float&); // eine float wert lesen
    sDspVar* DspVarRead(QString&,QByteArray*); // lesen dsp variable;  name , länge stehen im parameter string; werte im anschluss im qbytearray
    const char* DspVarWriteRM(QString&); // dito schreiben mit rückmeldung
    bool DspVarWrite(QString&);  // schreiben  true wenn ok
    QList<cDspCmd>& GetDspCmdList(); // damit der server die komplette liste aller clients
    QList<cDspCmd>& GetDspIntCmdList(); // an den dsp übertragen kann
    tDspMemArray& GetDspMemData(); 
    int getSocket();
    cDspVarResolver DspVarResolver; // zum auflösen der variablen aus kommandos
    int sock; // socket für den die verbindung besteht
    ProtoNetPeer* m_pNetClient; // our network client

private:
    void init(int socket, ProtoNetPeer *netclient, cZDSP1Server* server);
    cZDSP1Server* myServer; 
    bool m_bActive;
    bool GenCmdList(QString&, QList<cDspCmd>& ,QString&);
    bool syntaxCheck(QString&);
          
    int Encryption;
    char* qSEncryption(char*,int );
    QString sOutput;
    QString m_sCmdListDef; // kommando liste defintion
    QString m_sIntCmdListDef; // interrupt kommando  liste defintion
        
//    ulong m_nStartAdr; // die absolute adresse an der ein variablen "block" im dsp steht 
    int m_nlen; // länge des gesamten datenblocks (in float bzw. long)
    tDspMemArray m_fDspMemData; // der datenblock bzw. kopie desselben
    QList<cDspClientVar> m_DspVarList; // liste mit variablen beschreibungen
    QList<cDspCmd> m_DspCmdList; // liste mit dsp kommandos (periodisch)
    QList<cDspCmd>  m_DspIntCmdList; // liste mit dsp kommandos (interrupt)
    QVector<sDspVar> varArray; // array von sDspVar
    sMemSection msec; // eine memory section für den DspVarResolver für die variablen des clients

};

class cZDSP1Server: public QObject, public cZHServer, public cbIFace {
    Q_OBJECT
public:
    cZDSP1Server();
    virtual ~cZDSP1Server();
    virtual cZDSP1Client* AddClient(ProtoNetPeer *m_pNetClient); // fügt einen client hinzu
    virtual void DelClient(ProtoNetPeer *netClient); // entfernt einen client

    virtual QString SCPICmd( SCPICmdType, QChar*);
    virtual QString SCPIQuery( SCPICmdType);
    
    int m_nFPGAfd;
    void SetFASync(); // async. benachrichtung einschalten
    
    int SetBootPath(const char*);
    int SetDeviceNode(char*);
    int SetDebugLevel(const char*);
    
    int DspDevRead(int, char*, int); 
    int DspDevWrite(int, char*, int);
    int DspDevSeek(int,ulong);
    int DspDevOpen();
    
    void DspIntHandler();

    int DevFileDescriptor; // kerneltreiber wird nur 1x geöffnet und dann gehalten
    int m_nDebugLevel;
    
    cDebugSettings* m_pDebugSettings;
    cETHSettings* m_pETHSettings;
    cDSPSettings* m_pDspSettings;

signals:
    void serverSetup();
    void sendAnswer(QByteArray);
    void abortInit();

private:
    ProtoNetServer* myServer; // the real server that does the communication job
    cZDSPDProtobufWrapper m_ProtobufWrapper;
    quint16 m_nSocketIdentifier; // we will use this instead of real sockets, because protobuf extension clientId
    QHash<QByteArray, cZDSP1Client*> m_zdspdClientHash;
    QHash<cZDSP1Client*, QByteArray> m_clientIDHash; // liste der clientID's für die dspclients die über protobuf erzeugt wurden
    quint8 m_nerror;
    uchar ActivatedCmdList;
    QList<cZDSP1Client*> clientlist; // liste aller clients
    
    bool resetDsp();
    bool bootDsp();
    bool setSamplingSystem();

    // die routinen für das system modell
    QString mCommand2Dsp(QString&); // indirekt für system modell
    
    QString mTestDsp(QChar *);
    QString mResetDsp(QChar*);
    QString mBootDsp(QChar*);
    QString mSetDspBootPath(QChar*);
    QString mGetDspBootPath();
    QString mGetPCBSerialNumber();
    QString mGetDeviceVersion();
    QString mGetServerVersion();
    QString mSetSamplingSystem(QChar*);
    QString mGetSamplingSystem();
    QString mSetCommEncryption(QChar*);
    QString mGetCommEncryption();
    QString mSetEN61850DestAdr(QChar*);
    QString mGetEN61850DestAdr();
    QString mSetEN61850SourceAdr(QChar*);
    QString mGetEN61850SourceAdr();
    QString mSetEN61850EthTypeAppId(QChar*);
    QString mGetEN61850EthTypeAppId();
    QString mSetEN61850PriorityTagged(QChar*);
    QString mGetEN61850PriorityTagged();
    QString mSetEN61850EthSync(QChar*);
    QString mGetEN61850EthSync();
    QString mSetDspCommandStat(QChar*);
    QString mGetDspCommandStat();
    QString mSetEN61850DataCount(QChar*);
    QString mGetEN61850DataCount();
    QString mSetEN61850SyncLostCount(QChar*);
    QString mGetEN61850SyncLostCount();
    QString mTriggerIntListHKSK(QChar*);
    QString mTriggerIntListALL(QChar*);
    QString mResetMaxima(QChar*);
        
    // die routinen für das status modell
    
    QString mResetDeviceLoadMax();
    QString mGetDeviceLoadMax();
    QString mGetDeviceLoadAct();
    QString mGetDspStatus();
    QString mGetDeviceStatus();
  
    // die routinen für das measure modell
    
    QString mFetch(QChar *);
    QString mInitiate(QChar*);
    QString mUnloadCmdList(QChar*);
    QString mLoadCmdList(QChar*);
    QString mSetRavList(QChar*);
    QString mGetRavList();
    QString mSetCmdIntList(QChar*);
    QString mGetCmdIntList();
    QString mSetCmdList(QChar*);
    QString mGetCmdList();
    QString mMeasure(QChar*);
    
    // die routinen für das memory modell
    
    QString mDspMemoryRead(QChar *);
    QString mDspMemoryWrite(QChar *);
    
    bool LoadDSProgram();
    bool setDspType();
    int readMagicId();
    bool Test4HWPresent(); 
    bool Test4DspRunning();
    cZDSP1Client* GetClient(int s);
    cZDSP1Client* GetClient(ProtoNetPeer* peer);
    QString m_sDspDeviceVersion; // version der hardware
    QString m_sDspSerialNumber; // seriennummer der hardware
    QString m_sDspDeviceNode; // für den zugriff zur hardware
    QString m_sDspBootPath;
    Zera::XMLConfig::cReader* myXMLConfigReader;

    QString Answer;

    QStateMachine* m_pInitializationMachine;
    QState* stateconnect2RM;
    QState* stateSendRMIdentandRegister;
    cRMConnection* m_pRMConnection;


private slots:
    virtual void establishNewConnection(ProtoNetPeer* newClient);
    virtual void deleteConnection();
    virtual void executeCommand(google::protobuf::Message* cmd);

    void doConfiguration();
    void doSetupServer();
    void doCloseServer();
    void doConnect2RM();
    void doIdentAndRegister();
};

#endif
