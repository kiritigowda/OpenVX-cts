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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>


TESTCASE(Gaussian3x3, CT_VXContext, ct_setup_vx_context, 0)


TEST(Gaussian3x3, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxGaussian3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}


// Generate input to cover these requirements:
// There should be a image with randomly generated pixel intensities.
static CT_Image gaussian3x3_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image gaussian3x3_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static int32_t gaussian_get(int32_t *values)
{
    int32_t res = (values[0] + values[2] + values[6] + values[8]) * 1 +
                (values[1] + values[3] + values[5] + values[7]) * 2 +
                (values[4]) * 4;
    res = res >> 4;
    return res;
}

static uint8_t gaussian3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    return (uint8_t)gaussian_get(values);
}

static uint8_t gaussian3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    return (uint8_t)gaussian_get(values);
}

static uint8_t gaussian3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    return (uint8_t)gaussian_get(values);
}


static CT_Image gaussian3x3_create_reference_image(CT_Image src, vx_border_t border)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, src->format);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    uint8_t res = gaussian3x3_calculate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian3x3_calculate_replicate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian3x3_calculate_constant(src, x, y, constant_value);
                    *dst_data = res;
                });
    }
    else
    {
        ASSERT_(return 0, 0);
    }
    return dst;
}


static void gaussian3x3_check(CT_Image src, CT_Image dst, vx_border_t border)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = gaussian3x3_create_reference_image(src, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst,  1, 1, 1, 1);
            ct_adjust_roi(dst_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
#if 0
    if (CT_HasFailure())
    {
        printf("=== SRC ===\n");
        ct_dump_image_info(src);
        printf("=== DST ===\n");
        ct_dump_image_info(dst);
        printf("=== EXPECTED ===\n");
        ct_dump_image_info(dst_ref);
    }
#endif
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
} Filter_Arg;

#define GAUSSIAN_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_SMALL_SET, ARG, gaussian3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, gaussian3x3_read_image, "lena.bmp")

TEST_WITH_ARG(Gaussian3x3, testGraphProcessing, Filter_Arg,
    GAUSSIAN_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxGaussian3x3Node(graph, src_image, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(gaussian3x3_check(src, dst, border));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(Gaussian3x3, testImmediateProcessing, Filter_Arg,
    GAUSSIAN_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));

    VX_CALL(vxuGaussian3x3(context, src_image, dst_image));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(gaussian3x3_check(src, dst, border));

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(Gaussian3x3, testNodeCreation, testGraphProcessing, testImmediateProcessing)
