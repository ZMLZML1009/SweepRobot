/********************************************************************************
 * This code is used for robot control
*********************************************************************************
 * Author        Data            Version
 * Liren         2018/12/16      1.0
********************************************************************************/
#include <string.h>

#include "tim.h"
#include "usart.h"
#include "inv_mpu.h"

#include "RocLog.h"
#include "RocLed.h"
#include "RocServo.h"
#include "RocMotor.h"
#include "RocBeeper.h"
#include "RocBattery.h"
#include "RocPca9685.h"
#include "RocMpu6050.h"
#include "RocBluetooth.h"
#include "RocRemoteControl.h"
#include "RocRobotControl.h"
#include "RocRobotDhAlgorithm.h"



static ROC_ROBOT_CONTROL_s      *g_pRocRobotCtrl = NULL;
static ROC_ROBOT_CTRL_FlAG_s    g_RocRobotCtrlFlag = {0};
static ROC_REMOTE_CTRL_INPUT_s  g_RocRobotRemoteCtrlInput = {0};
static ROC_ROBOT_WALK_MODE_e    g_RocRobotWalkModeStatus = ROC_ROBOT_WALK_MODE_HEXAPOD;


/*********************************************************************************
 *  Description:
 *              Set the robot walk mode
 *
 *  Parameter:
 *              WalkMode: the expected robot walk mode
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static void RocRobotWalkModeSet(ROC_ROBOT_WALK_MODE_e WalkMode)
{
    g_RocRobotWalkModeStatus = WalkMode;
}

/*********************************************************************************
 *  Description:
 *              Get the robot walk mode
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              The current robot walk mode
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static uint32_t RocRobotWalkModeGet(void)
{
    return g_RocRobotWalkModeStatus;
}

/*********************************************************************************
 *  Description:
 *              Set the robot control flag
 *
 *  Parameter:
 *              FlagNum: the control flag number to be setted
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.04.17)
**********************************************************************************/
static void RocRobotCtrlFlagSet(uint8_t FlagNum)
{
    uint8_t i = 0;

    for(i = 0; i < ROC_ROBOT_CTRL_CMD_NUM; i++)
    {
        if(FlagNum == i)
        {
            g_RocRobotCtrlFlag.CtrlFlag[i] = ROC_TRUE;
        }
        else
        {
            g_RocRobotCtrlFlag.CtrlFlag[i] = ROC_FALSE;
        }
    }
}

/*********************************************************************************
 *  Description:
 *              Get the robot control flag
 *
 *  Parameter:
 *              FlagNum: the control flag number to be getted
 *
 *  Return:
 *              The value of the control flag
 *
 *  Author:
 *              ROC LiRen(2019.04.17)
**********************************************************************************/
static uint8_t RocRobotCtrlFlagGet(uint8_t FlagNum)
{
    return g_RocRobotCtrlFlag.CtrlFlag[FlagNum];
}

/*********************************************************************************
 *  Description:
 *              Get current robot IMU euler angle
 *
 *  Parameter:
 *              *ImuDat: the pointer to the robot IMU data structure
 *
 *  Return:
 *              The PCA9685 driver init status
 *
 *  Author:
 *              ROC LiRen(2019.04.15)
**********************************************************************************/
static ROC_RESULT RocRobotImuEulerAngleGet(ROC_ROBOT_IMU_DATA_s *ImuDat)
{
    ROC_RESULT Ret = RET_OK;

    Ret = RocMpu6050EulerAngleGet(&ImuDat->Pitch, &ImuDat->Roll, &ImuDat->Yaw);
    if(RET_OK != Ret)
    {
        //ROC_LOGE("Robot get the IMU angle is in error(%d)! Be careful!", Ret); /* For the FIFO overflow problem */
    }

    return Ret;
}

/*********************************************************************************
 *  Description:
 *              Start the measure of robot sensor
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.15)
**********************************************************************************/
static void RocRobotSensorMeasure(void)
{
    if(ROC_ROBOT_CTRL_MEASURE_START == RocBluetoothCtrlCmd_Get())
    {
      
    }
    else if(ROC_ROBOT_CTRL_MEASURE_STOP == RocBluetoothCtrlCmd_Get())
    {

    }
}

