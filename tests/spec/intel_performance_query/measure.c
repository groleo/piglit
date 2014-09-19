/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file measure.c
 *
 * Some INTEL_performance_query
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_height = 250;
	config.window_width = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/******************************************************************************/
static int
test_basic_measurement(char* queryName, const char* counterName,uint64_t value)
{
#define GEN_QUERY_NAME_MAX_LENGTH 256
#define GEN_COUNTER_NAME_MAX_LEN  256
#define GEN_COUNTER_DESC_MAX_LEN  1024
	typedef struct
	{
		char                    Name[GEN_COUNTER_NAME_MAX_LEN];
		char                    Desc[GEN_COUNTER_DESC_MAX_LEN];
		uint32_t                Offset;
		uint32_t                DataSize;
		uint32_t                Type;
		uint32_t                DataType;
		uint64_t                MaxValue;
	} GEN_COUNTER_INFO;

	unsigned queryId;
	char query_name[GEN_QUERY_NAME_MAX_LENGTH];
	uint32_t data_size = 0;
	uint32_t counter_num = 0;
	uint32_t max_query_number = 0;
	uint32_t caps_mask = 0;
	int counter_id;
	GEN_COUNTER_INFO    gpu_counter_info;
	uint32_t data_query_handle;
	uint32_t bytes_written = 0;
	void* data;
	unsigned int counter_value;

	// get vendor queryID by name
	glGetPerfQueryIdByNameINTEL(queryName, &queryId);
	assert(queryId>0);

	glGetPerfQueryInfoINTEL(queryId, GEN_QUERY_NAME_MAX_LENGTH, query_name, &data_size, &counter_num, &max_query_number, &caps_mask);
	assert(data_size>0);
	assert(counter_num>0);

	// Look for counter info

	memset(&gpu_counter_info, 0, sizeof(gpu_counter_info));
	for (counter_id = 1; counter_id <= counter_num; ++counter_id)
	{
		memset(&gpu_counter_info, 0, sizeof(gpu_counter_info));
		glGetPerfCounterInfoINTEL(queryId, counter_id, GEN_COUNTER_NAME_MAX_LEN,
				gpu_counter_info.Name, GEN_COUNTER_DESC_MAX_LEN, gpu_counter_info.Desc,
				&gpu_counter_info.Offset, &gpu_counter_info.DataSize,
				&gpu_counter_info.Type, &gpu_counter_info.DataType,
				&gpu_counter_info.MaxValue);
		if (0 == strcmp(gpu_counter_info.Name, counterName))
		{
			break;
		}
	}

	// Create data query
	glCreatePerfQueryINTEL(queryId, &data_query_handle);
	if (data_query_handle<=0) {
		piglit_report_subtest_result(PIGLIT_FAIL, __FUNCTION__);
		return PIGLIT_FAIL;
	}

	// Allocate buffer for metrics data
	data = malloc(data_size);

	// Run sample
	glBeginPerfQueryINTEL(data_query_handle);
	piglit_draw_rect(0, 0, 1,1);
	glEndPerfQueryINTEL(data_query_handle);

	glGetPerfQueryDataINTEL(data_query_handle, GL_PERFQUERY_WAIT_INTEL, data_size, data, &bytes_written);
	if (data_size != bytes_written) {
		piglit_report_subtest_result(PIGLIT_FAIL, __FUNCTION__);
		return PIGLIT_FAIL;
	}

	counter_value = (unsigned int) (*( (uint64_t*) (( (char*) data) + gpu_counter_info.Offset)));
	printf("VALUE(%s) = %u\n", counterName, counter_value);

	// Release resources
	free(data);
	glDeletePerfQueryINTEL(data_query_handle);
	if (counter_value==value) {
		piglit_report_subtest_result(PIGLIT_PASS, counterName);
		return PIGLIT_PASS;
	}
	piglit_report_subtest_result(PIGLIT_FAIL, counterName);
	return PIGLIT_FAIL;
}



/******************************************************************************/

enum piglit_result
piglit_display(void)
{
	int rep=PIGLIT_PASS;
	rep |= test_basic_measurement("Pipeline Statistics Registers","IA_VERTICES_COUNT",1);
	rep |= test_basic_measurement("Pipeline Statistics Registers","IA_PRIMITIVES_COUNT",1);
	rep |= test_basic_measurement("Pipeline Statistics Registers","VS_INVOCATION_COUNT",4);
	rep |= test_basic_measurement("Pipeline Statistics Registers","GS_INVOCATION_COUNT",0);
	rep |= test_basic_measurement("Pipeline Statistics Registers","GS_PRIMITIVES_COUNT",0);
	rep |= test_basic_measurement("Pipeline Statistics Registers","CL_INVOCATION_COUNT",2);
	rep |= test_basic_measurement("Pipeline Statistics Registers","CL_PRIMITIVES_COUNT",2);
	rep |= test_basic_measurement("Pipeline Statistics Registers","PS_INVOCATION_COUNT",(250*250)/4);
	rep |= test_basic_measurement("Pipeline Statistics Registers","PS_DEPTH_COUNT",(250*250)/4);
	rep |= test_basic_measurement("Pipeline Statistics Registers","SO_NUM_PRIMS_WRITTEN",1);
	rep |= test_basic_measurement("Pipeline Statistics Registers","SO_PRIM_STORAGE_NEEDED",1);
	if (rep == PIGLIT_PASS) {
		piglit_report_result(PIGLIT_PASS);
		return PIGLIT_PASS;
	}
	piglit_report_result(PIGLIT_FAIL);
	return PIGLIT_FAIL;
}

/**
 * The main test program.
 */
void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(30);
	piglit_require_extension("GL_INTEL_performance_query");
	piglit_test_min_int(GL_PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL, 256);
	piglit_test_min_int(GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL, 256);
	piglit_test_min_int(GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL, 1024);
}
