/*
 * capdialog.hpp
 */
#ifndef CAPDIALOG_HPP
#define	CAPDIALOG_HPP

#include <boost/thread.hpp>

#include "block.hpp"
#include "appconn/cfie.hpp"
#include "appconn/cfmessage.hpp"
#include "../../base/appconn/appclient.hpp"
#include "gmapapp.hpp"

class ATIDialog : public Dialog
{
private:
    boost::mutex mx;
    boost::condition_variable sync;
    
    int appId;
    int localId;
    
    PutGBlockQueue* putGBlockQueue;
    Executor* dialogTaskService;
    Queue<CFMessage>cfMessageQueue;
    
    /**
     * To dispatch the response message.
     */
    Id* idIE;
    
    /**
     * Name (msisdn / imsi) of the resource that is looking for.
     */
    string name;
    
    /**
     * Identifies the name type. 0x1 = MSISDN, 0x2 = IMSI
     */
    char type;
    
    unsigned char localSSN;
    
    /**
     * SMSC or CF point code
     */
    unsigned int localPC;
    
    /**
     * MSISDN of SMSC or CF
     */
    string localMSISDN;
    
    /**
     * MSISDN of SMSC or CF type
     */
    unsigned char localMSISDNType;
    
    /**
     * HLR point code
     */
    unsigned int remotePC;
    
    /**
     * HLR MSISDN address
     */
    string remoteMSISDN;
    
    /**
     * HLR MSISDN type
     */
    unsigned char remoteMSISDNType;
    
    /**
     * App client ref
     */
    AppClient* appClient;
    
    string* strCellId;
    
public:
    static U8 REMOTE_GT_SSN;
    
    ATIDialog(const int& appId,
              const int&localId,
              PutGBlockQueue* putGBlockQueue,
              Executor* dialogTaskService,
              AppClient* appClient,
              // 
              Id* idIE,
              string& name,
              char& type,
              unsigned char& localSSN,
              const int& localPC,
              string& localMSISDN,
              unsigned char& localMSISDNType,
              const int& remotePC,
              string& remoteMSISDN,
              unsigned char& remoteMSISDNType)
    {
        this->appId = appId; 
        this->localId = localId;
        this->putGBlockQueue = putGBlockQueue;
        this->dialogTaskService = dialogTaskService;
        this->appClient = appClient;
        //
        this->idIE = idIE;
        this->name = name;
        this->type = type;
        this->localSSN = localSSN;
        this->localPC = localPC;
        this->localMSISDN = localMSISDN;
        this->localMSISDNType = localMSISDNType;
        this->remotePC = remotePC;
        this->remoteMSISDN = remoteMSISDN;
        this->remoteMSISDNType = remoteMSISDNType;
        // 
        strCellId = new string();
    }
    
    ~ATIDialog()
    {
        cout<<"call ati destroy..."<<endl;
        if (idIE != NULL)
        {
            delete idIE;
        }
        
        if (strCellId != NULL)
        {
            delete strCellId;
        }
    }
    
    void init()
    {
        setState(W_INVOKE);
        // Open
        putGBlockQueue->push(new OpenReqBlock(this,
                                              &anyTimeInfoEnquiryContext_v3,
                                              localSSN,
                                              localPC,
                                              localMSISDN, // Local GT / Service Centre Addr
                                              localMSISDNType, // Local GT / Service Centre Addr Type
                                              REMOTE_GT_SSN, // Remote SSN = 6
                                              remotePC,
                                              remoteMSISDN, // Remote GT / HLR
                                              remoteMSISDNType)); // Remote GT / HLR Type
        // Send ATI
        putGBlockQueue->push(new AnytimeInterrogationReqBlock(this,
                                                              name,
                                                              type,
                                                              localMSISDN, // Local GT / Service Centre Addr
                                                              localMSISDNType)); // Local GT / Service Centre Addr Type)
        // Delimiter
        putGBlockQueue->push(new DelimiterReqBlock(this));
    }
    