/*********************************************************************************
 *  Description:
 *              Robot remote control function
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static void RocRobotRemoteControl(void)
{
    uint8_t RobotCtrlCmd;

    RobotCtrlCmd = RocBluetoothCtrlCmd_Get();

    switch(RobotCtrlCmd)
    {
        case ROC_ROBOT_CTRL_CMD_MOSTAND:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = 0;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = 0;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_STANDING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(0))
                {
                    RocRobotCtrlFlagSet(0);
                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_FORWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_WALKING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(1))
                {
                    RocRobotCtrlFlagSet(1);

                    g_pRocRobotCtrl->CurState.RefImuAngle = g_pRocRobotCtrl->CurState.CurImuAngle;
                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_BAKWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = -ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_WALKING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(2))
                {
                    RocRobotCtrlFlagSet(2);

                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_LFCLOCK:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = 0;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = ROC_ROBOT_DEFAULT_TURN_ANGLE;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_CIRCLING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(3))
                {
                    RocRobotCtrlFlagSet(3);
                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_RGCLOCK:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = 0;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = -ROC_ROBOT_DEFAULT_TURN_ANGLE;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_CIRCLING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(4))
                {
                    RocRobotCtrlFlagSet(4);
                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_LFRWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = -ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Y = ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_WALKING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(5))
                {
                    RocRobotCtrlFlagSet(5);


                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_RFRWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Y = ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_WALKING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(6))
                {
                    RocRobotCtrlFlagSet(6);


                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_LBKWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = -ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Y = -ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_WALKING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(7))
                {
                    RocRobotCtrlFlagSet(7);


                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_RBKWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Y = -ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;

                RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_WALKING);

                if(ROC_FALSE == RocRobotCtrlFlagGet(8))
                {
                    RocRobotCtrlFlagSet(8);


                }
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_TURNLDR:
        {
            break;
        }

        case ROC_ROBOT_CTRL_CMD_TURNRDR:
        {
            break;
        }

        case ROC_ROBOT_CTRL_CMD_PARAMET:
        {
            break;
        }

        case ROC_ROBOT_CTRL_MEASURE_START:
        {
            RocRobotSensorMeasure();
            break;
        }

        default:
        {
            break;
        }
    }
}

/*********************************************************************************
 *  Description:
 *              Check robot control command is changed
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              The running state
 *
 *  Author:
 *              ROC LiRen(2019.04.06)
**********************************************************************************/
static ROC_RESULT RocRobotCtrlCmdIsChanged(void)
{
    uint8_t         CurCtrlCmd = ROC_NONE;
    static uint8_t  LastCtrlCmd = ROC_NONE;

    CurCtrlCmd = RocBluetoothCtrlCmd_Get();

    if(LastCtrlCmd != CurCtrlCmd)
    {
        LastCtrlCmd = CurCtrlCmd;

        return ROC_TRUE;
    }
    else
    {
        return ROC_FALSE;
    }
}

/*********************************************************************************
 *  Description:
 *              Switch the move context to forbid mutation action
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.04.06)
**********************************************************************************/
static void RocRobotMoveContextSwitch(ROC_ROBOT_CONTROL_s *pRobotCtrl)
{
    uint8_t i = 0;

    for(i = 0; i < ROC_ROBOT_CNT_LEGS; i++)
    {
        pRobotCtrl->CurState.LegCurPos[i].X = pRobotCtrl->CurState.TravelLength.X / 2;
        pRobotCtrl->CurState.LegCurPos[i].Y = pRobotCtrl->CurState.TravelLength.Y / 2;
        pRobotCtrl->CurState.LegCurPos[i].Z = pRobotCtrl->CurState.TravelLength.Z / 2;
        pRobotCtrl->CurState.LegCurPos[i].A = pRobotCtrl->CurState.TravelLength.A / 2;
    }
}

