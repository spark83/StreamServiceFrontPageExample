#ifndef DEFAULTDATAS_H
#define DEFAULTDATAS_H

#ifdef __cplusplus
extern "C" {
#endif

// No need to load from file, we will only need few simple shaders to do the job.
// I am trying to keep this shader as simple as possible but perhaps I will add in
// orthographic projection later, so that each quads can rotate through a given
// axis.
#define INDEXED_VERTEX_SHADER_SRC "\n"\
"#version 460 core\n"\
"layout (location = 0) in vec3 vertex;\n"\
"layout (location = 1) in vec2 uv;\n"\
"out vec2 frag_uv;\n"\
"uniform mat4 ortho;\n"\
"uniform vec2 tile_scalar;\n"\
"uniform ivec2 row_col_index;\n"\
"uniform vec2 size_2d;\n"\
"uniform vec2 pos_2d;\n"\
"uniform vec2 scalar;\n"\
"void main() {\n"\
"	frag_uv = uv * tile_scalar;\n"\
"   frag_uv.x += tile_scalar.x * float(row_col_index[0]);\n"\
"   frag_uv.y += tile_scalar.y * float(row_col_index[1]);\n"\
"   vec3 vtx = vertex * vec3(scalar, 1.0) * vec3(size_2d, 1.0);\n"\
"   vec4 projected_vtx = ortho * (vec4(vtx, 1.0) + vec4(pos_2d, 0.0, 0.0));\n"\
"	gl_Position = projected_vtx;\n"\
"}";

#define INDEXED_FRAGMENT_SHADER_SRC "\n"\
"#version 460 core\n"\
"out vec4 frag_out;\n"\
"in vec2 frag_uv;\n"\
"uniform sampler2D image;\n"\
"void main() {\n"\
"	frag_out = texture(image, frag_uv);\n"\
"}";

#define FONT_VERTEX_SHADER_SRC "\n"\
"#version 460 core\n"\
"layout (location = 0) in vec3 vertex;\n"\
"layout (location = 1) in vec2 uv;\n"\
"out vec2 frag_uv;\n"\
"uniform mat4 ortho;\n"\
"uniform vec2 pos_2d;\n"\
"uniform float font_size;\n"\
"uniform vec2 topleft;\n"\
"uniform vec2 bottomright;\n"\
"uniform vec2 scalar;\n"\
"void main() {\n"\
"    vec2 size = bottomright - topleft;\n"\
"    vec2 frag_coord = uv * size;\n"\
"    frag_coord += topleft;\n"\
"    frag_uv = frag_coord;\n"\
"    vec3 vtx = vertex * vec3(scalar, 1.0);\n"\
"    vtx += vec3(pos_2d.x, pos_2d.y, 0.0);\n"\
"    gl_Position = ortho * vec4(vtx, 1.0);\n"\
"}";

#define FONT_FRAGMENT_SHADER_SRC "\n"\
"#version 460 core\n"\
"out vec4 frag_out;\n"\
"in vec2 frag_uv;\n"\
"uniform sampler2D image;\n"\
"void main() {\n"\
"   vec4 color = texture(image, frag_uv);\n"\
"	frag_out = vec4(color.r, color.r, color.r, color.r);\n"\
"}";

#define QUAD_VERTEX_SHADER_SRC "\n"\
"#version 460 core\n"\
"layout (location = 0) in vec3 vertex;\n"\
"layout (location = 1) in vec2 uv;\n"\
"out vec2 frag_uv;\n"\
"uniform mat4 ortho;\n"\
"uniform vec2 size_2d;\n"\
"uniform vec2 pos_2d;\n"\
"uniform vec2 scalar;\n"\
"void main() {\n"\
"   vec3 vtx = vertex * vec3(scalar, 1.0) * vec3(size_2d, 1.0);\n"\
"   vec4 projected_vtx = ortho * (vec4(vtx, 1.0) + vec4(pos_2d, 0.0, 0.0));\n"\
"   frag_uv = uv;\n"\
"	gl_Position = projected_vtx;\n"\
"}";

#define QUAD_FRAGMENT_SHADER_SRC "\n"\
"#version 460 core\n"\
"out vec4 frag_out;\n"\
"in vec2 frag_uv;\n"\
"uniform float opacity;\n"\
"uniform sampler2D image;\n"\
"void main() {\n"\
"	frag_out = texture(image, frag_uv) * opacity;\n"\
"}";

#ifdef __cplusplus
}
#endif

#endif
