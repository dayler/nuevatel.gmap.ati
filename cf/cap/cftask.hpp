/*
 * cftask.hpp
 */
#ifndef CFTASK_HPP
#define	CFTASK_HPP

#include "block.hpp"
#include "appconn/cfie.hpp"
#include "appconn/cfmessage.hpp"
#include "../../base/appconn/appclient.hpp"
#include "gmapapp.hpp"
#include "gmapdialog.hpp"

/**
 * <p>The TestSessionTask class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class TestSessionTask : public Task
{
    /** The dialogMap. */
    DialogMap *dialogMap;

public:

    /**
     * Creates a new instance of TestSessionTask.
     * @param *dialogMap DialogMap
     * @param *putBlockQueue PutBlockQueue
     */
    TestSessionTask(DialogMap *dialogMap, PutGBlockQueue *putBlockQueue)
    {
        this->dialogMap = dialogMap;
    }

    Message* execute(Conn *conn, Message *msg) throw (Exception)
    {
        // id
        CompositeIE *idIE = msg->getComposite(CFIE::ID_IE);
        if (idIE != NULL)
        {
            Id id(idIE);
            Dialog *dialog = dialogMap->get(id.getId1());
            if (dialog != NULL)
            {
                dialog->incrementInvokeId();
                dialog->setState(Dialog::W_INVOKE);
                return SetSessionRet(AppMessages::ACCEPTED).toMessage();
            }
        }
        return TestSessionRet(AppMessages::FAILED).toMessage();
    }
};

/**
 * Anytime interrogation call handler. Receive call from handler.
 */
class AnytimeInterrogationCall : public Task
{
private:
    DialogMap* dialogMap;
    PutGBlockQueue* putBlockQueue;

    // To create dialog
    int appId;
    int localId;
    int localPC;
    string localMSISDN;
    unsigned char localMSISDNType;
    int remotePC;
    string remoteMSISDN;
    unsigned char remoteMSISDNType;
    
    AppClient* appClient;
    Executor* dialogTaskService;
    
    MAP_Init* mapInit;
    
public:
    /**
     * name is MSISDN
     */
    static int TYPE_MSISDN;
    
    /**
     * name is IMSI
     */
    static int TYPE_IMSI;
    
    AnytimeInterrogationCall(DialogMap *dialogMap,
                             MAP_Init* mapInit,
                             const int& appId,
                             const int& localId,
                             
                             const int& localPC, // SMSC
                             string& localMSISDN, // SMSC
                             unsigned char& localMSISDNType, // SMSC type
                             int& remotePC, // SW PC
                             string& remoteMSISDN, // HLR 
                             unsigned char& remoteMSISDNType, // HLR type
                             
                             PutGBlockQueue *putBlockQueue,
                             AppClient *appClient,
                             Executor *dialogTaskService) : Task()
    {
        this->dialogMap = dialogMap;
        this->mapInit = mapInit;
        // To create dialogs
        this->appId = appId;
        this->localId = localId;
        this->localPC = localPC;
        this->localMSISDN = localMSISDN;
        this->localMSISDNType = localMSISDNType;
        this->remotePC = remotePC;
        this->remoteMSISDN = remoteMSISDN;
        this->remoteMSISDNType = remoteMSISDNType;
        // 
        this->putBlockQueue = putBlockQueue;
        this->appClient = appClient;
        this->dialogTaskService =  dialogTaskService;
    }
    
    virtual ~AnytimeInterrogationCall()
    {
        cout<<"~AnytimeInterrogationCall"<<endl;
        // no op
    }
    
    Message* execute(Conn* conn, Message* msg) throw (Exception)
    {
        // Get composite props
        CompositeIE* idIE = msg->getComposite(CFIE::ID_IE);
        if (idIE == NULL)
        {
            return AnytimeInterrogationRet(AppMessages::FAILED).toMessage();
        }
        
        Id* id = new Id(idIE);
        char type = msg->getByte(CFIE::TYPE_IE);
        string name = msg->getString(CFIE::SUBSCRIBER_NAME_IE);
        ATIDelegate delegate;
        ATIDialog* dialog = new ATIDialog(appId,
                                          localId,
                                          putBlockQueue,
                                          dialogTaskService,
                                          appClient,
                                          &delegate,
                                          // 
                                          id,
                                          name,
                                          type,
                                          mapInit->ssn,
                                          localPC,
                                          localMSISDN,
                                          localMSISDNType,
                                          remotePC,
                                          remoteMSISDN,
                                          remoteMSISDNType);
        // dispatch blocks
        dialog->init();
        const char* rawCellId = delegate.getCellId();
        
        if (std::strcmp("0000000000", rawCellId) == 0)
        {
            // no found cell id for unit
            return AnytimeInterrogationRet(AppMessages::FAILED).toMessage();
        }
        
        return AnytimeInterrogationRet(rawCellId).toMessage();
    }
};

int AnytimeInterrogationCall::TYPE_MSISDN = 0x1;
int AnytimeInterrogationCall::TYPE_IMSI = 0x2;

#endif	/* CFTASK_HPP */
