# Copyright 2012 Intel Corporation
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
# Set Android specific paths

LIST(APPEND CMAKE_PREFIX_PATH $ENV{ANDROID_PRODUCT_OUT}/system)
LIST(APPEND CMAKE_PREFIX_PATH $ENV{ANDROID_PRODUCT_OUT}/obj/include)
LIST(APPEND CMAKE_PREFIX_PATH $ENV{ANDROID_BUILD_TOP}/external/zlib)
LIST(APPEND CMAKE_SYSTEM_INCLUDE_PATH $ENV{ANDROID_BUILD_TOP}/external/mesa/include)
LIST(APPEND CMAKE_SYSTEM_INCLUDE_PATH $ENV{ANDROID_BUILD_TOP}/external/mesa3d/include)

SET(CMAKE_DL_LIBS "dl")

SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,--allow-shlib-undefined")
SET(CMAKE_SHARED_LIBRARY_RPATH_LINK_C_FLAG "-Wl,-rpath-link,")
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")
SET(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-soname,")

SET(CMAKE_EXE_EXPORTS_C_FLAG "-Wl,--export-dynamic")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--allow-shlib-undefined")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_LD_FLAGS}")

# piglit has no "install" target, so RPATH is never set to CMAKE_INSTALL_PATH
SET(CMAKE_PLATFORM_REQUIRED_RUNTIME_PATH "/system/lib")
