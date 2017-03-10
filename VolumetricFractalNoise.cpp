#include "VolumetricFractalNoise.h"

#include "Smart_Utils.h"
#include "AEFX_SuiteHelper.h"
#include "AEGLContextStore.h"

#define _USE_MATH_DEFINES

#include <math.h>
#include <atomic>
#include <mutex>
#include <memory>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_cross_product.hpp>

#include "GLRendererManager.h"
#include "ParamScoper.h"
#include "ShaderCodes.h"

using namespace glm;

using namespace gl;

std::shared_ptr<GLRendererManager> rendererManager = nullptr;
std::recursive_mutex mutex;

BOOL raiseError = FALSE;

inline PF_FpShort AngleToRad(PF_FpShort angle)
{
    return angle * (M_PI / 180.0);
}

static std::shared_ptr<GLFilterRenderer> GetRenderer(A_long width, A_long height)
{
    mutex.lock();

    std::shared_ptr<GLFilterRenderer> result = rendererManager->GetRenderer(std::this_thread::get_id(), width, height);

    mutex.unlock();

    return result;
}

static PixelFormatInfo DetectColorFormat(PF_PixelFormat format)
{
    switch (format) {
    case PF_PixelFormat_ARGB128:
        return PixelFormatInfo(format, GL_FLOAT, sizeof(PF_PixelFloat));

    case PF_PixelFormat_ARGB64:
        return PixelFormatInfo(format, GL_UNSIGNED_SHORT, sizeof(PF_Pixel16));

    case PF_PixelFormat_ARGB32:
        return PixelFormatInfo(format, GL_UNSIGNED_BYTE, sizeof(PF_Pixel8));

    default:
        return PixelFormatInfo(format, GL_NONE, 0);
    }
}

template <typename TPixel> static PF_Err CopyPixelToOut(void *refcon, A_long x, A_long y, TPixel * _, TPixel *outPixel)
{
    PixelData<TPixel> *data = reinterpret_cast<PixelData<TPixel> *>(refcon);
    TPixel *inputPixel = data->buffer + y * data->inputWorld->width + x;

    outPixel->alpha = inputPixel->alpha;
    outPixel->blue = inputPixel->blue;
    outPixel->green = inputPixel->green;
    outPixel->red = inputPixel->red;

    return PF_Err_NONE;
}

template <typename TPixel> static PF_Err CopyPixelToOutHalf(void *refcon, A_long x, A_long y, TPixel * _, TPixel *outPixel)
{
    PixelData<TPixel> *data = reinterpret_cast<PixelData<TPixel> *>(refcon);
    TPixel *inputPixel = data->buffer + y * data->inputWorld->width + x;

    outPixel->alpha = inputPixel->alpha / 2;
    outPixel->blue = inputPixel->blue / 2;
    outPixel->green = inputPixel->green / 2;
    outPixel->red = inputPixel->red / 2;

    return PF_Err_NONE;
}

static PF_Err ReadRenderResult(std::shared_ptr<GLFilterRenderer> renderer, PixelFormatInfo& formatInfo, AEGP_SuiteHandler& suites, PF_EffectWorld *inputWorld, PF_EffectWorld *outputWorld, PF_InData *inData)
{
    PF_Handle buffer = suites.HandleSuite1()->host_new_handle(renderer->GetTotalPixels() * formatInfo.pixelSize);
    if (!buffer)
    {
        return PF_Err_OUT_OF_MEMORY;
    }

    void *lockedBuffer = suites.HandleSuite1()->host_lock_handle(buffer);
    renderer->ReadPixels(formatInfo.glFormat, lockedBuffer);

    PF_Err err = PF_Err_NONE;
    switch (formatInfo.format)
    {
        case PF_PixelFormat_ARGB128:
            {
                PixelData<PF_PixelFloat> data(reinterpret_cast<PF_PixelFloat *>(lockedBuffer), inputWorld);
                ERR(suites.IterateFloatSuite1()->iterate(inData, 0, inputWorld->height, inputWorld, nullptr, reinterpret_cast<void *>(&data), CopyPixelToOut<PF_PixelFloat>, outputWorld));
            }
            break;

        case PF_PixelFormat_ARGB64:
            {
                PixelData<PF_Pixel16> data(reinterpret_cast<PF_Pixel16 *>(lockedBuffer), inputWorld);
                ERR(suites.Iterate16Suite1()->iterate(inData, 0, inputWorld->height, inputWorld, nullptr, reinterpret_cast<void *>(&data), CopyPixelToOutHalf<PF_Pixel16>, outputWorld));
            }
            break;

        case PF_PixelFormat_ARGB32:
            {
                PixelData<PF_Pixel8> data(reinterpret_cast<PF_Pixel8 *>(lockedBuffer), inputWorld);
                ERR(suites.Iterate8Suite1()->iterate(inData, 0, inputWorld->height, inputWorld, nullptr, reinterpret_cast<void *>(&data), CopyPixelToOut<PF_Pixel8>, outputWorld));
            }
            break;

        default:
            break;
    }

    suites.HandleSuite1()->host_unlock_handle(buffer);
    suites.HandleSuite1()->host_dispose_handle(buffer);
    return PF_Err_NONE;
}

