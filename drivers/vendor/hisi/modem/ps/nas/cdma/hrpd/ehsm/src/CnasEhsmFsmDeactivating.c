/******************************************************************************

             Copyright 2014, Huawei Technologies Co. Ltd.

  ******************************************************************************
 File Name       :   CnasEhsmFsmDeactivating.c
 Author          :
 Version         :
 Date            :   18/05/2015
 Description     :   This file has definition of each message handler in EHSM ACTIVATING FSM.
 Function list   :
 History         :
     1) Date :
        Author:
        Modification:
********************************************************************************/

/*****************************************************************************
  1 Header File Include
*****************************************************************************/
#include "CnasEhsmFsmDeactivating.h"
#include "CnasEhsmCtx.h"
#include "CnasEhsmMntn.h"
#include "ehsm_aps_pif.h"
/* To Do: Need to add EHSM to APS interface file */

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

#define THIS_FILE_ID                    PS_FILE_ID_CNAS_EHSM_FSM_DEACTIVATING_C

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

/*****************************************************************************
  2 The Define Of The Gloabal Variable
*****************************************************************************/


/*****************************************************************************
  3 Function Define
*****************************************************************************/

/* To Do: Need to write deifinitions of all functions of EHSM deactivating FSM message handlers */

VOS_UINT32 CNAS_EHSM_RcvHsdPowerOffReq_Deactivating_Init(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /*save entry msg*/
    CNAS_EHSM_SaveCurEntryMsg(ulEventType, pstMsg);

    /*power off ,whether the air link is exist */
    if (VOS_FALSE == CNAS_EHSM_GetAirLinkExistFlag())
    {
        CNAS_EHSM_SndHsmConnEstReq();
        CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_HSM_CONN_EST_CNF);
        CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF, TI_CNAS_EHSM_WAIT_HSM_CONN_CNF_LEN);
    }
    else
    {
        CNAS_EHSM_SetSendPdnDetachFlag_Deactivating(VOS_TRUE);

        CNAS_EHSM_SndCttfEhrpdDetachReq();
        CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_DETACH_CNF);
        CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF, TI_CNAS_EHSM_WAIT_DETACH_CNF_LEN);
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvApsPdnDeactivate_Deactivating_Init(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDiscReq;

    pstPdnDiscReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)pstMsg;

    /*save entry msg*/
    CNAS_EHSM_SaveCurEntryMsg(ulEventType,pstMsg);

    /*pdn deactivate,judge whether air link is exist*/
    if (VOS_FALSE == CNAS_EHSM_GetAirLinkExistFlag())
    {
        CNAS_EHSM_SndHsmConnEstReq();
        CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_HSM_CONN_EST_CNF);
        CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF, TI_CNAS_EHSM_WAIT_HSM_CONN_CNF_LEN);
    }
    else
    {
        if (VOS_TRUE != CNAS_EHSM_GetEhrpdAutoAttachFlag())
        {
            if (2 == CNAS_EHSM_GetLocalActivePdnConnNum())
            {
                if (CNAS_EHSM_INVALID_PDN_ID != CNAS_EHSM_GetPdnIdByCid(CNAS_EHSM_DEFAULT_CID))
                {
                    CNAS_EHSM_SndCttfEhrpdDetachReq();

                    CNAS_EHSM_SetSendPdnDetachFlag_Deactivating(VOS_TRUE);

                    CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_DETACH_CNF);

                    CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF, TI_CNAS_EHSM_WAIT_DETACH_CNF_LEN);

                    return VOS_TRUE;
                }
            }
        }

        if (1 == CNAS_EHSM_GetLocalActivePdnConnNum())
        {
            CNAS_EHSM_SndCttfEhrpdDetachReq();

            CNAS_EHSM_SetSendPdnDetachFlag_Deactivating(VOS_TRUE);

            CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_DETACH_CNF);

            CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF, TI_CNAS_EHSM_WAIT_DETACH_CNF_LEN);
        }
        else
        {
            CNAS_EHSM_SndCttfEhrpdPdnDiscReq(pstPdnDiscReq->ucPdnId);

            CNAS_EHSM_SetSendPdnDeactFlag_Deactivating(VOS_TRUE);

            CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_PDN_DISC_CNF);

            CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF, TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF_LEN);
        }
    }

    return VOS_TRUE;
}



VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdPdnDiscCnf_Deactivating_WaitPdnDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_PDN_DISC_CNF_STRU  *pstPdnDiscCnf = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo    = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstPdnDiscCnf = (CTTF_CNAS_EHRPD_PDN_DISC_CNF_STRU *)pstMsg;

    ucPdnId       = pstPdnDiscCnf->ucPdnId;
    ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

    /* 判断消息有效性 */
    if (VOS_FALSE == CNAS_EHSM_IsPdnActived(ucPdnId))
    {
        return VOS_TRUE;
    }

    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

    CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

    CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);

    if (CNAS_CTTF_EHRPD_RSLT_FAILURE == pstPdnDiscCnf->enDiscRslt)
    {
        CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);
    }

    CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

    CNAS_EHSM_SndDeactivatingRslt();

    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdPdnDiscInd_Deactivating_WaitPdnDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_PDN_DISC_IND_STRU  *pstPdnDiscInd = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo    = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstPdnDiscInd = (CTTF_CNAS_EHRPD_PDN_DISC_IND_STRU *)pstMsg;

    ucPdnId       = pstPdnDiscInd->ucPdnId;
    ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

    /* 判断消息有效性 */
    if (VOS_FALSE == CNAS_EHSM_IsPdnActived(pstPdnDiscInd->ucPdnId))
    {
        return VOS_TRUE;
    }

    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

    CNAS_EHSM_SndApsPdnDeactivateCnf(pstPdnDiscInd->ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

    CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);

    CNAS_EHSM_ClearEhrpdLocalPdnInfo(pstPdnDiscInd->ucPdnId);

    CNAS_EHSM_SndDeactivatingRslt();

    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdReconnInd_Deactivating_WaitPdnDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_SndHsmConnEstReq();

    CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_HSM_CONN_EST_CNF);

    CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF, TI_CNAS_EHSM_WAIT_HSM_CONN_CNF_LEN);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdDetachCnf_Deactivating_WaitDetachCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_DETACH_CNF_STRU    *pstEhrpdDetachCnf;

    pstEhrpdDetachCnf = (CTTF_CNAS_EHRPD_DETACH_CNF_STRU *)pstMsg;

    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF);

    if (CNAS_CTTF_EHRPD_RSLT_SUCCESS != pstEhrpdDetachCnf->enDetachRslt)
    {
        CNAS_EHSM_SndCttfEhrpdDetachInd();
    }

    CNAS_EHSM_ProcAbortMsg_Deactivating(EHSM_APS_CAUSE_SUCCESS);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdReconnInd_Deactivating_WaitDetachCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_SndHsmConnEstReq();

    CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_HSM_CONN_EST_CNF);

    CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF, TI_CNAS_EHSM_WAIT_HSM_CONN_CNF_LEN);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvHsmConnEstCnf_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HSM_EHSM_CONN_EST_CNF_STRU         *pstConnEstCnf       = VOS_NULL_PTR;
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg         = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDeactivatReq  = VOS_NULL_PTR;

    pstEntryMsg   = CNAS_EHSM_GetCurrFsmEntryMsgAddr();
    pstConnEstCnf = (HSM_EHSM_CONN_EST_CNF_STRU *)pstMsg;

    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);

    if (EHSM_HSM_RSLT_SUCCESS == pstConnEstCnf->enResult)
    {
        /* 更新EHRPD空口链路状态 */
        CNAS_EHSM_SetAirLinkExistFlag(VOS_TRUE);
    }

    /* 先判断power off 标记，如果为true 直接退出状态机*/
    if (VOS_TRUE == CNAS_EHSM_GetAbortFlag_Deactivating())
    {
        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();

        return VOS_TRUE;
    }

    if (EHSM_HSM_RSLT_SUCCESS == pstConnEstCnf->enResult)
    {
        CNAS_EHSM_ResetConntRetryTimes_Deactivating();

        /* 之前已经发过Detach请求,本次链路重建是由PPP触发的，建链成功之后通知PPP */
        if (VOS_TRUE == CNAS_EHSM_GetSendPdnDetachFlag_Deactivating())
        {
            CNAS_EHSM_SndCttfEhrpdLinkStatusNotify(CNAS_CTTF_EHRPD_LINK_CONNECTED);

            /* 更新状态机状态 */
            CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_DETACH_CNF);

            return VOS_TRUE;
        }

        /* 之前已经发过Pdn Disc请求,本次链路重建是由PPP触发的，建链成功之后通知PPP */
        if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
        {
            CNAS_EHSM_SndCttfEhrpdLinkStatusNotify(CNAS_CTTF_EHRPD_LINK_CONNECTED);

            CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_PDN_DISC_CNF);

            return VOS_TRUE;
        }

        if (pstEntryMsg->ulEventType == CNAS_BuildEventType(UEPS_PID_HSD, ID_HSD_EHSM_POWER_OFF_REQ))
        {
            /* 发送Detach请求 */
            CNAS_EHSM_SndCttfEhrpdDetachReq();

            CNAS_EHSM_SetSendPdnDetachFlag_Deactivating(VOS_TRUE);

            CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF, TI_CNAS_EHSM_WAIT_DETACH_CNF_LEN);

            /* 更新状态机状态 */
            CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_DETACH_CNF);
        }

        if (CNAS_BuildEventType(WUEPS_PID_TAF, ID_APS_EHSM_PDN_DEACTIVATE_REQ) == pstEntryMsg->ulEventType)
        {
            pstPdnDeactivatReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);

            if (VOS_TRUE != CNAS_EHSM_GetEhrpdAutoAttachFlag())
            {
                if (2 == CNAS_EHSM_GetLocalActivePdnConnNum())
                {
                    if (CNAS_EHSM_INVALID_PDN_ID != CNAS_EHSM_GetPdnIdByCid(CNAS_EHSM_DEFAULT_CID))
                    {
                        CNAS_EHSM_SndCttfEhrpdDetachReq();

                        CNAS_EHSM_SetSendPdnDetachFlag_Deactivating(VOS_TRUE);

                        CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_DETACH_CNF);

                        CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF, TI_CNAS_EHSM_WAIT_DETACH_CNF_LEN);

                        return VOS_TRUE;
                    }
                }
            }

            if (1 == CNAS_EHSM_GetLocalActivePdnConnNum())
            {
                CNAS_EHSM_SndCttfEhrpdDetachReq();

                CNAS_EHSM_SetSendPdnDetachFlag_Deactivating(VOS_TRUE);

                CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_DETACH_CNF);

                CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF, TI_CNAS_EHSM_WAIT_DETACH_CNF_LEN);
            }
            else
            {
                /* 发送PDN Disc Req */
                CNAS_EHSM_SndCttfEhrpdPdnDiscReq(pstPdnDeactivatReq->ucPdnId);

                CNAS_EHSM_SetSendPdnDeactFlag_Deactivating(VOS_TRUE);

                CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF, TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF_LEN);

                /* 更新状态机状态 */
                CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_PDN_DISC_CNF);
            }
        }
    }
    else
    {
        /*如果是建联失败，入口消息是pdn deactivate请求，需要retry*/
        CNAS_EHSM_ProcConnEstFail_Deactivating((pstConnEstCnf->enResult), VOS_FALSE);

    }
    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiWaitHsmConnEstCnf_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* 先判断power off 标记，如果为true 直接退出状态机*/
    if (VOS_TRUE == CNAS_EHSM_GetAbortFlag_Deactivating())
    {
        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();

        return VOS_TRUE;
    }

    /*如果是建联超时,处理流程和建联失败相似，入口消息是pdn deactivate ，不需要建联，本地去激活。*/
    CNAS_EHSM_ProcConnEstFail_Deactivating((EHSM_HSM_RSLT_TIMEOUT), VOS_TRUE);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiWaitConnRetryEst_Deativating_WaitConnEstRetry(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* Increment the COnnection establish retry times */
    CNAS_EHSM_IncreConntRetryTimes_Deactivating();

    CNAS_EHSM_SndHsmConnEstReq();

    CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF, TI_CNAS_EHSM_WAIT_HSM_CONN_CNF_LEN);

    CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_HSM_CONN_EST_CNF);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvHsmSessionInfoInd_Deativating_WaitConnEstRetry(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HSM_EHSM_SESSION_INFO_IND_STRU     *pstHsmSessionInd = VOS_NULL_PTR;

    pstHsmSessionInd = (HSM_EHSM_SESSION_INFO_IND_STRU *)pstMsg;

    CNAS_EHSM_SetSessionType(pstHsmSessionInd->enSessionType);

    if (EHSM_HSM_SESSION_TYPE_EHRPD == pstHsmSessionInd->enSessionType)
    {
        /* 停止保护定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CONN_RETRY_EST);

        /* Increment the COnnection establish retry times */
        CNAS_EHSM_IncreConntRetryTimes_Deactivating();

        CNAS_EHSM_SndHsmConnEstReq();

        CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF, TI_CNAS_EHSM_WAIT_HSM_CONN_CNF_LEN);

        CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_HSM_CONN_EST_CNF);

        return VOS_TRUE;
    }
    else
    {
        /* 此处必须返回VOS_FALSE,非EHRPD模式后续会处理为Abort的内部消息 */
        return VOS_FALSE;
    }
}


