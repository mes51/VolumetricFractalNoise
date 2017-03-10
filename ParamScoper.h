#ifndef PARAM_SCOPER_H
#define PARAM_SCOPER_H

#include <AE_Effect.h>
#include <AE_EffectCB.h>
#include <SPBasic.h>
#include <SPSuites.h>

#include <glm/glm.hpp>

class ParamScoper
{
public:
    ParamScoper(const PF_InData* inData, PF_ParamIndex index, A_long time, A_long timeStep, A_u_long timeScale);
    ParamScoper(const PF_InData* inData, PF_ParamIndex index) : ParamScoper(inData, index, inData->current_time, inData->time_step, inData->time_scale) { }
    ~ParamScoper();

    PF_FpLong GetAngle();
    A_long GetLong();
    PF_FpLong GetFloat();
    glm::vec3 GetPosition();
    A_long GetPopup();
    A_Boolean GetBoolean();
    glm::vec3 GetColor();

    PF_FpLong GetDefaultFloat();

private:
    const PF_InData* inData;
    PF_ParamDef param;
    PF_Err err;
};

class ParamScoper3DProps
{
public:
    ParamScoper3DProps(const PF_InData* inData, PF_ParamIndex index[3], A_long time, A_long timeStep, A_u_long timeScale);
    ParamScoper3DProps(const PF_InData* inData, PF_ParamIndex index[3]) : ParamScoper3DProps(inData, index, inData->current_time, inData->time_step, inData->time_scale) { }
    ~ParamScoper3DProps();

    glm::vec3 GetPosition();
    PF_FpShort GetPositionX();
    PF_FpShort GetPositionY();
    PF_FpShort GetPositionZ();

    glm::vec3 GetRotate();
    PF_FpShort GetRotateX();
    PF_FpShort GetRotateY();
    PF_FpShort GetRotateZ();

private:
    const PF_InData* inData;
    PF_ParamDef param[3];
    PF_Err err;
};

#endif // !PARAM_SCOPER_H