static PF_FpLong GetFieldOfView(PF_FpLong zoom, A_long width, A_long height, BOOL vertical)
{
    if (vertical)
    {
        return atan((height / zoom) * 0.5) * 2.0;
    }
    else
    {
        return atan((width / zoom) * 0.5) * 2.0;
    }
}

static PF_Err GetModelViewAndFov(PF_InData *in_data, AEGP_SuiteHandler& suites, mat4 *modelView, PF_FpShort *fovy)
{
    PF_Err err = PF_Err_NONE;

    A_Time compositionTime;
    AEGP_LayerH currentLayer;
    AEGP_CompH composition;
    AEGP_ItemH compositionItem;
    A_long compositionWidth;
    A_long compositionHeight;
    suites.PFInterfaceSuite1()->AEGP_GetEffectLayer(in_data->effect_ref, &currentLayer);
    ERR(suites.PFInterfaceSuite1()->AEGP_ConvertEffectToCompTime(in_data->effect_ref, in_data->current_time, in_data->time_scale, &compositionTime));
    ERR(suites.LayerSuite5()->AEGP_GetLayerParentComp(currentLayer, &composition));
    ERR(suites.CompSuite10()->AEGP_GetItemFromComp(composition, &compositionItem));
    ERR(suites.ItemSuite8()->AEGP_GetItemDimensions(compositionItem, &compositionWidth, &compositionHeight));

    AEGP_LayerH cameraLayer;
    ERR(suites.PFInterfaceSuite1()->AEGP_GetEffectCamera(in_data->effect_ref, &compositionTime, &cameraLayer));
    PF_FpLong distance;
    A_Matrix4 matrix;
    AEFX_CLR_STRUCT(matrix);
    if (cameraLayer)
    {
        AEGP_StreamVal streamVal;
        ERR(suites.StreamSuite1()->AEGP_GetLayerStreamValue(cameraLayer, AEGP_LayerStream_ZOOM, AEGP_LTimeMode_CompTime, &compositionTime, TRUE, &streamVal, NULL));
        distance = streamVal.one_d;

        ERR(suites.LayerSuite5()->AEGP_GetLayerToWorldXform(cameraLayer, &compositionTime, &matrix));
    } else
    {
        ERR(suites.CameraSuite2()->AEGP_GetDefaultCameraDistanceToImagePlane(composition, &distance));

        matrix.mat[0][0] = 1.0;
        matrix.mat[1][1] = 1.0;
        matrix.mat[2][2] = 1.0;
        matrix.mat[3][3] = 1.0;
        matrix.mat[3][0] = compositionWidth * 0.5;
        matrix.mat[3][1] = compositionHeight * 0.5;
        matrix.mat[3][2] = -distance;
    }
    *fovy = GetFieldOfView(distance, compositionWidth, compositionHeight, TRUE);
    *modelView = mat4(
        matrix.mat[0][0], matrix.mat[0][1], matrix.mat[0][2], matrix.mat[0][3],
        matrix.mat[1][0], matrix.mat[1][1], matrix.mat[1][2], matrix.mat[1][3],
        matrix.mat[2][0], matrix.mat[2][1], matrix.mat[2][2], matrix.mat[2][3],
        matrix.mat[3][0], matrix.mat[3][1], matrix.mat[3][2], matrix.mat[3][3]
    ) * mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1
    );

    return err;
}