VOS_UINT32  CNAS_EHSM_RcvTiWaitCttfPdnDiscCnf_Deactivating_WaitPdnDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg     = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDiscReq   = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo      = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstEntryMsg   = CNAS_EHSM_GetCurrFsmEntryMsgAddr();
    pstPdnDiscReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);
    ucPdnId       = pstPdnDiscReq->ucPdnId;
    ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

    CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_TIME_OUT);
    CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);

    CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);
    CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

    CNAS_EHSM_SndDeactivatingRslt();
    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiCttfEhrpdDetachCnf_Deactivating_WaitDetachCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_SndCttfEhrpdDetachInd();
    CNAS_EHSM_ProcAbortMsg_Deactivating(EHSM_APS_CAUSE_TIME_OUT);
    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiProtectingPowerOff_Deactivating_WaitDetachCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF);

    CNAS_EHSM_SndCttfEhrpdDetachInd();

    CNAS_EHSM_ProcAbortMsg_Deactivating(EHSM_APS_CAUSE_TIME_OUT);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvHsmDiscInd_Deactivating_WaitPdnDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /*本地去激活，停定时器，air link 标志位设为false*/
    CNAS_EHSM_SetAirLinkExistFlag(VOS_FALSE);

    if (CNAS_EHSM_EHRPD_ACTIVE_STA == CNAS_EHSM_GetEhrpdState())
    {
        CNAS_EHSM_SetEhrpdState(CNAS_EHSM_EHRPD_DORMANT_STA);
    }

    CNAS_EHSM_SndCttfEhrpdLinkStatusNotify(CNAS_CTTF_EHRPD_LINK_NOT_EXISTED);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvHsmDiscInd_Deactivating_WaitDetachCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_SetAirLinkExistFlag(VOS_FALSE);

    if (CNAS_EHSM_EHRPD_ACTIVE_STA == CNAS_EHSM_GetEhrpdState())
    {
        CNAS_EHSM_SetEhrpdState(CNAS_EHSM_EHRPD_DORMANT_STA);
    }

    CNAS_EHSM_SndCttfEhrpdLinkStatusNotify(CNAS_CTTF_EHRPD_LINK_NOT_EXISTED);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvEhsmAbortReq_Deactivating_WaitPdnDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg      = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDeactivate = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo       = VOS_NULL_PTR;
    CNAS_EHSM_EHSM_ABORT_REQ_STRU      *pstAbortMsg      = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstEntryMsg      = CNAS_EHSM_GetCurrFsmEntryMsgAddr();
    pstPdnDeactivate = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);
    ucPdnId          = pstPdnDeactivate->ucPdnId;
    ucCid            = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo       = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);
    pstAbortMsg      = (CNAS_EHSM_EHSM_ABORT_REQ_STRU *)pstMsg;

    /*stop timer*/
    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

    /*CNF MSG*/
    CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_POWER_OFF);

    /*仅网测的ID_CTTF_CNAS_EHRPD_DETACH_IND消息打断不需要通知cttf本地去激活*/
    if (CNAS_BuildEventType(MSPS_PID_PPPC, ID_CTTF_CNAS_EHRPD_DETACH_IND) != pstAbortMsg->ulEventType)
    {
        CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);
    }
    CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);
    CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

    CNAS_EHSM_SndDeactivatingRslt();
    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvEhsmAbortReq_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_EHSM_ABORT_REQ_STRU      *pstRcvAbortMsg = VOS_NULL_PTR;

    pstRcvAbortMsg = (CNAS_EHSM_EHSM_ABORT_REQ_STRU *)pstMsg;

    if (CNAS_BuildEventType(WUEPS_PID_TAF, ID_APS_EHSM_LOC_DETACH_NOTIFY) == pstRcvAbortMsg->ulEventType)
    {
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);
        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();
        return VOS_TRUE;
    }

    /*加abort标记 等收到cnf消息时候判定*/
    CNAS_EHSM_SetAbortFlag_Deactivating(VOS_TRUE);

    CNAS_EHSM_SetAbortEventType_Deactivating(pstRcvAbortMsg->ulEventType);

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiProtectingPowerOff_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_CTX_STRU                 *pstEhsmCtx = VOS_NULL_PTR;
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstEhsmCtx  = CNAS_EHSM_GetEhsmCtxAddr();
    pstEntryMsg = CNAS_EHSM_GetCurrFsmEntryMsgAddr();

    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);

    /*clear local used pdn info*/
    ulIndex = 0;
    while (ulIndex < CNAS_EHSM_MAX_PDN_NUM)
    {
        if (VOS_TRUE == pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucInUsed)
        {
            ucPdnId = pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucPdnId;
            ucCid = pstEhsmCtx->astLocalPdnBearInfo[ucPdnId].ucCid;
            CNAS_EHSM_SndApsPdnDeactivateInd(ucPdnId, ucCid, EHSM_APS_CAUSE_POWER_OFF);
        }
        ulIndex++;
    }
    CNAS_EHSM_ClearAllEhrpdLocalPdnInfo();
    CNAS_EHSM_SndCttfEhrpdDetachInd();

    CNAS_EHSM_SndEsmClearAllNtf();
    if (CNAS_BuildEventType(UEPS_PID_HSD, ID_HSD_EHSM_POWER_OFF_REQ) == pstEntryMsg->ulEventType)
    {
        CNAS_EHSM_SndHsdPowerOffCnf();

        CNAS_EHSM_ClearCtx(CNAS_EHSM_INIT_CTX_POWEROFF);
    }

    CNAS_EHSM_SndDeactivatingRslt();
    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdPdnDiscCnf_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_PDN_DISC_CNF_STRU  *pstPdnDiscCnf = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo    = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstPdnDiscCnf = (CTTF_CNAS_EHRPD_PDN_DISC_CNF_STRU *)pstMsg;
    ucPdnId       = pstPdnDiscCnf->ucPdnId;
    ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

    /* 判断消息有效性 */
    if (VOS_FALSE == CNAS_EHSM_IsPdnActived(ucPdnId))
    {
        return VOS_TRUE;
    }

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
    {
        /* 停止本次建链流程保护定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);

        /* 停止之前PDN去激活定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

        CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);

        if (CNAS_CTTF_EHRPD_RSLT_FAILURE == pstPdnDiscCnf->enDiscRslt)
        {
            CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);
        }

        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdPdnDiscInd_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_PDN_DISC_IND_STRU  *pstPdnDiscInd = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo    = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstPdnDiscInd = (CTTF_CNAS_EHRPD_PDN_DISC_IND_STRU *)pstMsg;
    ucPdnId       = pstPdnDiscInd->ucPdnId;
    ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

    /* 判断消息有效性 */
    if (VOS_FALSE == CNAS_EHSM_IsPdnActived(ucPdnId))
    {
        return VOS_TRUE;
    }

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
    {
        /* 停止本次建链流程保护定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);

        /* 停止之前PDN去激活定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

        CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);

        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdDetachCnf_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_DETACH_CNF_STRU    *pstEhrpdDetachCnf;

    pstEhrpdDetachCnf = (CTTF_CNAS_EHRPD_DETACH_CNF_STRU *)pstMsg;

    /* 如果之前发送过Detach Req */
    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDetachFlag_Deactivating())
    {
        /* 停止本次建链流程保护定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);

        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF);

        if (CNAS_CTTF_EHRPD_RSLT_SUCCESS != pstEhrpdDetachCnf->enDetachRslt)
        {
            CNAS_EHSM_SndCttfEhrpdDetachInd();
        }

        CNAS_EHSM_ProcAbortMsg_Deactivating(EHSM_APS_CAUSE_SUCCESS);
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiWaitCttfPdnDiscCnfExpired_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg     = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDiscReq   = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo      = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
    {
        /* 停止本次建链流程保护定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);

        pstEntryMsg   = CNAS_EHSM_GetCurrFsmEntryMsgAddr();
        pstPdnDiscReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);
        ucPdnId       = pstPdnDiscReq->ucPdnId;
        ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
        pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_TIME_OUT);
        CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);

        CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);
        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

        CNAS_EHSM_SndDeactivatingRslt();
        CNAS_EHSM_QuitFsmL2();
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiCttfEhrpdDetachCnf_Deactivating_WaitHsmConnEstCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDetachFlag_Deactivating())
    {
        /* 停止本次建链流程保护定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_CONN_EST_CNF);

        CNAS_EHSM_SndCttfEhrpdDetachInd();

        CNAS_EHSM_ProcAbortMsg_Deactivating(EHSM_APS_CAUSE_TIME_OUT);
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvEhsmAbortReq_Deactivating_WaitConnEstRetry(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg    = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDeactReq = VOS_NULL_PTR;
    CNAS_EHSM_EHSM_ABORT_REQ_STRU      *pstRcvAbortMsg = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstRcvAbortMsg = (CNAS_EHSM_EHSM_ABORT_REQ_STRU *)pstMsg;
    pstEntryMsg    = CNAS_EHSM_GetCurrFsmEntryMsgAddr();
    pstPdnDeactReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);
    ucPdnId        = pstPdnDeactReq->ucPdnId;
    ucCid          = CNAS_EHSM_GetCidByPdnId(ucPdnId);

    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CONN_RETRY_EST);

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
    {
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

        CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);

        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);
    }

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDetachFlag_Deactivating())
    {
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF);

        CNAS_EHSM_SndCttfEhrpdDetachInd();

        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);
    }

    if (CNAS_BuildEventType(UEPS_PID_HSM, ID_HSM_EHSM_SUSPEND_IND) == pstRcvAbortMsg->ulEventType)
    {
        /*回复aps带suspend原因  退出状态机*/
        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_EHRPD_SUSPEND);
    }

    CNAS_EHSM_SndDeactivatingRslt();
    CNAS_EHSM_QuitFsmL2();
    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdPdnDiscCnf_Deactivating_WaitConnEstRetry(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_PDN_DISC_CNF_STRU  *pstPdnDiscCnf = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo    = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstPdnDiscCnf = (CTTF_CNAS_EHRPD_PDN_DISC_CNF_STRU *)pstMsg;
    ucPdnId       = pstPdnDiscCnf->ucPdnId;
    ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
    {
        /* 不再等待Retry定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CONN_RETRY_EST);

        /* 停止PDN去激活定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

        if (CNAS_CTTF_EHRPD_RSLT_FAILURE == pstPdnDiscCnf->enDiscRslt)
        {
            CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);
        }

        CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);

        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvCttfEhrpdPdnDiscInd_Deactivating_WaitConnEstRetry(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_EHRPD_PDN_DISC_IND_STRU  *pstPdnDiscInd = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo    = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    pstPdnDiscInd = (CTTF_CNAS_EHRPD_PDN_DISC_IND_STRU *)pstMsg;
    ucPdnId       = pstPdnDiscInd->ucPdnId;
    ucCid         = CNAS_EHSM_GetCidByPdnId(ucPdnId);
    pstPdnInfo    = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
    {
        /* 不再等待Retry定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CONN_RETRY_EST);

        /* 停止PDN去激活定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CTTF_PDN_DISC_CNF);

        CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);

        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvTiWaitCttfPdnDiscCnfExpired_Deactivating_WaitConnEstRetry(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg    = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDeactReq = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU *pstPdnInfo     = VOS_NULL_PTR;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;

    if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
    {
        pstEntryMsg    = CNAS_EHSM_GetCurrFsmEntryMsgAddr();
        pstPdnDeactReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);
        ucPdnId        = pstPdnDeactReq->ucPdnId;
        ucCid          = CNAS_EHSM_GetCidByPdnId(ucPdnId);
        pstPdnInfo     = &(CNAS_EHSM_GetEhsmCtxAddr()->astLocalPdnBearInfo[ucPdnId]);

        /* 不再等待Retry定时器 */
        CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_CONN_RETRY_EST);

        CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(ucPdnId);

        CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);

        CNAS_EHSM_ClearEhrpdLocalPdnInfo(ucPdnId);

        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

        CNAS_EHSM_SndDeactivatingRslt();

        CNAS_EHSM_QuitFsmL2();
    }

    return VOS_TRUE;
}


