/*
 * capdialog.hpp
 */
#ifndef CAPDIALOG_HPP
#define	CAPDIALOG_HPP

#include "block.hpp"
#include "appconn/cfie.hpp"
#include "appconn/cfmessage.hpp"
#include "../../base/appconn/appclient.hpp"
#include "gmapapp.hpp"

class ATIDialog : public Dialog
{
private:
    int appId;
    int localId;
    PutGBlockQueue* putGBlockQueue;
    Executor* dialogTaskService;
    Queue<CFMessage>cfMessageQueue;
    
    unsigned char localSSN;
    unsigned int localPC;
    string localMSISDN;
    unsigned char localMSISDNType;
    unsigned int remotePC;
    string remoteMSISDN;
    unsigned char remoteMSISDNType;
    
    string name;
    char type;
    
public:
    U8 REMOTE_SSN=6;
    
    ATIDialog(const int& appId,
              const int&localId,
              PutGBlockQueue* putGBlockQueue,
              Executor* dialogTaskService,
              // 
              string& name,
              char& type,
              unsigned char& localSSN,
              unsigned int& localPC,
              string& localMSISDN,
              unsigned char& localMSISDNType,
              unsigned int& remotePC,
              string& remoteMSISDN,
              unsigned char& remoteMSISDNType)
    {
        this->appId = appId; 
        this->localId = localId;
        this->putGBlockQueue = putGBlockQueue;
        this->dialogTaskService = dialogTaskService;
        //
        this->name = name;
        this->type = type;
        this->localSSN = localSSN;
        this->localPC = localPC;
        this->localMSISDN = localMSISDN;
        this->localMSISDNType = localMSISDNType;
        this->remotePC = remotePC;
        this->remoteMSISDN = remoteMSISDN;
        this->remoteMSISDNType = remoteMSISDNType;
    }
    
    ~ATIDialog()
    {
        // No op
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
                                              REMOTE_SSN, // Remote SSN = 6
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
                Address addr((char*)gb->parameter.anyTimeInterrogationRes_v3.subscriberInfo.locationInformation.cellGlobalIdOrServiceAreaIdOrLAI.u.cellGlobalIdOrServiceAreaIdFixedLength.value,
                             gb->parameter.anyTimeInterrogationRes_v3.subscriberInfo.locationInformation.cellGlobalIdOrServiceAreaIdOrLAI.u.cellGlobalIdOrServiceAreaIdFixedLength.length);
                setState(W_CLOSE_0);
                // schedule to dispatch cfMessage
                
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
        //
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

#endif	/* CAPDIALOG_HPP */