/*********************************************************************************
 *  Description:
 *              Robot move core
 *
 *  Parameter:
 *              pRobotCtrl: the pointer to the robot control structure
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.03.31)
**********************************************************************************/
static void RocRobotMoveCtrlCore(ROC_ROBOT_CONTROL_s *pRobotCtrl)
{
    ROC_RESULT                  ChangeStatus = ROC_NONE;
    ROC_ROBOT_MOVE_STATUS_e     MoveStatus = ROC_ROBOT_MOVE_STATUS_NUM;

    RocRobotCtrlDeltaMoveCoorInput(g_RocRobotRemoteCtrlInput.X, g_RocRobotRemoteCtrlInput.Y,
                                   g_RocRobotRemoteCtrlInput.Z, g_RocRobotRemoteCtrlInput.A,
                                   g_RocRobotRemoteCtrlInput.H);

    MoveStatus = RocRobotMoveStatus_Get();

    switch(MoveStatus)
    {
        case ROC_ROBOT_MOVE_STATUS_POWER_ON:
        {
            RocRobotSingleLegCtrl(&pRobotCtrl->CurServo);

            break;
        }

        case ROC_ROBOT_MOVE_STATUS_WALKING:
        {
            ChangeStatus = RocRobotCtrlCmdIsChanged();
            if(ROC_TRUE == ChangeStatus)
            {
               RocRobotMoveContextSwitch(pRobotCtrl);
            }

            RocRobotGaitSeqUpdate();

#ifdef ROC_ROBOT_CLOSED_LOOP_CONTROL
            RocRobotClosedLoopWalkCalculate(&pRobotCtrl->CurServo);
#else
            RocRobotOpenLoopWalkCalculate(&pRobotCtrl->CurServo);
#endif

            break;
        }

        case ROC_ROBOT_MOVE_STATUS_CIRCLING:
        {
            ChangeStatus = RocRobotCtrlCmdIsChanged();
            if(ROC_TRUE == ChangeStatus)
            {
               RocRobotMoveContextSwitch(pRobotCtrl);
            }

            RocRobotGaitSeqUpdate();

            RocRobotOpenLoopCircleCalculate(&pRobotCtrl->CurServo);

            break;
        }

        default:
        {
            break;
        }
    }

    //RocServoSpeedSet(g_pRocRobotCtrl->CurGait.NomGaitSpeed);
}

/*********************************************************************************
 *  Description:
 *              Robot run special gait sequence when power on
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.04.06)
**********************************************************************************/
static void RocRobotPowerOnGaitSeq_Run(void)
{
    ROC_ROBOT_LEG_e SelectNum = ROC_ROBOT_CNT_LEGS;

    g_RocRobotRemoteCtrlInput.X = 0;
    g_RocRobotRemoteCtrlInput.Y = 0;
    g_RocRobotRemoteCtrlInput.Z = ROC_ROBOT_DEFAULT_FEET_LIFT;
    g_RocRobotRemoteCtrlInput.A = 0;
    g_RocRobotRemoteCtrlInput.H = 0;
    HAL_Delay(ROC_ROBOT_RUN_SPEED_POWER_ON);

    g_RocRobotRemoteCtrlInput.X = 0;
    g_RocRobotRemoteCtrlInput.Y = 0;
    g_RocRobotRemoteCtrlInput.Z = ROC_ROBOT_DEFAULT_FEET_LIFT * 1.8;
    g_RocRobotRemoteCtrlInput.A = 0;
    g_RocRobotRemoteCtrlInput.H = 0;
    HAL_Delay(ROC_ROBOT_RUN_SPEED_POWER_ON);

    g_RocRobotRemoteCtrlInput.X = 0;
    g_RocRobotRemoteCtrlInput.Y = 0;
    g_RocRobotRemoteCtrlInput.Z = ROC_ROBOT_DEFAULT_FEET_LIFT;
    g_RocRobotRemoteCtrlInput.A = 0;
    g_RocRobotRemoteCtrlInput.H = 0;
    HAL_Delay(ROC_ROBOT_RUN_SPEED_POWER_ON);

    g_RocRobotRemoteCtrlInput.X = 0;
    g_RocRobotRemoteCtrlInput.Y = 0;
    g_RocRobotRemoteCtrlInput.Z = 0;
    g_RocRobotRemoteCtrlInput.A = 0;
    g_RocRobotRemoteCtrlInput.H = 0;
    HAL_Delay(ROC_ROBOT_RUN_SPEED_POWER_ON);

    for(SelectNum = ROC_ROBOT_RIG_FRO_LEG; SelectNum < ROC_ROBOT_CNT_LEGS; SelectNum++)
    {
        RocRobotSingleLegSelect(SelectNum);
        HAL_Delay(ROC_ROBOT_RUN_SPEED_POWER_ON);
    }

    RocRobotSingleLegSelect(ROC_ROBOT_CNT_LEGS);
    HAL_Delay(ROC_ROBOT_RUN_SPEED_POWER_ON);

}