static PF_Err About(PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output)
{
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg,
                                          "%s v%d.%d\r%s",
                                          STR(StrID_Name), 
                                          MAJOR_VERSION, 
                                          MINOR_VERSION, 
                                          STR(StrID_Description));
    return PF_Err_NONE;
}

static PF_Err GlobalSetup(PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output)
{
    out_data->my_version = PF_VERSION(MAJOR_VERSION, 
                                      MINOR_VERSION,
                                      BUG_VERSION, 
                                      STAGE_VERSION, 
                                      BUILD_VERSION);

    out_data->out_flags2 = PF_OutFlag2_FLOAT_COLOR_AWARE | PF_OutFlag2_SUPPORTS_SMART_RENDER | PF_OutFlag2_I_USE_3D_CAMERA | PF_OutFlag2_I_USE_3D_LIGHTS;
    out_data->out_flags = PF_OutFlag_DEEP_COLOR_AWARE;

    AEGLContextStore aeGLContext = AEGLContextStore::SaveContext();

    rendererManager = std::make_shared<GLRendererManager>(std::string(FRAGMENT_SHADER));

    return PF_Err_NONE;
}

static PF_Err GlobalSetdown(PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output)
{
    AEGLContextStore aeGLContext = AEGLContextStore::SaveContext();

    return PF_Err_NONE;
}

