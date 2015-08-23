/*
 * capapp.hpp
 */
#ifndef CAPAPP_HPP
#define	CAPAPP_HPP

#include "cftask.hpp"
#include "../../base/logger.hpp"

/********************************
*	UNIX system files	*
********************************/
#include <apiinc.h>
#include <stdarg.h>
#include <time.h>
#include <poll.h>

/********************************
*      SIGNALWARE INCLUDES      *
********************************/
#include <tapsc.h> /* contains struct. definitions for tap */
#include <tcap.h>  /* contains struct. definitions for tcap*/

/********************************
*        GSM Map files          *
********************************/
#include <gmap.h>

using namespace std;

/** The appState. */
static char appState=0;

static char OFFLINE=0;
static char ONLINE=1;
/**
 * The sigintHandler.
 */
static void sigintHandler()
{
    if (appState == ONLINE)
    {
        appState = OFFLINE;
    }
    else
    {
        exit(0);
    }
}

/** The appMutex */
boost::mutex appMutex;
/**
 * <p>The PutBlockService class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class PutGBlockService : public PutGBlockQueue
{
private:
    /** The dialogMap. */
    DialogMap *dialogMap;
    
    /**
     * Gblock instance, unique instance is used through app. The reason is the structure is too long (~10M)
     */
    gblock_t gb;
    
public:
    PutGBlockService(DialogMap *dialogMap)
    {
        this->dialogMap = dialogMap;
    }

private:
    void run()
    {
        try
        {
            // FtThreadRegister
            if (FtThreadRegister() == RETURNok)
            {
                while (appState == ONLINE)
                {
                    GBlock *gblock = blockQueue.waitAndPop();
                    gblock->getGBlock(&gb);
                    // gMAPPrintGBlock(&gb);
                    Dialog *dialog;
                    dialog = gblock->getDialog();
                    
                    if (dialog->getState() != Dialog::CLOSE_0 && dialog->getState() != Dialog::ABORT_0)
                    {
                        int mapPutBlockRet = -1;
                        {
                            boost::lock_guard<boost::mutex> lock(appMutex);
                            // Dispatch gblock_t to platform
                            mapPutBlockRet = gMAPPutGBlock(&gb);
                        }
                        
                        if (mapPutBlockRet == 0)
                        {
                            if (gb.serviceType == GMAP_REQ)
                            {
                                if (gb.serviceMsg == GMAP_OPEN)
                                {
                                    dialog->setDialogId(gb.dialogId);
                                    dialogMap->put(dialog);
                                }
                                else if (gb.serviceMsg == GMAP_CLOSE)
                                {
                                    if (dialog->getState() == Dialog::KILL_0)
                                    {
                                        dialog->setState(Dialog::KILL_1);
                                        std::stringstream ss;
                                        ss << "dialog killed ";
                                        ss << std::hex << dialog->getDialogId();
                                        Logger::getLogger()->logp(&Level::WARNING, "PutGBlockService", "run", ss.str());
                                    }
                                    else
                                    {
                                        dialog->setState(Dialog::CLOSE_0);
                                    }
                                }
                            }
                        }
                        else
                        {
                            std::stringstream ss;
                            ss << "mapPutBlock failed " << gb.serviceMsg << " ";
                            ss << std::hex << dialog->getDialogId();
                            Logger::getLogger()->logp(&Level::SEVERE, "PutBlockService", "run", ss.str());
                            dialog->setState(Dialog::ABORT_0);
                        }
                    }
                    // Delete pointer gblock
                    delete gblock;
                }
            }
            else
            {
                std::stringstream ss;
                ss << "FtThreadRegister() failed, errno=" << errno;
                throw Exception(ss.str(), __FILE__, __LINE__);
            }
        }
        catch (Exception e)
        {
            Logger::getLogger()->logp(&Level::SEVERE, "PutBlockService", "run", e.toString());
        }
        // FtThreadUnregister
        FtThreadUnregister();
    }
};
/**
 * <p>The CAPApp singleton class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class GMAPApp : public Thread
{
public:

    /* constants for properties */
    static string LOCAL_ID;
    static string REMOTE_ID;
    static string LOGICAL_NAME;
    static string N_DIALOGS;
    static string N_INVOKES;
    static string N_COM_BUFS;
    static string NODE_NAME;
    static string STAND_ALONE;
    
    static string LOCAL_PC;
    static string LOCAL_GT;
    static string LOCAL_GT_TYPE;
    static string LOCAL_SSN;
    
    static string REMOTE_PC;
    static string REMOTE_GT;
    static string REMOTE_GT_TYPE;
    static string REMOTE_SSN;
    
