/* 

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include <VX/vx.h>
#include <VX/vxu.h>

#include "test_engine/test.h"

static void reference_mean_stddev(CT_Image src, vx_float32* _mean, vx_float32* _stddev)
{
    uint32_t x, y, width = src ? src->width : 0, height = src ? src->height : 0;
    uint32_t npix, stride;
    double sum = 0, sqsum = 0;
    int format = src ? src->format : VX_DF_IMAGE_U8;

    ASSERT(src);
    ASSERT(src->width > 0 && src->height > 0);
    npix = width*height;
    stride = ct_stride_bytes(src);

#define CASE_MEANSTDDEV(format, type, acctype) \
    case format: \
    { \
        acctype s = 0, s2 = 0; \
        for( y = 0; y < src->height; y++ ) \
        { \
            const type* ptr = (const type*)(src->data.y + stride*y); \
            for( x = 0; x < src->width; x++ ) \
            { \
                type val = ptr[x]; \
                s += val; \
                s2 += (acctype)val*val; \
            } \
        } \
        sum = (double)s; sqsum = (double)s2; \
    } \
    break

    switch(format)
    {
    CASE_MEANSTDDEV(VX_DF_IMAGE_U8, uint8_t, uint64_t);
    default:
        FAIL("Unsupported image format: (%d)", &src->format);
    }

    *_mean = (vx_float32)(sum/npix);
    sqsum = sqsum/npix - (sum/npix)*(sum/npix);
    *_stddev = (vx_float32)sqrt(CT_MAX(sqsum, 0.));
}


TESTCASE(MeanStdDev, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int mode;
    vx_df_image format;
} format_arg;


#define MEANSTDDEV_TEST_CASE(imm, tp) \
    {#imm "/" #tp, CT_##imm##_MODE, VX_DF_IMAGE_##tp}

TEST_WITH_ARG(MeanStdDev, testOnRandom, format_arg,
              MEANSTDDEV_TEST_CASE(Immediate, U8),
              MEANSTDDEV_TEST_CASE(Graph, U8),
              )
{
    double mean_tolerance = 1e-4;
    double stddev_tolerance = 1e-4;
    int format = arg_->format;
    int mode = arg_->mode;
    vx_image src;
    CT_Image src0;
    vx_node node = 0;
    vx_graph graph = 0;
    vx_scalar mean_s, stddev_s;
    vx_context context = context_->vx_context_;
    int iter, niters = 100;
    uint64_t rng;
    vx_float32 mean0 = 0.f, stddev0 = 0.f, mean = 0.f, stddev = 0.f;
    int a = 0, b = 256;

    rng = CT()->seed_;
    mean_tolerance *= b;
    stddev_tolerance *= b;

    mean_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean);
    ASSERT_VX_OBJECT(mean_s, VX_TYPE_SCALAR);
    stddev_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev);
    ASSERT_VX_OBJECT(stddev_s, VX_TYPE_SCALAR);

    for( iter = 0; iter < niters; iter++ )
    {
        int width = ct_roundf(ct_log_rng(&rng, 0, 10));
        int height = ct_roundf(ct_log_rng(&rng, 0, 10));
        double mean_diff, stddev_diff;
        width = CT_MAX(width, 1);
        height = CT_MAX(height, 1);

        if( !ct_check_any_size() )
        {
            width = CT_MIN((width + 7) & -8, 640);
            height = CT_MIN((height + 7) & -8, 480);
        }

        ct_update_progress(iter, niters);

        src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        reference_mean_stddev(src0, &mean0, &stddev0);
        src = ct_image_to_vx_image(src0, context);
        if( mode == CT_Immediate_MODE )
        {
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxuMeanStdDev(context, src, &mean, &stddev));
        }
        else
        {
            graph = vxCreateGraph(context);
            ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
            node = vxMeanStdDevNode(graph, src, mean_s, stddev_s);
            ASSERT_VX_OBJECT(node, VX_TYPE_NODE);
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));

            VX_CALL(vxCopyScalar(mean_s, &mean, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyScalar(stddev_s, &stddev, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        }

        mean_diff = fabs(mean - mean0);
        stddev_diff = fabs(stddev - stddev0);

        if( mean_diff > mean_tolerance ||
            stddev_diff > stddev_tolerance )
        {
            CT_RecordFailureAtFormat("Test case %d. width=%d, height=%d,\n"
                                     "\tExpected: mean=%.5g, stddev=%.5g\n"
                                     "\tActual:   mean=%.5g (diff=%.5g %s %.5g), stddev=%.5f (diff=%.5g %s %.5g)\n",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     mean0, stddev0,
                                     mean, mean_diff, mean_diff > mean_tolerance ? ">" : "<=", mean_tolerance,
                                     stddev, stddev_diff, stddev_diff > stddev_tolerance ? ">" : "<=", stddev_tolerance);
            break;
        }

        VX_CALL(vxReleaseImage(&src));
        if(node)
            VX_CALL(vxReleaseNode(&node));
        if(graph)
            VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);
    }

    VX_CALL(vxReleaseScalar(&mean_s));
    VX_CALL(vxReleaseScalar(&stddev_s));
}

TESTCASE_TESTS(MeanStdDev, testOnRandom)