static PF_Err ParamsSetup(PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output)
{
    PF_Err err = PF_Err_NONE;
    PF_ParamDef def;

    // noise properties

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC(STR(StrID_Noise_Param_Group_Name), NOISE_GROUP_BEGIN_DISK_ID);

    PF_ADD_COLOR(
        STR(StrID_Noise_Color_Param_Name),
        PF_MAX_CHAN8,
        PF_MAX_CHAN8,
        PF_MAX_CHAN8,
        NOISE_COLOR_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Noise_Contrast_Param_Name),
        -0.5,
        0.5,
        -0.5,
        0.5,
        0.0,
        PF_Precision_THOUSANDTHS,
        0,
        0,
        NOISE_CONTRAST_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Noise_Density_Param_Name),
        0.0,
        10.0,
        0.0,
        1.0,
        0.5,
        PF_Precision_THOUSANDTHS,
        0,
        0,
        NOISE_DENSITY_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC(STR(StrID_Noise_Transform_Param_Group_Name), NOISE_TRANSFORM_GROUP_BEGIN_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT_3D_FIX_Z(
        STR(StrID_Noise_Transform_Position_Param_Name),
        0,
        0,
        0,
        NOISE_TRANSFORM_POSITION_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Noise_Transform_ScaleX_Param_Name),
        0.01,
        10000.0,
        0.01,
        100.0,
        100.0,
        PF_Precision_HUNDREDTHS,
        0,
        0,
        NOISE_TRANSFORM_SCALE_X_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Noise_Transform_ScaleY_Param_Name),
        0.01,
        10000.0,
        0.01,
        100.0,
        100.0,
        PF_Precision_HUNDREDTHS,
        0,
        0,
        NOISE_TRANSFORM_SCALE_Y_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Noise_Transform_ScaleZ_Param_Name),
        0.01,
        10000.0,
        0.01,
        100.0,
        100.0,
        PF_Precision_HUNDREDTHS,
        0,
        0,
        NOISE_TRANSFORM_SCALE_Z_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE(STR(StrID_Noise_Transform_RotateX_Param_Name), 0, NOISE_TRANSFORM_ROTATE_X_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE(STR(StrID_Noise_Transform_RotateY_Param_Name), 0, NOISE_TRANSFORM_ROTATE_Y_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE(STR(StrID_Noise_Transform_RotateZ_Param_Name), 0, NOISE_TRANSFORM_ROTATE_Z_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(NOISE_TRANSFORM_GROUP_END_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Noise_Octave_Param_Name),
        1.0,
        20.0,
        1.0,
        20.0,
        6.0,
        PF_Precision_HUNDREDTHS,
        0,
        0,
        NOISE_OCTAVE_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE(STR(StrID_Noise_Evolution_Param_Name), 0, NOISE_EVOLUTION_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(NOISE_GROUP_END_DISK_ID);

    // fall off properties

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC(STR(StrID_FallOff_Param_Group_Name), FALLOFF_GROUP_BEGIN_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_FallOff_SizeX_Param_Name),
        0.0,
        100000.0,
        0.0,
        100.0,
        0.0,
        PF_Precision_TENTHS,
        0,
        0,
        FALLOFF_SIZE_X_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_FallOff_SizeY_Param_Name),
        0.0,
        100000.0,
        0.0,
        100.0,
        0.0,
        PF_Precision_TENTHS,
        0,
        0,
        FALLOFF_SIZE_Y_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_FallOff_SizeZ_Param_Name),
        0.0,
        100000.0,
        0.0,
        100.0,
        0.0,
        PF_Precision_TENTHS,
        0,
        0,
        FALLOFF_SIZE_Z_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_FallOff_Power_Param_Name),
        0.0,
        100.0,
        0.0,
        1.0,
        0.0,
        PF_Precision_THOUSANDTHS,
        0,
        0,
        FALLOFF_POWER_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(FALLOFF_GROUP_END_DISK_ID);

    // box properties

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC(STR(StrID_Geometry_Param_Group_Name), GEOMETRY_GROUP_BEGIN_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT_3D_FIX_Z(
        STR(StrID_Geometry_Position_Param_Name),
        VFN_POSITION_CENTER,
        VFN_POSITION_CENTER,
        0,
        GEOMETRY_POSITION_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Geometry_SizeX_Param_Name),
        0.0,
        100000.0,
        0.0,
        100.0,
        100.0,
        PF_Precision_TENTHS,
        0,
        0,
        GEOMETRY_SIZE_X_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Geometry_SizeY_Param_Name),
        0.0,
        100000.0,
        0.0,
        100.0,
        100.0,
        PF_Precision_TENTHS,
        0,
        0,
        GEOMETRY_SIZE_Y_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Geometry_SizeZ_Param_Name),
        0.0,
        100000.0,
        0.0,
        100.0,
        100.0,
        PF_Precision_TENTHS,
        0,
        0,
        GEOMETRY_SIZE_Z_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE(STR(StrID_Geometry_RotateX_Param_Name), 0, GEOMETRY_ROTATE_X_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE(STR(StrID_Geometry_RotateY_Param_Name), 0, GEOMETRY_ROTATE_Y_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE(STR(StrID_Geometry_RotateZ_Param_Name), 0, GEOMETRY_ROTATE_Z_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(GEOMETRY_GROUP_END_DISK_ID);

    // render properties

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC(STR(StrID_Render_Param_Group_Name), VFN_RENDER_GROUP_BEGIN_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Render_CastResolution_Params_Name),
        1.0,
        100.0,
        1.0,
        1000.0,
        10.0,
        PF_Precision_HUNDREDTHS,
        0,
        0,
        VFN_CAST_RESOLUTION_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        STR(StrID_Render_DistanceAttenuation_Params_Name),
        0.0,
        100.0,
        0.0,
        1.0,
        0.0,
        PF_Precision_THOUSANDTHS,
        0,
        0,
        VFN_DISTANCE_ATTENUATION_DISK_ID
    );

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(VFN_RENDER_GROUP_END_DISK_ID);

    out_data->num_params = VFN_NUM_PARAMS;

    return err;
}

static PF_Err PreRender(PF_InData *in_data, PF_OutData *out_data, PF_PreRenderExtra *extra)
{
    PF_Err err = PF_Err_NONE;
    PF_Err err2 = PF_Err_NONE;

    PF_RenderRequest req = extra->input->output_request;
    PF_CheckoutResult inResult;

    ERR(extra->cb->checkout_layer(in_data->effect_ref,
                                  VFN_INPUT,
                                  VFN_INPUT,
                                  &req,
                                  in_data->current_time,
                                  in_data->time_step,
                                  in_data->time_scale,
                                  &inResult));


    if (!err) {
        UnionLRect(&inResult.result_rect, &extra->output->result_rect);
        UnionLRect(&inResult.max_result_rect, &extra->output->max_result_rect);
    }

    return err;
}

static PF_Err RenderProcess(PF_InData *in_data, PF_OutData *out_data, PF_SmartRenderExtra *extra)
{
    PF_Err err = PF_Err_NONE;
    PF_Err err2 = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);
    AEFX_SuiteScoper<PF_WorldSuite2> worldSuite2 = AEFX_SuiteScoper<PF_WorldSuite2>(in_data, kPFWorldSuite, kPFWorldSuiteVersion2, out_data);

    PF_EffectWorld *inputWorld;
    PF_EffectWorld *outputWorld;
    ERR(extra->cb->checkout_layer_pixels(in_data->effect_ref, VFN_INPUT, &inputWorld));
    ERR(extra->cb->checkout_output(in_data->effect_ref, &outputWorld));
    PF_PixelFormat format = PF_PixelFormat_INVALID;
    ERR(worldSuite2->PF_GetPixelFormat(inputWorld, &format));

    PixelFormatInfo formatInfo = DetectColorFormat(format);
    if (formatInfo.pixelSize < 1)
    {
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    PF_ParamIndex noiseScaleIndex[3] = { VFN_NOISE_TRANSFORM_SCALE_X, VFN_NOISE_TRANSFORM_SCALE_Y, VFN_NOISE_TRANSFORM_SCALE_Z };
    PF_ParamIndex noiseRotateIndex[3] = { VFN_NOISE_TRANSFORM_ROTATE_X, VFN_NOISE_TRANSFORM_ROTATE_Y, VFN_NOISE_TRANSFORM_ROTATE_Z };
    ParamScoper noiseColor = ParamScoper(in_data, VFN_NOISE_COLOR);
    ParamScoper contrast = ParamScoper(in_data, VFN_NOISE_CONTRAST);
    ParamScoper density = ParamScoper(in_data, VFN_NOISE_DENSITY);
    ParamScoper noisePosition = ParamScoper(in_data, VFN_NOISE_TRANSFORM_POSITION);
    ParamScoper3DProps noiseScale = ParamScoper3DProps(in_data, noiseScaleIndex);
    ParamScoper3DProps noiseRotate = ParamScoper3DProps(in_data, noiseRotateIndex);
    ParamScoper octave = ParamScoper(in_data, VFN_NOISE_OCTAVE);
    ParamScoper evolution = ParamScoper(in_data, VFN_NOISE_EVOLUTION);

    PF_ParamIndex fallOffSizeIndex[3] = { VFN_FALLOFF_SIZE_X, VFN_FALLOFF_SIZE_Y, VFN_FALLOFF_SIZE_Z };
    ParamScoper3DProps fallOffSize = ParamScoper3DProps(in_data, fallOffSizeIndex);
    ParamScoper fallOffPower = ParamScoper(in_data, VFN_FALLOFF_POWER);

    PF_ParamIndex boxSizeIndex[3] = { VFN_GEOMETRY_SIZE_X, VFN_GEOMETRY_SIZE_Y, VFN_GEOMETRY_SIZE_Z };
    PF_ParamIndex boxRotateIndex[3] = { VFN_GEOMETRY_ROTATE_X, VFN_GEOMETRY_ROTATE_Y, VFN_GEOMETRY_ROTATE_Z };
    ParamScoper boxCenter = ParamScoper(in_data, VFN_GEOMETRY_POSITION);
    ParamScoper3DProps boxSize = ParamScoper3DProps(in_data, boxSizeIndex);
    ParamScoper3DProps boxRotate = ParamScoper3DProps(in_data, boxRotateIndex);

    ParamScoper castResolution = ParamScoper(in_data, VFN_CAST_RESOLUTION);
    ParamScoper distanceAttenuation = ParamScoper(in_data, VFN_DISTANCE_ATTENUATION);

    mat4 modelView;
    PF_FpShort fovy;
    ERR(GetModelViewAndFov(in_data, suites, &modelView, &fovy));
    modelView = glm::inverse(glm::translate(mat4(), boxCenter.GetPosition()) *
        glm::rotate(mat4(), AngleToRad(boxRotate.GetRotateX()), vec3(1.0, 0.0, 0.0)) *
        glm::rotate(mat4(), AngleToRad(boxRotate.GetRotateY()), vec3(0.0, 1.0, 0.0)) *
        glm::rotate(mat4(), AngleToRad(boxRotate.GetRotateZ()), vec3(0.0, 0.0, 1.0))) *
        modelView;

    mat4 noiseTransform = glm::inverse(
        glm::translate(mat4(), noisePosition.GetPosition()) *
        glm::rotate(mat4(), AngleToRad(noiseRotate.GetRotateX()), vec3(1.0, 0.0, 0.0)) *
        glm::rotate(mat4(), AngleToRad(noiseRotate.GetRotateY()), vec3(0.0, 1.0, 0.0)) *
        glm::rotate(mat4(), AngleToRad(noiseRotate.GetRotateZ()), vec3(0.0, 0.0, 1.0)) *
        glm::scale(mat4(), vec3(noiseScale.GetPositionX(), noiseScale.GetPositionY(), noiseScale.GetPositionZ()))
    );

    AEGLContextStore aeGLContext = AEGLContextStore::SaveContext();

    std::shared_ptr<GLFilterRenderer> renderer = GetRenderer(outputWorld->width, outputWorld->height);
    renderer->SetContext();

    renderer->shader->SetUniform2fv("resolution", glm::vec2(outputWorld->width / (in_data->downsample_x.num / (PF_FpShort)in_data->downsample_x.den), outputWorld->height / (in_data->downsample_y.num / (PF_FpShort)in_data->downsample_y.den)));
    renderer->shader->SetUniformMatrix4dv("modelView", dmat4(modelView));
    renderer->shader->SetUniform1f("aspect", outputWorld->height / (PF_FpShort)outputWorld->width);
    renderer->shader->SetUniform1f("fov", tan(fovy * 0.5F) * 2.0F);

    renderer->shader->SetUniform3fv("noiseColor", noiseColor.GetColor());
    renderer->shader->SetUniform1f("contrast", (PF_FpShort)contrast.GetFloat());
    renderer->shader->SetUniform1f("density", (PF_FpShort)density.GetFloat());
    renderer->shader->SetUniformMatrix4dv("noiseTransform", dmat4(noiseTransform));
    renderer->shader->SetUniform1f("octave", (PF_FpShort)octave.GetFloat());
    renderer->shader->SetUniform1f("evolution", (PF_FpShort)evolution.GetAngle());

    renderer->shader->SetUniform3dv("coreSize", glm::max(dvec3(), dvec3(boxSize.GetPosition() - fallOffSize.GetPosition())));
    renderer->shader->SetUniform1f("fallOffPower", fallOffPower.GetFloat() * 0.1);

    renderer->shader->SetUniform3dv("boxSize", dvec3(boxSize.GetPosition()));

    renderer->shader->SetUniform1d("castResolution", castResolution.GetFloat());
    renderer->shader->SetUniform1d("distanceAttenuation", distanceAttenuation.GetFloat() * 0.01);
    renderer->shader->SetUniform1d("densityRateByCast", castResolution.GetFloat() / castResolution.GetDefaultFloat());

    renderer->Draw();
    err = ReadRenderResult(renderer, formatInfo, suites, inputWorld, outputWorld, in_data);

    ERR(PF_ABORT(in_data));

    ERR2(extra->cb->checkin_layer_pixels(in_data->effect_ref, VFN_INPUT));

    return err;
}

static PF_Err SmartRender(PF_InData *in_data, PF_OutData *out_data, PF_SmartRenderExtra *extra)
{
    if (raiseError)
    {
        return PF_Err_NONE;
    }

    try
    {
        return RenderProcess(in_data, out_data, extra);
    }
    catch (std::runtime_error &e)
    {
        PF_SPRINTF(out_data->return_msg, const_cast<A_char *>(e.what()));
        raiseError = TRUE;
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
}

DllExport PF_Err EntryPointFunc(PF_Cmd cmd, PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output, void *extra)
{
    PF_Err err = PF_Err_NONE;
	
    try {
        switch (cmd) {
            case PF_Cmd_ABOUT:
                err = About(in_data, out_data, params, output);
                break;

            case PF_Cmd_GLOBAL_SETUP:
                err = GlobalSetup(in_data, out_data, params, output);
                break;

            case PF_Cmd_PARAMS_SETUP:
                err = ParamsSetup(in_data, out_data, params, output);
                break;

            case PF_Cmd_GLOBAL_SETDOWN:
                err = GlobalSetdown(in_data, out_data, params, output);
                break;

            case PF_Cmd_SMART_PRE_RENDER:
                err = PreRender(in_data, out_data, reinterpret_cast<PF_PreRenderExtra*>(extra));
                break;

            case PF_Cmd_SMART_RENDER:
                err = SmartRender(in_data, out_data, reinterpret_cast<PF_SmartRenderExtra*>(extra));
                break;
        }
    }
    catch(PF_Err &e){
        err = e;
    }
    return err;
}