private:

    /** The argc. */
    int argc;

    /** The argv. */
    char** argv;

    /** The appId. */
    int appId;

    /** The localId. */
    int localId;

    /** The remoteId. */
    int remoteId;
    
    int localPC;
    string localGT;
    unsigned char localGTType;
    
    int remotePC;
    string remoteGT;
    unsigned char remoteGTType;
    int remoteSSN;
    
    /**
     * gmap initialization structure
     */
    MAP_Init gMAPInit;

    /* properties */
    string logicalName;
    bool standAlone;

    /** The dialogMap. */
    DialogMap* dialogMap;

    /** The putBlockService. */
    PutGBlockService* putBlockService;

    /** The taskSet*/
    TaskSet* taskSet;
    TestSessionTask* testSessionTask;
    AnytimeInterrogationCall* atiTask;

    /** The appClient. */
    AppClient* appClient;

    /** The dialogTaskService. */
    Executor *dialogTaskService;

    /* gmap block */
    gblock_t gb;

public:

    /**
     * Creates a new instance of CAPApp.
     * @param &argc const int
     * @param **argv char
     * @param *properties Properties
     * @throws Exception
     */
    GMAPApp(const int &argc, char** argv, Properties *properties) throw (Exception)
    {
        this->argc = argc;
        this->argv = argv;
        setProperties(properties);
        // dialogMap
        dialogMap = new DialogMap();
        // putBlockService
        putBlockService = new PutGBlockService(dialogMap);
        // dialogTaskService
        dialogTaskService = new Executor();
        // taskSet
        taskSet = new TaskSet();
        
        // appClient
        appClient = new AppClient(localId, remoteId, taskSet, properties);
        
        testSessionTask = new TestSessionTask(dialogMap, putBlockService);
        atiTask = new AnytimeInterrogationCall(dialogMap,
                                               &gMAPInit,
                                               appId,
                                               localId,
                                               localPC,
                                               localGT,
                                               localGTType,
                                               remotePC,
                                               remoteGT,
                                               remoteGTType,
                                               putBlockService,
                                               appClient,
                                               dialogTaskService);
        // Populate appconn tasks
        taskSet->add(CFMessage::TEST_SESSION_CALL, testSessionTask);
        taskSet->add(CFMessage::ANYTIME_INTERROGATION_CALL, atiTask);
    }

    ~GMAPApp()
    {
        interrupt();
        delete dialogTaskService;
        delete appClient;
        // delete tasks
        delete testSessionTask;
        delete atiTask;
        delete taskSet;
        // 
        delete putBlockService;
        delete dialogMap;
    }

    /**
     * Starts this.
     */
    void start()
    {
        try
        {
            gMAPInit.protocol = itu7;
            gMAPInit.debugFile = stdout;

            if (standAlone)
            {
                // FtAttach
                if (FtAttach(logicalName.c_str(), // process logical name
                             argv[0], // process executable name
                             " ", // execution parameters
                             0, // execution priority
                             0, // RT time quantum
                             0, // RT time quantum
                             0, // process class identifier
                             10) == RETURNerror)
                {
                    // max. wait for CPT entry
                    stringstream ss;
                    ss << "FTAttach() failed, errno=" << errno + "(" << LastErrorReport << ")";
                    throw Exception(ss.str(), __FILE__, __LINE__);
                }
            }

            // FtRegister
            if (FtRegister(argc, // command Line Argument count
                           argv, // command Line Arguments
                           FALSE, // Debug Printouts Required ?
                           FALSE, // msg Activity Monitor Required ?
                           TRUE, // Ipc Queue Required ?
                           TRUE, // flush Ipc Queue Before Start ?
                           FALSE, // allow Ipc Msg Queueing Always
                           TRUE, // process Has SIGINT Handler
                           (U16) AUTOrestart, // automatic Restart allowed ?
                           0, // process Class Designation
                           0, // initial Process State Declaration
                           0, // event Distribution Filter Value
                           10) == RETURNerror)
            {
                // retry
                std::stringstream ss;
                ss << "FtRegister() failed, errno=" << errno + "(" << LastErrorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // FtAssignHandler. Set the  app to online state.
            if (FtAssignHandler(SIGINT, sigintHandler) == RETURNerror)
            {
                std::stringstream ss;
                ss << "cannot assign SIGINT handler, errno=" << errno;
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // SYSattach
            cout<<"["<<gMAPInit.nodeName<<"]"<<endl;
            int gAliasNameIndex = SYSattach(gMAPInit.nodeName, FALSE);
            if (gAliasNameIndex == RETURNerror)
            {
                std::stringstream ss;
                ss << "SYSattach() failed, errno=" << errno << "(" << LastErrorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // SYSbind
            if (SYSbind(gAliasNameIndex,
                        FALSE, // non-designatable
                        MTP_SCCP_TCAP_USER,
                        gMAPInit.ssn,
                        SCCP_TCAP_CLASS) != RETURNok)
            {
                std::stringstream ss;
                ss << "SYSbind() failed, errno=" << errno << "(" << LastErrorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // gMAPInitialize
            appId = gMAPInitialize(&gMAPInit, argc, argv);
            if (appId == RETURNerror)
            {
                std::stringstream ss;
                ss << "gMAPInitialize() failed, errno=" << errno + "(" << gMAPInit.errorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }
            else
            {
                std::stringstream ss;
                ss << "gMAPInitialize(), appId=" << appId;
                Logger::getLogger()->logp(&Level::INFO, "gMAPApp", "start", ss.str());
            }

            // CscUIS
            CscUIS(gAliasNameIndex, gMAPInit.ssn);

            Thread::start();

            // ONLINE
            appState = ONLINE;

            appClient->start();
            putBlockService->start();
        }
        catch (Exception e)
        {
            Logger::getLogger()->logp(&Level::SEVERE, "CAPApp", "start", e.toString());
            interrupt();
        }
    }

    /**
     * Interrupts this.
     */
    void interrupt()
    {
        appState = OFFLINE;
        putBlockService->join();
        appClient->interrupt();
        FtTerminate(NOrestart, 1);
    }

private:

    void run()
    {
        try
        {
            // FtThreadRegister
            if (FtThreadRegister() == RETURNok)
            {
                union
                {
                    Header_t hdr;
                    cblock_t cblock;
                    char ipc[MAXipcBUFFERsize];
                } ipc;

                while (appState == ONLINE)
                {
                    if (FtGetIpcEx(&ipc.hdr,
                                   0, // any message type
                                   sizeof (ipc), // max. size to rcv
                                   TRUE, // truncate if large
                                   TRUE, // blocking read
                                   TRUE) == RETURNerror)
                    {
                        // interruptible
                        if (errno != EINTR)
                        { // not interrupt
                            std::stringstream ss;
                            ss << "FtGetIpcEx() failed, errno=" << errno;
                            Logger::getLogger()->logp(&Level::SEVERE, "CAPApp", "run", ss.str());
                        }
                    }
                    else
                    {
                        // No errors, read message
                        switch (ipc.hdr.messageType)
                        {
                            case N_NOTICE_IND:
                            case N_UNITDATA_IND:
                            {
                                boost::lock_guard<boost::mutex> lock(appMutex);
                                // received a TCAP message 
                                // give MSU to library for state/event and parsing,
                                // we will retrieve GBlocks from library later 
                                gMAPTakeMsg(&ipc.cblock);
                            }
                                break;
                            case N_STATE_IND:
                            {
                                // TODO processNState
                                stringstream ss;
                                scmg_nstate_t *nstate;
                                nstate = &((iblock_t *) & ipc.hdr)->primitives.nstate;
                                ss << "N_STATE_IND PC=" << nstate->NS_affect_pc << " SSN=" << (int) nstate->NS_affect_ssn << " ";

                                if (nstate->NS_user_status == SCMG_UIS)
                                {
                                    ss << "UIS";
                                }
                                else if (nstate->NS_user_status == SCMG_UOS)
                                {
                                    ss << "UOS";
                                }

                                Logger::getLogger()->logp(&Level::WARNING, "CAPApp", "run", ss.str());
                            }
                                break;
                            case N_PCSTATE_IND:
                            {
                                // TODO processPCState
                                std::stringstream ss;
                                scmg_pcstate_t *pcstate;
                                pcstate = &((iblock_t *) & ipc.hdr)->primitives.pcstate;
                                ss << "N_PCSTATE_IND PC=" << pcstate->pc_pc << " ";

                                if (pcstate->pc_status == SCMG_INACCESSABLE)
                                {
                                    ss << "INACCESSABLE";
                                }
                                else if (pcstate->pc_status == SCMG_ACCESSABLE)
                                {
                                    ss << "ACCESSABLE";
                                }

                                Logger::getLogger()->logp(&Level::WARNING, "CAPApp", "run", ss.str());
                            }
                                break;
                            case TAP_STATE_CHANGE:
                                Logger::getLogger()->logp(&Level::INFO, "CAPApp", "run", "TAP_STATE_CHANGE received");
                                break;
                            default:
                            {
                                std::stringstream ss;
                                ss << "unknown ipc messageType received " << ipc.hdr.messageType;
                                Logger::getLogger()->logp(&Level::WARNING, "CAPApp", "run", ss.str());
                            }
                                break;
                        }
                    }
                    {
                        boost::lock_guard<boost::mutex> lock(appMutex);
                        while (gMAPGetGBlock(&gb) == RETURNok)
                        {
                            handle(&gb);
                        }
                    }
                }
            }
            else
            {
                std::stringstream ss;
                ss << "FtThreadRegister() failed, errno=" << errno;
                throw Exception(ss.str(), __FILE__, __LINE__);
            }
        }
        catch (Exception e)
        {
            Logger::getLogger()->logp(&Level::SEVERE, "CAPApp", "run", e.toString());
        }
        interrupt();
        // FtThreadUnregister
        FtThreadUnregister();
    }

    /**
     * Handle received messages in the platform.
     * 
     * @param p_gb Gblock pointer
     */
    void handle(gblock_t* gb)
    {
        if (gb->serviceType == GMAP_RSP ||
            gb->serviceType == GMAP_ERROR ||
            gb->serviceType == GMAP_PROVIDER_ERROR)
        {
            Dialog* dialog = dialogMap->get(gb->dialogId);
            if (dialog != NULL)
            {
                dialog->handle(gb);
            }
        }
        else if (gb->serviceType == GMAP_REQ)
        {
            if (gb->serviceMsg == GMAP_OPEN)
            {
                // TODO ForwardMOSM
            }
            else
            {
                // Any other dialog
                Dialog* dialog = dialogMap->get(gb->dialogId);
                if (dialog != NULL)
                {
                    dialog->handle(gb);
                }
            }
        }
    }

//        void handleGBlock(gblock_t *gb) {
//        //gMAPPrintGBlock(gb);
//        if(gb->serviceType==GMAP_RSP || gb->serviceType==GMAP_ERROR || gb->serviceType==GMAP_PROVIDER_ERROR) {
//            Dialog *dialog=dialogCache->get(gb->dialogId);
//            if(dialog!=NULL) dialog->handleGBlock(gb);
//        }
//        else if(gb->serviceType==GMAP_REQ) {
//            if(gb->serviceMsg==GMAP_OPEN) {
//                ForwardMOSM *dialog=new ForwardMOSM(this, appClient);
//                dialog->setDialogId(gb->dialogId);
//                dialog->init();
//                dialogCache->put(dialog);
//                dialog->handleGBlock(gb);
//            }
//            else if(gb->serviceMsg==MO_FORWARD_SM || gb->serviceMsg==ALERT_SERVICE_CENTRE_WITHOUT_RESULT || gb->serviceMsg==GMAP_DELIMITER || gb->serviceMsg==GMAP_CLOSE || gb->serviceMsg==GMAP_P_ABORT || gb->serviceMsg==GMAP_U_ABORT) {
//                Dialog *dialog=dialogCache->get(gb->dialogId);
//                if(dialog!=NULL) dialog->handleGBlock(gb);
//            }
//        }
//    }

    /**
     * Handles the capBlock.
     * @param *capBlock CapBlock
     */
//    void handle(CapBlock *capBlock)
//    {
//        //capPrintBlock(capBlock);
//        if (capBlock->serviceType == capRsp || capBlock->serviceType == capError || capBlock->serviceType == capProviderError)
//        {
//            Dialog *dialog = dialogMap->get(capBlock->dialogId);
//            if (dialog != NULL) dialog->handle(capBlock);
//        }
//        else if (capBlock->serviceType == capReq)
//        {
//            if (capBlock->serviceMsg == CAP_OPEN)
//            {
//                CAPDialog *dialog = new CAPDialog(appId, localId, putBlockService, appClient, dialogTaskService);
//                dialog->setDialogId(capBlock->dialogId);
//                dialog->init();
//                dialogMap->put(dialog);
//                dialog->handle(capBlock);
//            }
//            else
//            {
//                Dialog *dialog = dialogMap->get(capBlock->dialogId);
//                if (dialog != NULL) dialog->handle(capBlock);
//            }
//        }
//    }

private:

    /**
     * Sets the properties.
     * @param *properties Properties
     */
    void setProperties(Properties *properties)
    {
        if (properties == NULL)
        {
            throw Exception("null properties", __FILE__, __LINE__);
        }

        // localId
        string strLocalId = properties->getProperty(LOCAL_ID);
        if (strLocalId.length() > 0)
        {
            localId = atoi(strLocalId.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "localId=" + localId);
        }
        else
        {
            throw Exception("illegal " + LOCAL_ID, __FILE__, __LINE__);
        }

        // remoteId
        string strRemoteId = properties->getProperty(REMOTE_ID);
        if (strRemoteId.length() > 0)
        {
            remoteId = atoi(strRemoteId.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "remoteId=" + remoteId);
        }
        else
        {
            throw Exception("illegal " + REMOTE_ID, __FILE__, __LINE__);
        }

        // logicalName
        logicalName = properties->getProperty(LOGICAL_NAME);
        Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "logicalName=" + logicalName);
        if (logicalName.length() == 0)
        {
            throw Exception("illegal " + LOGICAL_NAME, __FILE__, __LINE__);
        }
        
        // capInit.nodeName
        string nodeName = properties->getProperty(NODE_NAME);
        if (nodeName.length() > 0)
        {
            strcpy(gMAPInit.nodeName, nodeName.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", string("nodeName=") + string(gMAPInit.nodeName));
        }
        else
        {
            throw Exception("illegal " + NODE_NAME, __FILE__, __LINE__);
        }
        
        // Local PC
        string strLocalPC = properties->getProperty(LOCAL_PC);
        if (strLocalPC.length() > 0)
        {
            localPC = atoi(strLocalPC.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "localPC=" + localPC);
        }
        else
        {
            throw Exception("Illegal " + LOCAL_PC, __FILE__, __LINE__);
        }
        
        // local GT
        localGT = properties->getProperty(LOCAL_GT);
        Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "localGT=" + localGT);
        if (localGT.length() == 0)
        {
            throw Exception("Illegal " + LOCAL_GT, __FILE__, __LINE__);
        }
        
        // Local GT Type
        string strLocalGTType = properties->getProperty(LOCAL_GT_TYPE);
        if (strLocalGTType.length() > 0)
        {
            localGTType = atoi(strLocalGTType.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "localGTType=" + localGTType);
        }
        else
        {
            throw Exception("Illegal " + LOCAL_GT_TYPE, __FILE__, __LINE__);
        }
        
        // gMAPInit.ssn.ssn
        string strSSN = properties->getProperty(LOCAL_SSN, "8");
        if (strSSN.length() > 0)
        {
            gMAPInit.ssn = (unsigned char) atoi(strSSN.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "localSSN=" + gMAPInit.ssn);
        }
        else
        {
            throw Exception("illegal " + LOCAL_SSN, __FILE__, __LINE__);
        }
        
        // remote PC
        string strRemotePC = properties->getProperty(REMOTE_PC);
        if (strRemotePC.length() > 0)
        {
            remotePC = atoi(strRemotePC.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "remotePC=" + remotePC);
        }
        else
        {
            throw Exception("Illegal " + REMOTE_PC, __FILE__, __LINE__);
        }
        
        // remote GT
        remoteGT = properties->getProperty(REMOTE_GT);
        Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "remoteGT=" + remoteGT);
        if (remoteGT.length() == 0)
        {
            throw Exception("Illegal " + REMOTE_GT, __FILE__, __LINE__);
        }
        
        // remote GT type
        string strRemoteGTType = properties->getProperty(REMOTE_GT_TYPE);
        if (strLocalGTType.length() > 0)
        {
            remoteGTType = atoi(strRemoteGTType.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "remoteGTType=" + remoteGTType);
        }
        else
        {
            throw Exception("Illegal " + REMOTE_GT_TYPE, __FILE__, __LINE__);
        }
        
        // remote SSN
        string strRemoteSSN = properties->getProperty(REMOTE_SSN);
        if (strRemoteSSN.length() > 0)
        {
            remoteSSN = atoi(strRemoteSSN.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "remoteSSN=" + remoteSSN);
        }
        else
        {
            throw Exception("Illegal " + REMOTE_SSN, __FILE__, __LINE__);
        }
        
        // capInit.nDialogs
        string strNDialogs = properties->getProperty(N_DIALOGS, "16383");
        if (strNDialogs.length() > 0)
        {
            gMAPInit.nDialogs = atoi(strNDialogs.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "nDialogs=" + gMAPInit.nDialogs);
        }
        else
        {
            throw Exception("illegal " + N_DIALOGS, __FILE__, __LINE__);
        }

        // capInit.nInvokes
        string strNInvokes = properties->getProperty(N_INVOKES, "16383");
        if (strNInvokes.length() > 0)
        {
            gMAPInit.nInvokes = atoi(strNInvokes.c_str());
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "nInvokes=" + gMAPInit.nInvokes);
        }
        else
        {
            throw Exception("illegal " + N_INVOKES, __FILE__, __LINE__);
        }

        // standAlone
        std::string strStandAlone = properties->getProperty(STAND_ALONE, "false");
        if (strStandAlone.compare("true") == 0)
        {
            standAlone = true;
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "standAlone=true");
        }
        else
        {
            standAlone = false;
            Logger::getLogger()->logp(&Level::FINE, "GMAPApp", "setProperties", "standAlone=false");
        }
    }
};

string GMAPApp::LOCAL_ID = "localId";
string GMAPApp::REMOTE_ID = "remoteId";
string GMAPApp::LOGICAL_NAME = "logicalName";
string GMAPApp::N_DIALOGS = "nDialogs";
string GMAPApp::N_INVOKES = "nInvokes";
string GMAPApp::N_COM_BUFS = "nComBufs";
string GMAPApp::NODE_NAME = "nodeName";
string GMAPApp::STAND_ALONE = "standAlone";

string GMAPApp::LOCAL_PC = "localPC";
string GMAPApp::LOCAL_GT = "localGT";
string GMAPApp::LOCAL_GT_TYPE = "localGTType";
string GMAPApp::LOCAL_SSN = "localSSN";

string GMAPApp::REMOTE_PC = "remotePC";
string GMAPApp::REMOTE_GT = "remoteGT";
string GMAPApp::REMOTE_GT_TYPE = "remoteGTType";
string GMAPApp::REMOTE_SSN = "remoteSSN";

#endif	/* CAPAPP_HPP */