    void handle(gblock_t* gb)
    {
        if (gb->serviceType == GMAP_RSP)
        {
            if (gb->serviceMsg == GMAP_OPEN)
            {
                setState(W_INVOKE);
            }
            else if (gb->serviceMsg == ANY_TIME_INTERROGATION) // ANY_TIME_INTERROGATION
            {
//                CellGlobalId tmpCellGlobalId((char*)gb->parameter.anyTimeInterrogationRes_v3.subscriberInfo.locationInformation.cellGlobalIdOrServiceAreaIdOrLAI.u.cellGlobalIdOrServiceAreaIdFixedLength.value,
//                                             gb->parameter.anyTimeInterrogationRes_v3.subscriberInfo.locationInformation.cellGlobalIdOrServiceAreaIdOrLAI.u.cellGlobalIdOrServiceAreaIdFixedLength.length);
//                
//                cout<<">>>>> "<<tmpCellGlobalId.getCellGlobalId().c_str()<<endl;
                string str ("234");
                setCellId(str);
                //cfMessageQueue.push(new AnytimeInterrogationRet(idIE, tmpCellID));
                // Set state
                setState(INVOKE);
                // submit to task service
                dialogTaskService->submit(new DialogTask(this));
            }
        }
        else if (gb->serviceType == GMAP_REQ)
        {
            if (gb->serviceMsg == GMAP_CLOSE)
            {
                setState(CLOSE_0);
                dialogTaskService->submit(new DialogTask(this));
            }
            else if (gb->serviceMsg == GMAP_U_ABORT || gb->serviceMsg == GMAP_P_ABORT)
            {
                setState(ABORT_0);
                dialogTaskService->submit(new DialogTask(this));
            }
        }
        else if (gb->serviceType == GMAP_ERROR)
        {
            setState(CLOSE_0);
            dialogTaskService->submit(new DialogTask(this));
        }
    }
    
    void run()
    {
//        CFMessage* cfMessage = cfMessageQueue.waitAndPop();
//        Message* msg = cfMessage->toMessage();
//        Message* ret = NULL;
//        
//        try
//        {
//            ret = appClient->dispatch(msg);
//        }
//        catch(Exception ex)
//        {
//            Logger::getLogger()->logp(&Level::WARNING, "GMAP Dialog", "run", ex.toString());
//        }
        
        // Dialog ready to close
        setState(W_CLOSE_1);
    }
    
    void check()
    {
        if (getState() == CLOSE_0)
        {
            setState(CLOSE_1);
        }
        else if (getState() == W_CLOSE_0)
        {
            setState(W_CLOSE_1);
        }
        else if (getState() == W_CLOSE_1)
        {
            putGBlockQueue->push(new CloseReqBlock(this));
        }
        else if (getState() == ABORT_0)
        {
            setState(ABORT_1);
        }
        else if (getState() == KILL_0)
        {
            putGBlockQueue->push(new CloseReqBlock(this));
        }
    }
    
        /**
     * Compares two objectIDs.
     * @param *id1 ObjectID
     * @param *id2 ObjectID
     * @return bool
     */
    bool compareObjectIDs(ObjectID *id1, ObjectID *id2)
    {
        if (id1->count != id2->count)
        {
            return false;
        }

        if (memcmp(id1->value, id2->value, id1->count * sizeof (unsigned int)) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    void setCellId(string& cellId)
    {
        cout<<"[setCellId]"<<endl;
        
        boost::mutex::scoped_lock lck(mx);
        //strCellId->assign(cellId->c_str());
        strCellId->assign(cellId.c_str());
        sync.notify_all();
        
    }
    
    /**
     * Get and wait cell id, 
     * 
     * @param cellId cell id reference.
     */
    const char* getCellId()
    {
        cout<<"[1]"<<endl;
        boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(30000);
        boost::mutex::scoped_lock lck(mx);
        
        while (strCellId->length() == 0)
        {
            if (!sync.timed_wait(lck, timeout))
            {
                return NULL;
            }
        }

        cout<<"[2] "<<endl;
        return "666";
    }
};

// Default ssn for remote GT
U8 ATIDialog::REMOTE_GT_SSN = 6;

#endif	/* CAPDIALOG_HPP */
