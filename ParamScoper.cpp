#include "ParamScoper.h"

#include "AE_Macros.h"

#include <glm/glm.hpp>

using namespace glm;

ParamScoper::ParamScoper(const PF_InData * inData, PF_ParamIndex index, A_long time, A_long timeStep, A_u_long timeScale)
{
    this->inData = inData;
    this->err = PF_CHECKOUT_PARAM(inData, index, time, timeStep, timeScale, &this->param);
}

ParamScoper::~ParamScoper()
{
    this->err = PF_CHECKIN_PARAM(this->inData, &this->param);
}

PF_FpLong ParamScoper::GetAngle()
{
    return FIX_2_FLOAT(this->param.u.ad.value);
}

A_long ParamScoper::GetLong()
{
    return this->param.u.sd.value;
}

PF_FpLong ParamScoper::GetFloat()
{
    return this->param.u.fs_d.value;
}

glm::vec3 ParamScoper::GetPosition()
{
    PF_RationalScale xScale = this->inData->downsample_x;
    PF_RationalScale yScale = this->inData->downsample_y;
    PF_FpShort xr = 1.0 / (xScale.num / (PF_FpShort)xScale.den);
    PF_FpShort yr = 1.0 / (yScale.num / (PF_FpShort)yScale.den);
    return glm::vec3(this->param.u.point3d_d.x_value * xr, this->param.u.point3d_d.y_value * yr, this->param.u.point3d_d.z_value * yr);
}

A_long ParamScoper::GetPopup()
{
    return this->param.u.pd.value;
}

A_Boolean ParamScoper::GetBoolean()
{
    return (A_Boolean)this->param.u.bd.value;
}

glm::vec3 ParamScoper::GetColor()
{
    return glm::vec3(
        this->param.u.cd.value.red / (PF_FpShort)PF_MAX_CHAN8,
        this->param.u.cd.value.green / (PF_FpShort)PF_MAX_CHAN8,
        this->param.u.cd.value.blue / (PF_FpShort)PF_MAX_CHAN8
    );
}

PF_FpLong ParamScoper::GetDefaultFloat()
{
    return this->param.u.fs_d.dephault;
}

ParamScoper3DProps::ParamScoper3DProps(const PF_InData* inData, PF_ParamIndex index[3], A_long time, A_long timeStep, A_u_long timeScale)
{
    this->inData = inData;
    this->err = PF_CHECKOUT_PARAM(inData, index[0], time, timeStep, timeScale, &this->param[0]);
    this->err = PF_CHECKOUT_PARAM(inData, index[1], time, timeStep, timeScale, &this->param[1]);
    this->err = PF_CHECKOUT_PARAM(inData, index[2], time, timeStep, timeScale, &this->param[2]);
}

ParamScoper3DProps::~ParamScoper3DProps()
{
    this->err = PF_CHECKIN_PARAM(this->inData, &this->param[0]);
    this->err = PF_CHECKIN_PARAM(this->inData, &this->param[1]);
    this->err = PF_CHECKIN_PARAM(this->inData, &this->param[2]);
}

glm::vec3 ParamScoper3DProps::GetPosition()
{
    return vec3(
        this->param[0].u.fs_d.value,
        this->param[1].u.fs_d.value,
        this->param[2].u.fs_d.value
    );
}

PF_FpShort ParamScoper3DProps::GetPositionX()
{
    return this->param[0].u.fs_d.value;
}

PF_FpShort ParamScoper3DProps::GetPositionY()
{
    return this->param[1].u.fs_d.value;
}

PF_FpShort ParamScoper3DProps::GetPositionZ()
{
    return this->param[2].u.fs_d.value;
}

glm::vec3 ParamScoper3DProps::GetRotate()
{
    return vec3(
        FIX_2_FLOAT(this->param[0].u.ad.value),
        FIX_2_FLOAT(this->param[1].u.ad.value),
        FIX_2_FLOAT(this->param[2].u.ad.value)
    );
}

PF_FpShort ParamScoper3DProps::GetRotateX()
{
    return FIX_2_FLOAT(this->param[0].u.ad.value);
}

PF_FpShort ParamScoper3DProps::GetRotateY()
{
    return FIX_2_FLOAT(this->param[1].u.ad.value);
}

PF_FpShort ParamScoper3DProps::GetRotateZ()
{
    return FIX_2_FLOAT(this->param[2].u.ad.value);
}
