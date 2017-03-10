#pragma once

typedef enum {
    StrID_NONE, 
    StrID_Name,
    StrID_Description,

    // noise properties
    StrID_Noise_Param_Group_Name,
    StrID_Noise_Color_Param_Name,
    StrID_Noise_Contrast_Param_Name,
    StrID_Noise_Density_Param_Name,
    StrID_Noise_Transform_Param_Group_Name,
    StrID_Noise_Transform_Position_Param_Name,
    StrID_Noise_Transform_ScaleX_Param_Name,
    StrID_Noise_Transform_ScaleY_Param_Name,
    StrID_Noise_Transform_ScaleZ_Param_Name,
    StrID_Noise_Transform_RotateX_Param_Name,
    StrID_Noise_Transform_RotateY_Param_Name,
    StrID_Noise_Transform_RotateZ_Param_Name,
    StrID_Noise_Octave_Param_Name,
    StrID_Noise_Evolution_Param_Name,

    // fall off properties
    StrID_FallOff_Param_Group_Name,
    StrID_FallOff_SizeX_Param_Name,
    StrID_FallOff_SizeY_Param_Name,
    StrID_FallOff_SizeZ_Param_Name,
    StrID_FallOff_Power_Param_Name,

    // box properties
    StrID_Geometry_Param_Group_Name,
    StrID_Geometry_Position_Param_Name,
    StrID_Geometry_SizeX_Param_Name,
    StrID_Geometry_SizeY_Param_Name,
    StrID_Geometry_SizeZ_Param_Name,
    StrID_Geometry_RotateX_Param_Name,
    StrID_Geometry_RotateY_Param_Name,
    StrID_Geometry_RotateZ_Param_Name,

    // render properties
    StrID_Render_Param_Group_Name,
    StrID_Render_CastResolution_Params_Name,
    StrID_Render_DistanceAttenuation_Params_Name,

    StrID_NUMTYPES
} StrIDType;
