/*
 * cftask.hpp
 */
#ifndef CFTASK_HPP
#define	CFTASK_HPP

#include "block.hpp"
#include "appconn/cfie.hpp"
#include "appconn/cfmessage.hpp"
#include "../../base/appconn/appclient.hpp"

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

#endif	/* CFTASK_HPP */
