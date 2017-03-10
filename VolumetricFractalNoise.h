#ifndef VFN_H
#define VFN_H

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned short u_int16;
typedef unsigned long u_long;
typedef short int int16;
typedef float fpshort;

#define PF_TABLE_BITS 12
#define PF_TABLE_SZ_16 4096

#define PF_DEEP_COLOR_AWARE 1

#include "AEConfig.h"

#ifdef AE_OS_WIN
    typedef unsigned short PixelType;
    #include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include <glbinding/gl33core/gl.h>

#include "VolumetricFractalNoise_Strings.h"

#define PF_ADD_POINT_3D_FIX_Z(NAME, X_DFLT, Y_DFLT, Z_DFLT, ID) \
	do	{\
		AEFX_CLR_STRUCT(def); \
		PF_Err	priv_err = PF_Err_NONE; \
		def.param_type = PF_Param_POINT_3D; \
		PF_STRCPY(def.name, (NAME) ); \
		def.u.point3d_d.x_value = def.u.point3d_d.x_dephault = X_DFLT; \
		def.u.point3d_d.y_value = def.u.point3d_d.y_dephault = Y_DFLT; \
		def.u.point3d_d.z_value = def.u.point3d_d.z_dephault = Z_DFLT; \
		def.uu.id = (ID); \
		if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
	} while (0)

/* Versioning information */

#define	MAJOR_VERSION 1
#define	MINOR_VERSION 0
#define	BUG_VERSION 0
#define	STAGE_VERSION PF_Stage_DEVELOP
#define	BUILD_VERSION 1

#define VFN_POSITION_CENTER 50
#define VFN_RESTRICT_BOUNDS 0

enum
{
    VFN_INPUT = 0,
    
    // noise properties
    VFN_NOISE_GROUP_BEGIN,
    VFN_NOISE_COLOR,
    VFN_NOISE_CONTRAST,
    VFN_NOISE_DENSITY,
    VFN_NOISE_TRANSFORM_GROUP_BEGIN,
    VFN_NOISE_TRANSFORM_POSITION,
    VFN_NOISE_TRANSFORM_SCALE_X,
    VFN_NOISE_TRANSFORM_SCALE_Y,
    VFN_NOISE_TRANSFORM_SCALE_Z,
    VFN_NOISE_TRANSFORM_ROTATE_X,
    VFN_NOISE_TRANSFORM_ROTATE_Y,
    VFN_NOISE_TRANSFORM_ROTATE_Z,
    VFN_NOISE_TRANSFORM_GROUP_END,
    VFN_NOISE_OCTAVE,
    VFN_NOISE_EVOLUTION,
    VFN_NOISE_GROUP_END,

    // fall off properties
    VFN_FALLOFF_GROUP_BEGIN,
    VFN_FALLOFF_SIZE_X,
    VFN_FALLOFF_SIZE_Y,
    VFN_FALLOFF_SIZE_Z,
    VFN_FALLOFF_POWER,
    VFN_FALLOFF_GROUP_END,

    // box properties
    VFN_GEOMETRY_GROUP_BEGIN,
    VFN_GEOMETRY_POSITION,
    VFN_GEOMETRY_SIZE_X,
    VFN_GEOMETRY_SIZE_Y,
    VFN_GEOMETRY_SIZE_Z,
    VFN_GEOMETRY_ROTATE_X,
    VFN_GEOMETRY_ROTATE_Y,
    VFN_GEOMETRY_ROTATE_Z,
    VFN_GEOMETRY_GROUP_END,

    // render properties
    VFN_RENDER_GROUP_BEGIN,
    VFN_CAST_RESOLUTION,
    VFN_DISTANCE_ATTENUATION,
    VFN_RENDER_GROUP_END,

    VFN_NUM_PARAMS
};

enum
{
    // noise properties
    NOISE_GROUP_BEGIN_DISK_ID = 1,
    NOISE_COLOR_DISK_ID,
    NOISE_CONTRAST_DISK_ID,
    NOISE_DENSITY_DISK_ID,
    NOISE_TRANSFORM_GROUP_BEGIN_DISK_ID,
    NOISE_TRANSFORM_POSITION_DISK_ID,
    NOISE_TRANSFORM_SCALE_X_DISK_ID,
    NOISE_TRANSFORM_SCALE_Y_DISK_ID,
    NOISE_TRANSFORM_SCALE_Z_DISK_ID,
    NOISE_TRANSFORM_ROTATE_X_DISK_ID,
    NOISE_TRANSFORM_ROTATE_Y_DISK_ID,
    NOISE_TRANSFORM_ROTATE_Z_DISK_ID,
    NOISE_TRANSFORM_GROUP_END_DISK_ID,
    NOISE_OCTAVE_DISK_ID,
    NOISE_EVOLUTION_DISK_ID,
    NOISE_GROUP_END_DISK_ID,

    // fall off properties
    FALLOFF_GROUP_BEGIN_DISK_ID,
    FALLOFF_SIZE_X_DISK_ID,
    FALLOFF_SIZE_Y_DISK_ID,
    FALLOFF_SIZE_Z_DISK_ID,
    FALLOFF_POWER_DISK_ID,
    FALLOFF_GROUP_END_DISK_ID,

    // box properties
    GEOMETRY_GROUP_BEGIN_DISK_ID,
    GEOMETRY_POSITION_DISK_ID,
    GEOMETRY_SIZE_X_DISK_ID,
    GEOMETRY_SIZE_Y_DISK_ID,
    GEOMETRY_SIZE_Z_DISK_ID,
    GEOMETRY_ROTATE_X_DISK_ID,
    GEOMETRY_ROTATE_Y_DISK_ID,
    GEOMETRY_ROTATE_Z_DISK_ID,
    GEOMETRY_FALL_OFF_SIZE_X_DISK_ID,
    GEOMETRY_FALL_OFF_SIZE_Y_DISK_ID,
    GEOMETRY_FALL_OFF_SIZE_Z_DISK_ID,
    GEOMETRY_GROUP_END_DISK_ID,

    // render properties
    VFN_RENDER_GROUP_BEGIN_DISK_ID,
    VFN_CAST_RESOLUTION_DISK_ID,
    VFN_DISTANCE_ATTENUATION_DISK_ID,
    VFN_RENDER_GROUP_END_DISK_ID
};

enum
{
    NOISE_TYPE_SMOOTH = 0,
    NOISE_TYPE_BLOCK,
    NOISE_TYPE_COUNT
};

template <typename TPixel> struct PixelData
{
public:
    TPixel *buffer;
    PF_EffectWorld *inputWorld;

    PixelData(TPixel *buffer, PF_EffectWorld *inputWorld)
    {
        this->buffer = buffer;
        this->inputWorld = inputWorld;
    }
};

struct PixelFormatInfo
{
public:
    PF_PixelFormat format;
    gl::GLenum glFormat;
    A_long pixelSize;

    PixelFormatInfo(PF_PixelFormat format, gl::GLenum glFormat, A_long pixelSize)
    {
        this->format = format;
        this->glFormat = glFormat;
        this->pixelSize = pixelSize;
    }
};

#ifdef __cplusplus
    extern "C" {
#endif

DllExport PF_Err EntryPointFunc(PF_Cmd cmd, PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output, void *extra) ;

#ifdef __cplusplus
}
#endif

#endif // VFN_H