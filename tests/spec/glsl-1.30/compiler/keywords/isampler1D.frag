// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Check that 'isampler1D' is a keyword.

#version 130

int f()
{
	int isampler1D;
	return 0;
}
