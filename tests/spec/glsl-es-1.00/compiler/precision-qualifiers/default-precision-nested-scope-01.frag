// [config]
// expect_result: pass
// glsl_version: 1.00
// [end config]
//
// Section 4.5.3 (Default Precision Qualifiers) of the GLSL ES 1.00 spec says:
//
//     "Non-precision qualified declarations will use the precision qualifier
//     specified in the most recent precision statement that is still in
//     scope. The precision statement has the same scoping rules as variable
//     declarations. If it is declared inside a compound statement, its effect
//     stops at the end of the innermost statement it was declared
//     in. Precision statements in nested scopes override precision statements
//     in outer scopes. Multiple precision statements for the same basic type
//     can appear inside the same scope, with later statements overriding
//     earlier statements within that scope."
//
// The declaration inside the function should use the precision set at global
// scope.

#version 100

precision mediump float;

void f()
{
	float x;
}

float y;