VOS_UINT32  CNAS_EHSM_ProcConnEstFail_Deactivating(
    EHSM_HSM_RSLT_ENUM_UINT32           enHsmEhsmRslt,
    VOS_UINT32                          ulIsTimeOutReason
)
{
    CNAS_EHSM_MSG_STRU                         *pstEntryMsg         = VOS_NULL_PTR;
    CNAS_EHSM_CTX_STRU                         *pstEhsmCtx          = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU           *pstPdnDeactivatReq  = VOS_NULL_PTR;
    CNAS_EHSM_DEACTIVATING_RETRY_INFO_STRU     *pstRetryInfo        = VOS_NULL_PTR;
    CNAS_EHSM_EHRPD_PDN_BEAR_INFO_STRU         *pstPdnInfo          = VOS_NULL_PTR;
    VOS_UINT32                                  ulIndex;
    VOS_UINT32                                  ulEhsmApsCause;
    VOS_UINT32                                  ulConnEstRetryFlag  = VOS_FALSE;
    VOS_UINT8                                   ucCid;
    VOS_UINT8                                   ucPdnId;

    pstEntryMsg    = CNAS_EHSM_GetCurrFsmEntryMsgAddr();
    pstEhsmCtx     = CNAS_EHSM_GetEhsmCtxAddr();
    pstRetryInfo   = CNAS_EHSM_GetRetryInfo_Deactivating();
    ulEhsmApsCause = CNAS_EHSM_MapHsmCauseToAps(enHsmEhsmRslt);

    if (pstEntryMsg->ulEventType == CNAS_BuildEventType(UEPS_PID_HSD, ID_HSD_EHSM_POWER_OFF_REQ))
    {
        if (VOS_TRUE == CNAS_EHSM_GetSendPdnDetachFlag_Deactivating())
        {
            CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_DETACH_CNF);
        }

        /* Stop Protect Timer for Power Off */
        if (CNAS_EHSM_TIMER_STATUS_RUNING == CNAS_EHSM_GetTimerStatus(TI_CNAS_EHSM_PROTECTING_POWER_OFF_PROCESS))
        {
            CNAS_EHSM_StopTimer(TI_CNAS_EHSM_PROTECTING_POWER_OFF_PROCESS);
        }

        /*clear local uesed pdn info and snd to ppp,aps,LNAS,do not need conn est retry*/
        ulIndex = 0;
        while (ulIndex < CNAS_EHSM_MAX_PDN_NUM)
        {
            if (VOS_TRUE == pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucInUsed)
            {
                ucPdnId = pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucPdnId;
                ucCid = pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucCid;
                CNAS_EHSM_SndApsPdnDeactivateInd(ucPdnId, ucCid, ulEhsmApsCause);
            }
            ulIndex++;
        }
        CNAS_EHSM_ClearAllEhrpdLocalPdnInfo();
        CNAS_EHSM_SndEsmClearAllNtf();
        CNAS_EHSM_SndCttfEhrpdDetachInd();
        CNAS_EHSM_SndHsdPowerOffCnf();

        CNAS_EHSM_ClearCtx(CNAS_EHSM_INIT_CTX_POWEROFF);

        CNAS_EHSM_SndDeactivatingRslt();
        CNAS_EHSM_QuitFsmL2();
        return VOS_TRUE;
    }
    else if (pstEntryMsg->ulEventType == CNAS_BuildEventType(WUEPS_PID_TAF, ID_APS_EHSM_PDN_DEACTIVATE_REQ))
    {
        /*if conn est time out,there is no need to retry*/
        ulConnEstRetryFlag = CNAS_EHSM_IsCommonConnRetryNeeded(enHsmEhsmRslt);

        if ((VOS_TRUE == ulConnEstRetryFlag)
         && (pstRetryInfo->usCurConnEstRetryTimes < pstEhsmCtx->stRetryCtrlInfo.stRetryConnEstInfo.usMaxNoOfRetry)
         && (ulIsTimeOutReason == VOS_FALSE))
        {
            CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_CONN_RETRY_EST, pstEhsmCtx->stRetryCtrlInfo.stRetryConnEstInfo.ulExpireTimerLen);
            CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_CONN_EST_RETRY_TIME_OUT);
        }
        else
        {
            if (VOS_TRUE == CNAS_EHSM_GetSendPdnDeactFlag_Deactivating())
            {
                CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_DISC_CNF);
            }

            /*retry times has over the max Retry num,just tell aps,ppp,LNAS,then quitfsmL2*/
            pstPdnDeactivatReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);
            pstPdnInfo = &(pstEhsmCtx->astLocalPdnBearInfo[pstPdnDeactivatReq->ucPdnId]);
            CNAS_EHSM_SndApsPdnDeactivateInd(pstPdnDeactivatReq->ucPdnId, pstPdnDeactivatReq->ucCid, ulEhsmApsCause);

            if (1 == CNAS_EHSM_GetLocalActivePdnConnNum())
            {
                CNAS_EHSM_SndCttfEhrpdDetachInd();
            }
            else
            {
                CNAS_EHSM_SndCttfEhrpdPdnLocalDiscInd(pstPdnDeactivatReq->ucPdnId);
            }

            CNAS_EHSM_SndEsmSyncEhrpdPdnInfoInd(pstPdnInfo, VOS_FALSE, EHSM_ESM_PDN_OPT_DISCONNECTED);
            CNAS_EHSM_ClearEhrpdLocalPdnInfo(pstPdnDeactivatReq->ucPdnId);
            CNAS_EHSM_SndDeactivatingRslt();
            CNAS_EHSM_QuitFsmL2();
            return VOS_TRUE;
        }
    }
    else
    {
    }

    return VOS_TRUE;
}

