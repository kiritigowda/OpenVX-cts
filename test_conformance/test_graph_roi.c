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

TESTCASE(GraphROI, CT_VXContext, ct_setup_vx_context, 0)

TEST(GraphROI, testSimple)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, interm_image_roi = 0, dst_image = 0;
    vx_rectangle_t rect = { 10, 10, 118, 118 };
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 108, 108, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image_roi = vxCreateImageFromROI(interm_image, &rect), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 108, 108, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image_roi), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image_roi, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image_roi));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}


static vx_bool own_error = vx_false_e;
static vx_bool own_cb1_called = vx_false_e;
static vx_bool own_cb2_called = vx_false_e;
static vx_action VX_CALLBACK own_callback1(vx_node node)
{
    // printf("callback1 called\n"); fflush(stdout);
    own_cb1_called = vx_true_e;
    if (own_cb2_called == vx_true_e)
    {
        own_error = vx_true_e;
    }
    return VX_ACTION_CONTINUE;
}
static vx_action VX_CALLBACK own_callback2(vx_node node)
{
    // printf("callback2 called\n"); fflush(stdout);
    own_cb2_called = vx_true_e;
    if (own_cb1_called != vx_true_e)
    {
        own_error = vx_true_e;
    }
    return VX_ACTION_CONTINUE;
}

typedef struct {
    const char* testName;
    vx_bool forward;
} Arg;


TEST_WITH_ARG(GraphROI, testCallbackOrder, Arg,
        CT_ARG("Forward", vx_true_e),
        CT_ARG("Reverse", vx_false_e)
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, interm_image_roi = 0, dst_image = 0;
    vx_rectangle_t rect = { 10, 10, 118, 118 };
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    int i = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image_roi = vxCreateImageFromROI(interm_image, &rect), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 108, 108, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (arg_->forward == vx_true_e)
    {
        ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image_roi, dst_image), VX_TYPE_NODE);
    }
    else
    {
        ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image_roi, dst_image), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    }

    VX_CALL(vxAssignNodeCallback(node1, own_callback1));
    VX_CALL(vxAssignNodeCallback(node2, own_callback2));

    VX_CALL(vxVerifyGraph(graph));

    for (i = 0; i < 10; i++)
    {
        own_cb1_called = vx_false_e;
        own_cb2_called = vx_false_e;
        own_error = vx_false_e;

        VX_CALL(vxProcessGraph(graph));

        ASSERT(own_cb1_called == vx_true_e);
        ASSERT(own_cb2_called == vx_true_e);
        EXPECT(own_error == vx_false_e);
    }

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image_roi));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}

TESTCASE_TESTS(GraphROI,
        testSimple,
        testCallbackOrder
        )
