/**
 * sever.c implements the server part of the unit test
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */
#include "legato.h"
#include "interfaces.h"
#include "defs.h"

#define DSIZE           512     /* default buffer size */
#define COMMANDS_MAX    50
#define PARAM_MAX       24

//--------------------------------------------------------------------------------------------------
/**
 * AtCmd_t definition
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    const char* atCmdPtr;
    le_atServer_CmdRef_t cmdRef;
    le_atServer_CommandHandlerFunc_t handlerPtr;
}
AtCmd_t;

//--------------------------------------------------------------------------------------------------
/**
 * struct AtSession definition
 *
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_atServer_DeviceRef_t devRef;
    int cmdsCount;
    AtCmd_t atCmds[COMMANDS_MAX];
}
AtSession_t;

//--------------------------------------------------------------------------------------------------
/**
 * at server session
 *
 */
//--------------------------------------------------------------------------------------------------
static AtSession_t AtSession;

//--------------------------------------------------------------------------------------------------
/**
 * AT command handler
 *
 * API tested:
 *      le_atServer_GetCommandName
 *      le_atServer_SendIntermediateResponse
 *      le_atServer_GetParameter
 *      le_atServer_SendFinalResponse
 *
 */
//--------------------------------------------------------------------------------------------------
static void AtCmdHandler
(
    le_atServer_CmdRef_t commandRef,
    le_atServer_Type_t type,
    uint32_t parametersNumber,
    void* contextPtr
)
{
    char atCommandName[LE_ATSERVER_COMMAND_MAX_BYTES];
    char rsp[LE_ATSERVER_RESPONSE_MAX_BYTES];
    char param[LE_ATSERVER_PARAMETER_MAX_BYTES];
    int i = 0;

    LE_DEBUG("commandRef %p", commandRef);

    // check whether command's name is registred on the server app
    memset(atCommandName,0,LE_ATSERVER_COMMAND_MAX_BYTES);
    LE_ASSERT(le_atServer_GetCommandName(commandRef,
                atCommandName,LE_ATSERVER_COMMAND_MAX_BYTES)
            == LE_OK);

    LE_DEBUG("AT command name %s", atCommandName);

    memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
    snprintf(rsp, LE_ATSERVER_RESPONSE_MAX_BYTES, "%s TYPE: ", atCommandName+2);

    switch (type)
    {
        case LE_ATSERVER_TYPE_PARA:
            LE_DEBUG("Type PARA");
            snprintf(rsp+strlen(rsp),
                LE_ATSERVER_RESPONSE_MAX_BYTES-strlen(rsp), "PARA");
        break;

        case LE_ATSERVER_TYPE_TEST:
            LE_DEBUG("Type TEST");
            snprintf(rsp+strlen(rsp),
                LE_ATSERVER_RESPONSE_MAX_BYTES-strlen(rsp), "TEST");
        break;

        case LE_ATSERVER_TYPE_READ:
            LE_DEBUG("Type READ");
            snprintf(rsp+strlen(rsp),
                LE_ATSERVER_RESPONSE_MAX_BYTES-strlen(rsp), "READ");
        break;

        case LE_ATSERVER_TYPE_ACT:
            LE_DEBUG("Type ACT");
            snprintf(rsp+strlen(rsp),
                LE_ATSERVER_RESPONSE_MAX_BYTES-strlen(rsp), "ACT");
        break;

        default:
            LE_ASSERT(0);
        break;
    }

    // send an intermediate response with the command type
    LE_ASSERT(le_atServer_SendIntermediateResponse(commandRef,
                                                rsp)
            == LE_OK);

    // get the command parameters and send them in an intermediate response
    for (i = 0; i < parametersNumber && parametersNumber <= PARAM_MAX; i++)
    {
        memset(param,0,LE_ATSERVER_PARAMETER_MAX_BYTES);
        LE_ASSERT(le_atServer_GetParameter(commandRef,
                                        i,
                                        param,
                                        LE_ATSERVER_PARAMETER_MAX_BYTES)
                == LE_OK);
        LE_DEBUG("Param %d: %s", i, param);

        memset(rsp,0,LE_ATSERVER_RESPONSE_MAX_BYTES);
        snprintf(rsp,LE_ATSERVER_RESPONSE_MAX_BYTES,
            "%s PARAM %d: %s", atCommandName+2, i, param);

        LE_ASSERT(
            le_atServer_SendIntermediateResponse(commandRef, rsp)
            == LE_OK);
    }

    // test for bad parameter
    LE_ASSERT(le_atServer_GetParameter(commandRef,
                                    parametersNumber+1,
                                    param,
                                    LE_ATSERVER_PARAMETER_MAX_BYTES )
            == LE_BAD_PARAMETER);

    // send OK final response
    LE_ASSERT(le_atServer_SendFinalResponse(commandRef,
                                        LE_ATSERVER_OK, false, "")
        == LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * ATI command handler
 *
 * tests long responses and multiple intermediates send
 *
 * API tested:
 *      le_atServer_SendIntermediateResponse
 *      le_atServer_SendFinalResponse
 *
 */
//--------------------------------------------------------------------------------------------------
static void AtiCmdHandler
(
    le_atServer_CmdRef_t commandRef,
    le_atServer_Type_t type,
    uint32_t parametersNumber,
    void* contextPtr
)
{
    char rsp[LE_ATSERVER_RESPONSE_MAX_BYTES];

    switch (type)
    {
        // this command cannot be of types parameter, test or read
        case LE_ATSERVER_TYPE_PARA:
        case LE_ATSERVER_TYPE_TEST:
        case LE_ATSERVER_TYPE_READ:
            LE_ASSERT(
                // send and ERROR final response
                le_atServer_SendFinalResponse(commandRef,
                    LE_ATSERVER_ERROR, false, "")
                == LE_OK);
            break;
        // this is an action type command so send multiple intermediate
        // responses and an OK final response
        case LE_ATSERVER_TYPE_ACT:
            memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
            sprintf(rsp, "%s",
                "Manufacturer: Sierra Wireless, Incorporated");
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef, rsp)
                == LE_OK);

            memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
            sprintf(rsp, "%s", "Model: WP8548");
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef, rsp)
                == LE_OK);

            memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
            sprintf(rsp, "%s",
                "Revision: SWI9X15Y_07.10.04.00 12c1700 jenkins"
                " 2016/06/02 02:52:45");
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef, rsp)
                == LE_OK);

            memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
            sprintf(rsp, "%s", "IMEI: 359377060009700");
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef, rsp)
                == LE_OK);

            memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
            sprintf(rsp, "%s", "IMEI SV: 42");
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef, rsp)
                == LE_OK);

            memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
            sprintf(rsp, "%s", "FSN: LL542500111503");
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef, rsp)
                == LE_OK);

            memset(rsp, 0, LE_ATSERVER_RESPONSE_MAX_BYTES);
            sprintf(rsp, "%s", "+GCAP: +CGSM");
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef, rsp)
                == LE_OK);
            // send an OK final response
            LE_ASSERT(
                le_atServer_SendFinalResponse(commandRef,
                    LE_ATSERVER_OK, false, "")
                == LE_OK);
            break;

        default:
            LE_ASSERT(0);
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * get command name refrence
 *
 * atServer API doesn't provide a way to get command's refrence directly
 * loop through the table, if it's a known command return its refrence
 *
 */