VOS_VOID CNAS_EHSM_IncreConntRetryTimes_Deactivating(VOS_VOID)
{
    CNAS_EHSM_DEACTIVATING_RETRY_INFO_STRU     *pstRetryInfo = VOS_NULL_PTR;

    /*update conn est retry time +1*/
    pstRetryInfo = CNAS_EHSM_GetRetryInfo_Deactivating();

    pstRetryInfo->usCurConnEstRetryTimes++;

    return;
}


VOS_VOID CNAS_EHSM_ResetConntRetryTimes_Deactivating(VOS_VOID)
{
    CNAS_EHSM_DEACTIVATING_RETRY_INFO_STRU     *pstRetryInfo = VOS_NULL_PTR;

    pstRetryInfo = CNAS_EHSM_GetRetryInfo_Deactivating();

    pstRetryInfo->usCurConnEstRetryTimes = 0;

    return;
}


VOS_VOID CNAS_EHSM_ProcAbortMsg_Deactivating(
    VOS_UINT32                          enCause
)
{
    CNAS_EHSM_CTX_STRU                 *pstEhsmCtx         = VOS_NULL_PTR;
    CNAS_EHSM_MSG_STRU                 *pstEntryMsg        = VOS_NULL_PTR;
    APS_EHSM_PDN_DEACTIVATE_REQ_STRU   *pstPdnDeactReq     = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucPdnId;
    VOS_UINT8                           ucPdnIdByCid;

    ucPdnIdByCid                        = 0;

    pstEhsmCtx  = CNAS_EHSM_GetEhsmCtxAddr();
    pstEntryMsg = CNAS_EHSM_GetCurrFsmEntryMsgAddr();

    /*在wait detach cnf过程中不会再次收到power off req消息，可能的打断为网测的detach和hsm conn disc*/
    if (CNAS_BuildEventType(UEPS_PID_HSD, ID_HSD_EHSM_POWER_OFF_REQ) == pstEntryMsg->ulEventType)
    {
        /*the situation of wait conn est time out is same as that conn est cnf fail*/
        for (ulIndex = 0; ulIndex < CNAS_EHSM_MAX_PDN_NUM; ulIndex++)
        {
            if (VOS_TRUE == pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucInUsed)
            {
                ucPdnId = pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucPdnId;
                ucCid   = pstEhsmCtx->astLocalPdnBearInfo[ulIndex].ucCid;

                CNAS_EHSM_SndApsPdnDeactivateInd(ucPdnId, ucCid, enCause);
            }
        }

        CNAS_EHSM_ClearAllEhrpdLocalPdnInfo();
        CNAS_EHSM_SndEsmClearAllNtf();

        /* Stop Protect Timer for Power Off */
        if (CNAS_EHSM_TIMER_STATUS_RUNING == CNAS_EHSM_GetTimerStatus(TI_CNAS_EHSM_PROTECTING_POWER_OFF_PROCESS))
        {
            CNAS_EHSM_StopTimer(TI_CNAS_EHSM_PROTECTING_POWER_OFF_PROCESS);
        }

        CNAS_EHSM_SndHsdPowerOffCnf();

        CNAS_EHSM_ClearCtx(CNAS_EHSM_INIT_CTX_POWEROFF);
    }
    else if (CNAS_BuildEventType(WUEPS_PID_TAF, ID_APS_EHSM_PDN_DEACTIVATE_REQ) == pstEntryMsg->ulEventType)
    {
        pstPdnDeactReq = (APS_EHSM_PDN_DEACTIVATE_REQ_STRU *)(pstEntryMsg->aucMsgBuffer);

        CNAS_CCB_SetCurrPsRatType(CNAS_CCB_PS_RATTYPE_NULL);

        ucPdnId = pstPdnDeactReq->ucPdnId;
        ucCid   = CNAS_EHSM_GetCidByPdnId(ucPdnId);

        CNAS_EHSM_SndApsPdnDeactivateCnf(ucPdnId, ucCid, EHSM_APS_CAUSE_SUCCESS);

        ucPdnIdByCid = CNAS_EHSM_GetPdnIdByCid(CNAS_EHSM_DEFAULT_CID);

        if (CNAS_EHSM_INVALID_PDN_ID != ucPdnIdByCid)
        {
            CNAS_EHSM_SndApsPdnDeactivateInd(ucPdnIdByCid,
                                             CNAS_EHSM_DEFAULT_CID,
                                             EHSM_APS_CAUSE_LAST_PDN);
        }

        CNAS_EHSM_ClearAllEhrpdLocalPdnInfo();

        CNAS_EHSM_SndEsmClearAllNtf();
    }
    else
    {
    }

    CNAS_EHSM_SndDeactivatingRslt();
    CNAS_EHSM_QuitFsmL2();

    return ;
}


