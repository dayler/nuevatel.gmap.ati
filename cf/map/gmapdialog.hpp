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

/**
 * Delegate, used to return cell id.
 */
class ATIDelegate
{
private:
    boost::mutex mx;
    boost::condition_variable sync;
    
    string cellId;
    
public:
    static char* NULL_CELL_ID;
    
    ATIDelegate()
    {
        // No op
    }
    
    ~ATIDelegate()
    {
        // No op
    }
    
    void setCellId(string& cellId)
    {
        boost::mutex::scoped_lock lck(mx);
        this->cellId.assign(cellId.c_str());
        sync.notify_all();
    }
    
    const char* getCellId()
    {
        boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(1000);
        boost::mutex::scoped_lock lck(mx);
        while (this->cellId.length() == 0)
        {
            if (!sync.timed_wait(lck, timeout))
            {
                return NULL_CELL_ID;
            }
        }
        
        char* data = new char[this->cellId.size() + 1];
        std::copy(this->cellId.begin(), this->cellId.end(), data);
        data[this->cellId.size()] = '\0';
        return data;
    }
};

char* ATIDelegate::NULL_CELL_ID = "0000000000";

/**
 * Dialog to make AnytimeInterrogation Map call. REF (GPP TS 09.02 version 7.15.0 Release 1998)
 */
class ATIDialog : public Dialog
{
private:
    
    int appId;
    int localId;
    
    PutGBlockQueue* putGBlockQueue;
    Executor* dialogTaskService;
    
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
    
    string strCellId;
    
    ATIDelegate* delegate;
    
public:
    static U8 REMOTE_GT_SSN;
    
    ATIDialog(const int& appId,
              const int&localId,
              PutGBlockQueue* putGBlockQueue,
              Executor* dialogTaskService,
              AppClient* appClient,
              ATIDelegate* delegate,
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
        this->delegate = delegate;
        //
        this->idIE = idIE;
        // Set tmp dialog ID, is is used in case the dialog is killed.
        setDialogId(this->idIE->getId1());
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
    }
    
    ~ATIDialog()
    {
        if (idIE != NULL)
        {
            delete idIE;
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
//        cout<<"--------------------------"<<endl;
//        gMAPPrintGBlock(gb);
//        cout<<"--------------------------"<<endl;
        if (gb->serviceType == GMAP_RSP)
        {
            if (gb->serviceMsg == GMAP_OPEN)
            {
                setState(W_INVOKE);
            }
            else if (gb->serviceMsg == ANY_TIME_INTERROGATION)
            {
                CellGlobalId tmpCellGlobalId((char*)gb->parameter.anyTimeInterrogationRes_v3.subscriberInfo.locationInformation.cellGlobalIdOrServiceAreaIdOrLAI.u.cellGlobalIdOrServiceAreaIdFixedLength.value,
                                             gb->parameter.anyTimeInterrogationRes_v3.subscriberInfo.locationInformation.cellGlobalIdOrServiceAreaIdOrLAI.u.cellGlobalIdOrServiceAreaIdFixedLength.length);
                stringstream msg;
                msg <<"ATI Ret CellId: "<<tmpCellGlobalId.getCellGlobalId().c_str();
                Logger::getLogger()->getLogger()->logp(&Level::INFO, "ATIDialog", "handle", msg.str());
                string tmpStr = tmpCellGlobalId.getCellGlobalId();
                delegate->setCellId(tmpStr);
                setState(INVOKE);
                // submit to task service
                dialogTaskService->submit(new DialogTask(this));
            }
            else if (gb->serviceMsg == UNKNOWN_SUBSCRIBER)
            {
                // Unknown subscriber, malformed IMSI, malformed MSISDN
                string tmpStr = ATIDelegate::NULL_CELL_ID;
                delegate->setCellId(tmpStr);
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
        // No actions it is ready to close dialog.
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
};

// Default ssn for remote GT
U8 ATIDialog::REMOTE_GT_SSN = 6;

#endif	/* CAPDIALOG_HPP */
