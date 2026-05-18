/* License
 *
 * ${PROJ_DESCRIPTION}
 * Copyright (C) 2025 Yaokai Liu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 * Project Name: xGL
 * Module Name: xGLVulkan
 * Filename: xgl.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef XGL_H
#define XGL_H

#include "xglm.h"
#include "xGLVulkan/XGLVulkan.h"

#ifdef XGL_MATH_DEBUG_PRINT
#define printFMat4(M) \
        rt_debug(#M" =\n"     \
        "\t%f\t%f\t%f\t%f\n"  \
        "\t%f\t%f\t%f\t%f\n"  \
        "\t%f\t%f\t%f\t%f\n"  \
        "\t%f\t%f\t%f\t%f\n", \
        M[0][0], M[0][1], M[0][2], M[0][3], \
        M[1][0], M[1][1], M[1][2], M[1][3], \
        M[2][0], M[2][1], M[2][2], M[2][3], \
        M[3][0], M[3][1], M[3][2], M[3][3]  \
)
#define printFMat2(M)     \
        rt_debug(#M" =\n" \
        "\t%f\t%f\n"      \
        "\t%f\t%f\n"      \
        M[0][0], M[0][1], \
        M[1][0], M[1][1]  \
)
#define printFVec4(V)           \
        rt_debug(#V" =\n"       \
        "\t%f\t%f\t%f\t%f\n",   \
        V[0], V[1], V[2], V[3]  \
)
#define printFVec2(V)     \
        rt_debug(#V" =\n" \
        "\t%f\t%f\n"      \
        V[0], V[1]        \
)
#endif

typedef FVec4 XGLColor;
typedef FVec4 XGLCoord;
typedef FVec2 XGLTexCoord;

typedef struct XGLVertex {
  XGLCoord coord;
  XGLColor color;
  XGLTexCoord uvCoord;
} XGLVertex;

typedef struct XGLViewProjection {
  FMat4 view [[gnu::aligned(16)]];
  FMat4 proj [[gnu::aligned(16)]];
} XGLViewProjection;

typedef struct XGLModelViewProjection {
  FMat4 model [[gnu::aligned(16)]];
  FMat4 view [[gnu::aligned(16)]];
  FMat4 proj [[gnu::aligned(16)]];
} XGLModelViewProjection;

typedef uint32_t XGLRgba;

typedef struct Vertex2D {
  FVec2       coord;
  XGLRgba     color;
} Vertex2D;

typedef struct PixelVertex2D {
  IVec2      coord;
  XGLRgba color;
} PixelVertex2D;

#endif //XGL_H