//--------------------------------------------------------------------------------------------------
static le_atServer_CmdRef_t GetRef
(
    AtCmd_t* AtCmds,
    int cmdsCount,
    const char* cmdNamePtr
)
{
    int i = 0;

    for (i=0; i<cmdsCount; i++)
    {
        if ( ! (strcmp(AtCmds[i].atCmdPtr, cmdNamePtr)) )
        {
            return AtCmds[i].cmdRef;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * AT+DEL command handler
 *
 * tests commands deletion
 *
 * API tested:
 *      le_atServer_GetParameter
 *      le_atServer_SendIntermediateResponse
 *      le_atServer_SendFinalResponse
 *      le_atServer_Delete
 *
 */
//--------------------------------------------------------------------------------------------------
static void DelCmdHandler
(
    le_atServer_CmdRef_t commandRef,
    le_atServer_Type_t type,
    uint32_t parametersNumber,
    void* contextPtr
)
{
    AtSession_t *AtSession = (AtSession_t *)contextPtr;
    le_atServer_FinalRsp_t finalRsp = LE_ATSERVER_OK;
    le_atServer_CmdRef_t cmdRef;
    char param[LE_ATSERVER_PARAMETER_MAX_BYTES];
    int i = 0;

    switch (type)
    {
        case LE_ATSERVER_TYPE_PARA:
            for (i = 0;
                i < parametersNumber && parametersNumber <= PARAM_MAX;
                i++)
            {
                memset(param,0,LE_ATSERVER_PARAMETER_MAX_BYTES);
                // get the command to delete
                LE_ASSERT(
                le_atServer_GetParameter(commandRef,
                                        i,
                                        param,
                                        LE_ATSERVER_PARAMETER_MAX_BYTES)
                                    == LE_OK);
                // get its refrence
                cmdRef = GetRef(AtSession->atCmds, AtSession->cmdsCount, param);
                LE_DEBUG("Deleting %p => %s", cmdRef, param);
                // delete the command
                LE_ASSERT(le_atServer_Delete(cmdRef) == LE_OK);
            }
            // send an OK final response
            finalRsp = LE_ATSERVER_OK;
            break;
        // this command doesn't support test and read send an ERROR final
        // reponse
        case LE_ATSERVER_TYPE_TEST:
        case LE_ATSERVER_TYPE_READ:
            finalRsp = LE_ATSERVER_ERROR;
            break;
        // an action command type to verify that AT+DEL command does exist
        // send an OK final response
        case LE_ATSERVER_TYPE_ACT:
            finalRsp = LE_ATSERVER_OK;
            break;

        default:
            LE_ASSERT(0);
        break;
    }
    // send final response
    LE_ASSERT(
        le_atServer_SendFinalResponse(
            commandRef, finalRsp, false, "")
        == LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * STOP command handler
 *
 * tests closing server session
 *
 * API tested:
 *      le_atServer_SendFinalResponse
 *      le_atServer_Stop
 */
//--------------------------------------------------------------------------------------------------
static void StopCmdHandler
(
    le_atServer_CmdRef_t commandRef,
    le_atServer_Type_t type,
    uint32_t parametersNumber,
    void* contextPtr
)
{
    AtSession_t* AtSession = (AtSession_t *)contextPtr;

    switch (type) {
    // this command doesn't accept parameter, test or read send an ERROR
    // final response
    case LE_ATSERVER_TYPE_PARA:
    case LE_ATSERVER_TYPE_TEST:
    case LE_ATSERVER_TYPE_READ:
        LE_ASSERT(
            le_atServer_SendFinalResponse(
                commandRef, LE_ATSERVER_ERROR, false, "")
            == LE_OK);

        break;
    // in case of an action command just close the session
    // we cannot send a response, the closing is in progress
    case LE_ATSERVER_TYPE_ACT:
        LE_ASSERT(le_atServer_Stop(AtSession->devRef) == LE_OK);
        break;

    default:
        LE_ASSERT(0);
        break;
    }
}

//------------------------------------------------------------------------------
/**
 * CBC command handler
 *
 * tests unsolicited responses
 *
 * API tested:
 *      le_atServer_SendIntermediateResponse
 *      le_atServer_SendFinalResponse
 *      le_atServer_SendUnsolicitedResponse, specific device and all devices
 *
 */
//------------------------------------------------------------------------------
static void CbcCmdHandler
(
    le_atServer_CmdRef_t commandRef,
    le_atServer_Type_t type,
    uint32_t parametersNumber,
    void* contextPtr
)
{
    AtSession_t* AtSession = (AtSession_t *)contextPtr;
    le_atServer_FinalRsp_t finalRsp = LE_ATSERVER_OK;

    switch (type)
    {
        // this command doesn't support parameter type send an ERROR
        // final response
        case LE_ATSERVER_TYPE_PARA:
            finalRsp = LE_ATSERVER_ERROR;
            break;
        // tell the user/host how to read the command, send an OK final response
        case LE_ATSERVER_TYPE_TEST:
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef,
                    "+CBC: (0-2),(1-100),(voltage)")
                == LE_OK);
            finalRsp = LE_ATSERVER_OK;
            break;
        // read isn't allowed
        case LE_ATSERVER_TYPE_READ:
            finalRsp = LE_ATSERVER_ERROR;
            break;
        // send an intermediate response containing the values
        // send unsolicited responses with updates
        case LE_ATSERVER_TYPE_ACT:
            LE_ASSERT(
                le_atServer_SendIntermediateResponse(commandRef,
                    "+CBC: 1,50,4190")
                == LE_OK);
            LE_ASSERT(le_atServer_SendUnsolicitedResponse("+CBC: 1,70,4190",
                            LE_ATSERVER_SPECIFIC_DEVICE, AtSession->devRef)
                == LE_OK);
            LE_ASSERT(le_atServer_SendUnsolicitedResponse("+CBC: 2,100,4190",
                            LE_ATSERVER_ALL_DEVICES, 0)
                == LE_OK);
            finalRsp = LE_ATSERVER_OK;
            break;

        default:
            LE_ASSERT(0);
            break;
    }
    // send an OK final response
    LE_ASSERT(le_atServer_SendFinalResponse(commandRef, finalRsp, false, "")
        == LE_OK);

}

//--------------------------------------------------------------------------------------------------
/**
 * server thread function
 *
 * the main function of the thread
 * start the server
 * initialize/create new commands and register them within the server app
 *
 * API tested:
 *      le_atServer_Start
 *      le_atServer_Create
 *      le_atServer_AddCommandHandler
 *
 */
//--------------------------------------------------------------------------------------------------
void* AtServer
(
    void* contextPtr
)
{
    SharedData_t* sharedDataPtr;
    char errMsg[DSIZE];
    struct sockaddr_un addr;
    int socketFd;
    int connFd;
    int i = 0;

    AtCmd_t AtCmdCreation[] =
    {
        {
            .atCmdPtr = "ATI",
            .cmdRef = NULL,
            .handlerPtr = AtiCmdHandler,
        },
        {
            .atCmdPtr = "AT+CBC",
            .cmdRef = NULL,
            .handlerPtr = CbcCmdHandler,
        },
        {
            .atCmdPtr = "AT+DEL",
            .cmdRef = NULL,
            .handlerPtr = DelCmdHandler,
        },
        {
            .atCmdPtr = "AT+STOP",
            .cmdRef = NULL,
            .handlerPtr = StopCmdHandler,
        },
        {
            .atCmdPtr = "AT",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "AT+ABCD",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "ATA",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "AT&F",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "ATS",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "ATV",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "AT&C",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "AT&D",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
        {
            .atCmdPtr = "ATE",
            .cmdRef = NULL,
            .handlerPtr = AtCmdHandler,
        },
    };

    sharedDataPtr = (SharedData_t *)contextPtr;

    LE_DEBUG("Server Started");

    pthread_mutex_lock(&sharedDataPtr->mutex);

    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == -1)
    {
        memset(errMsg, 0, DSIZE);
        LE_ERROR("socket failed: %s",
            strerror_r(errno, errMsg, DSIZE));
        return NULL;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family= AF_UNIX;
    strncpy(addr.sun_path, sharedDataPtr->devPathPtr, sizeof(addr.sun_path)-1);

    if (bind(socketFd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        memset(errMsg, 0, DSIZE);
        LE_ERROR(" bind failed: %s",
            strerror_r(errno, errMsg, DSIZE));
        return NULL;
    }

    if (listen(socketFd, 1) == -1)
    {
        memset(errMsg, 0, DSIZE);
        LE_ERROR("listen failed: %s",
            strerror_r(errno, errMsg, DSIZE));
        return NULL;
    }

    sharedDataPtr->ready = true;
    pthread_cond_signal(&sharedDataPtr->cond);
    pthread_mutex_unlock(&sharedDataPtr->mutex);

    connFd = accept(socketFd, NULL, NULL);
    if (connFd == -1)
    {
        memset(errMsg, 0, DSIZE);
        LE_ERROR("accept failed: %s",
            strerror_r(errno, errMsg, DSIZE));
        return NULL;
    }

    // start the server
    AtSession.devRef = le_atServer_Start(connFd);
    LE_ASSERT(AtSession.devRef != NULL);

    AtSession.cmdsCount = NUM_ARRAY_MEMBERS(AtCmdCreation);

    // AT commands subscriptions
    while (i < AtSession.cmdsCount)
    {
        AtCmdCreation[i].cmdRef = le_atServer_Create(AtCmdCreation[i].atCmdPtr);
        LE_ASSERT(AtCmdCreation[i].cmdRef != NULL);

        AtSession.atCmds[i] = AtCmdCreation[i];

        LE_ASSERT(le_atServer_AddCommandHandler(
                AtCmdCreation[i].cmdRef,
                AtCmdCreation[i].handlerPtr,
                (void *)&AtSession)
            != NULL);

        i++;
    }

    le_event_RunLoop();

    return NULL;
}
