#include "VolumetricFractalNoise.h"

typedef struct {
    A_u_long index;
    A_char str[256];
} TableString;

TableString g_strs[StrID_NUMTYPES] = {
    StrID_NONE, "",
    StrID_Name, "Volumetric Fractal Noise",
    StrID_Description, "Generate Volumetric Fractal Noise\rCopyright 2017 mes.",

    // noise properties
    StrID_Noise_Param_Group_Name, "Noise",
    StrID_Noise_Color_Param_Name, "Color",
    StrID_Noise_Contrast_Param_Name, "Contrast",
    StrID_Noise_Density_Param_Name, "Density",
    StrID_Noise_Transform_Param_Group_Name, "Transform",
    StrID_Noise_Transform_Position_Param_Name, "Position",
    StrID_Noise_Transform_ScaleX_Param_Name, "Scale X",
    StrID_Noise_Transform_ScaleY_Param_Name, "Scale Y",
    StrID_Noise_Transform_ScaleZ_Param_Name, "Scale Z",
    StrID_Noise_Transform_RotateX_Param_Name, "Rotate X",
    StrID_Noise_Transform_RotateY_Param_Name, "Rotate Y",
    StrID_Noise_Transform_RotateZ_Param_Name, "Rotate Z",
    StrID_Noise_Octave_Param_Name, "Octave",
    StrID_Noise_Evolution_Param_Name, "Evolution",

    // fall off properties
    StrID_FallOff_Param_Group_Name, "Fall off",
    StrID_FallOff_SizeX_Param_Name, "Size X",
    StrID_FallOff_SizeY_Param_Name, "Size Y",
    StrID_FallOff_SizeZ_Param_Name, "Size Z",
    StrID_FallOff_Power_Param_Name, "Power",

    // box properties
    StrID_Geometry_Param_Group_Name, "Geometry",
    StrID_Geometry_Position_Param_Name, "Position",
    StrID_Geometry_SizeX_Param_Name, "Size X",
    StrID_Geometry_SizeY_Param_Name, "Size Y",
    StrID_Geometry_SizeZ_Param_Name, "Size Z",
    StrID_Geometry_RotateX_Param_Name, "Rotate X",
    StrID_Geometry_RotateY_Param_Name, "Rotate Y",
    StrID_Geometry_RotateZ_Param_Name, "Rotate Z",

    // render properties
    StrID_Render_Param_Group_Name, "Render",
    StrID_Render_CastResolution_Params_Name, "Cast Resolution",
    StrID_Render_DistanceAttenuation_Params_Name, "Distance Attenuation"
};

char *GetStringPtr(int strNum)
{
    return g_strs[strNum].str;
}