/*********************************************************************************
 *  Description:
 *              Robot control init
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static ROC_RESULT RocRobotControlInit(void)
{
    ROC_RESULT Ret = RET_OK;

    g_pRocRobotCtrl = RocRobotCtrlInfoGet();

    RocRobotOpenLoopWalkCalculate(&g_pRocRobotCtrl->CurServo);

    Ret = RocServoInit((int16_t *)(&g_pRocRobotCtrl->CurServo));
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");
    }

    return Ret;
}

/*********************************************************************************
 *  Description:
 *              Robot start running
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static ROC_RESULT RocRobotStartRun(void)
{
    ROC_RESULT Ret = RET_OK;

    RocRobotWalkModeSet(ROC_ROBOT_WALK_MODE_HEXAPOD);

    RocRobotSingleLegSelect(ROC_ROBOT_CNT_LEGS);

    g_pRocRobotCtrl->CurGait.NomGaitSpeed = ROC_ROBOT_RUN_SPEED_POWER_ON;
    RocServoSpeedSet(g_pRocRobotCtrl->CurGait.NomGaitSpeed);

    RocRobotPowerOnGaitSeq_Run();

    g_pRocRobotCtrl->CurGait.NomGaitSpeed = ROC_ROBOT_RUN_SPEED_DEFAULT;
    RocServoSpeedSet(g_pRocRobotCtrl->CurGait.NomGaitSpeed);

    RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_STANDING);

    return Ret;
}

/*********************************************************************************
 *  Description:
 *              Robot start running
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static ROC_RESULT RocRobotStopRun(void)
{
    ROC_RESULT Ret = RET_OK;

    RocServoSpeedSet(0);

    Ret = RocServoTimerStop();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot stop servo motor in error!");
        return Ret;
    }

    while(1)
    {
        RocBeeperBlink(4, 800);
    }
}

/*********************************************************************************
 *  Description:
 *              Robot control control init
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.15)
**********************************************************************************/
void RocRobotInit(void)
{
    ROC_RESULT Ret = RET_OK;

    ROC_LOGW("############# Robot hardware version is 0.5! #############");

    RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_POWER_ON);

    Ret = RocLedInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocBluetoothInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocPca9685Init();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocBatteryInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    //Ret = RocZmod4410Init();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocMotorInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    //Ret = RocRemoteControlInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocBeeperInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");
    
        while(1);
    }

    Ret = RocRobotAlgoCtrlInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");
    
        while(1);
    }

    Ret = RocRobotControlInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");
    
        while(1);
    }

    Ret = RocMpu6050Init();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    ROC_LOGI("Robot hardware init is in success, and the system start running.");

    Ret = RocRobotStartRun();
    if(RET_OK == Ret)
    {
        ROC_LOGW("############# Robot is running! Be careful! #############");
    }

    RocRobotMoveStatus_Set(ROC_ROBOT_MOVE_STATUS_STANDING);
}