VOS_UINT32 CNAS_EHSM_RcvApsDiscNotify_Deactivating_Init(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /*save entry msg*/
    CNAS_EHSM_SaveCurEntryMsg(ulEventType,pstMsg);

    if (VOS_TRUE == CNAS_EHSM_GetAirLinkExistFlag())
    {
        CNAS_EHSM_SetEhrpdState(CNAS_EHSM_EHRPD_DORMANT_STA);

        CNAS_EHSM_SndHsmDiscReq();

        CNAS_EHSM_SetCurrFsmState(CNAS_EHSM_DEACTIVATING_STA_WAIT_HSM_DISC_CNF);

        CNAS_EHSM_StartTimer(TI_CNAS_EHSM_WAIT_HSM_DISC_CNF, TI_CNAS_EHSM_WAIT_HSM_DISC_CNF_LEN);
    }
    else
    {
        CNAS_EHSM_QuitFsmL2();
    }

    return VOS_TRUE;
}


VOS_UINT32 CNAS_EHSM_RcvHsmDiscCnf_Deactivating_WaitHsmDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_DISC_CNF);

    /* 更新空口链路状态 */
    CNAS_EHSM_SetAirLinkExistFlag(VOS_FALSE);

    CNAS_EHSM_SndDeactivatingRslt();

    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}

VOS_UINT32 CNAS_EHSM_RcvEhsmAbortReq_Deactivating_WaitHsmDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_StopTimer(TI_CNAS_EHSM_WAIT_HSM_DISC_CNF);

    CNAS_EHSM_SetAirLinkExistFlag(VOS_FALSE);

    CNAS_EHSM_SndDeactivatingRslt();

    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}

VOS_UINT32 CNAS_EHSM_RcvTiWaitHsmDiscCnf_Deactivating_WaitHsmDiscCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_EHSM_SetAirLinkExistFlag(VOS_FALSE);

    CNAS_EHSM_SndDeactivatingRslt();

    CNAS_EHSM_QuitFsmL2();

    return VOS_TRUE;
}

#endif /* FEATURE_UE_MODE_CDMA */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