/*********************************************************************************
 *  Description:
 *              Robot power on control task entry
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.04.10)
**********************************************************************************/
static void RocRobotPowerOnTaskEntry(void)
{
    if(ROC_TRUE == RocServoTurnIsFinshed())
    {
        RocRobotMoveCtrlCore(g_pRocRobotCtrl);
    }

    RocServoControl((int16_t *)(&g_pRocRobotCtrl->CurServo));
}

/*********************************************************************************
 *  Description:
 *              Robot battery check task entry
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.04.10)
**********************************************************************************/
static void RocBatteryCheckTaskEntry(void)
{
    if(ROC_TRUE == g_pRocRobotCtrl->CurState.BatTimeIsReady)
    {
        g_pRocRobotCtrl->CurState.BatTimeIsReady = ROC_FALSE;

        RocBatteryVoltageAdcSample();
    }

#ifndef ROC_ROBOT_GAIT_DEBUG
    if(ROC_ROBOT_BATTERY_LIMITED_VOLTATE > RocBatteryVoltageGet())
    {
        ROC_LOGN("Battery is in low electricity! Charge it!");
        RocRobotStopRun();
    }
#endif

#ifdef ROC_ROBOT_SENSOR_MEASURE
    {
        RocRobotSensorMeasure();
    }
#endif
}

/*********************************************************************************
 *  Description:
 *              Robot control task entry
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.04.10)
**********************************************************************************/
static void RocRobotCtrlTaskEntry(void)
{
    uint32_t CurrentExecutionTime = 0;
    static uint32_t LastExecutionTime = 0;

    if(ROC_TRUE == g_pRocRobotCtrl->CurState.CtrlTimeIsReady)
    {
        RocLedToggle();

        RocRobotRemoteControl();

        if(ROC_TRUE == RocServoTurnIsFinshed())
        {
            RocRobotMoveCtrlCore(g_pRocRobotCtrl);
        }

        RocServoControl((int16_t *)(&g_pRocRobotCtrl->CurServo));

#ifdef ROC_ROBOT_GAIT_DEBUG
        //ROC_LOGN("Exetime: %d \r\n", CurrentExecutionTime - LastExecutionTime);
#endif

        LastExecutionTime = CurrentExecutionTime;
        LastExecutionTime = LastExecutionTime;

        g_pRocRobotCtrl->CurState.CtrlTimeIsReady = ROC_FALSE;
    }
    else
    {
#ifdef ROC_ROBOT_CLOSED_LOOP_CONTROL
        RocRobotImuEulerAngleGet(&g_pRocRobotCtrl->CurState.CurImuAngle);
        HAL_Delay(10);

#ifdef ROC_ROBOT_GAIT_DEBUG
        ROC_LOGI("Pitch: %.2f, Roll: %.2f, Yaw: %.2f",  g_pRocRobotCtrl->CurState.CurImuAngle.Pitch,
                                                        g_pRocRobotCtrl->CurState.CurImuAngle.Roll,
                                                        g_pRocRobotCtrl->CurState.CurImuAngle.Yaw);
#endif

#endif
    }
}

/*********************************************************************************
 *  Description:
 *              The interrupt service handle for timer
 *
 *  Parameter:
 *              *htim: the point of the interrupt timer
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    ROC_ROBOT_MOVE_STATUS_e     MoveStatus;

    if(TIM6 == htim->Instance)
    {
        MoveStatus = RocRobotMoveStatus_Get();

        if(ROC_ROBOT_MOVE_STATUS_POWER_ON == MoveStatus)
        {
            RocRobotPowerOnTaskEntry();
        }
        else
        {
            g_pRocRobotCtrl->CurState.CtrlTimeIsReady = ROC_TRUE;
        }
    }
    else if(TIM7 == htim->Instance)
    {
        g_pRocRobotCtrl->CurState.BatTimeIsReady = ROC_TRUE;
    }
}

/*********************************************************************************
 *  Description:
 *              Robot control control while main
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.15)
**********************************************************************************/
void RocRobotMain(void)
{
    RocRobotCtrlTaskEntry();

    RocBatteryCheckTaskEntry();

    RocBluetoothRecvIsFinshed();
}

