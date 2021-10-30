#include <gl\glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <SDL.h>
#include <stb_truetype.h>
#include <cglm/cglm.h>

#include "DefaultDatas.h"
#include "logger.h"
#include "Types.h"

#include "Renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

static const u32 k_quad_indices[] = { 0, 1, 2, 2, 1, 3 };

static const QuadVertex k_quad_vtx_data[] = {
	{.vertex = { -0.5f,  0.5f,  0.0f },
	 .uv = { 0.0f, 0.0f } },
	{.vertex = { -0.5f, -0.5f,  0.0f },
	 .uv = { 0.0f, 1.0f } },
	{.vertex = { 0.5f,  0.5f,  0.0f },
	 .uv = { 1.0f, 0.0f } },
	{.vertex = { 0.5f, -0.5f,  0.0f },
	 .uv = { 1.0f, 1.0f } },
};

static const UniformInfo k_uniform_defs[NUM_UNIFORM_SLOTS] = {
	[UNIFORM_ORTHO] = {
		.uniform = UNIFORM_ORTHO,
		.uniform_location = -1,
		.name = "ortho"
	},
	[UNIFORM_TILE_SCALAR] = {
		.uniform = UNIFORM_TILE_SCALAR,
		.uniform_location = -1,
		.name = "tile_scalar"
	},
	[UNIFORM_ROW_COL_IDX] = {
		.uniform = UNIFORM_ROW_COL_IDX,
		.uniform_location = -1,
		.name = "row_col_index"
	},
	[UNIFORM_SIZE_2D] = {
		.uniform = UNIFORM_SIZE_2D,
		.uniform_location = -1,
		.name = "size_2d"
	},
	[UNIFORM_POS_2D] = {
		.uniform = UNIFORM_POS_2D,
		.uniform_location = -1,
		.name = "pos_2d"
	},
	[UNIFORM_SCALAR] = {
		.uniform = UNIFORM_SCALAR,
		.uniform_location = -1,
		.name = "scalar"
	},
	[UNIFORM_FONT_SIZE] = {
		.uniform = UNIFORM_FONT_SIZE,
		.uniform_location = -1,
		.name = "font_size"
	},
	[UNIFORM_TOP_LEFT] = {
		.uniform = UNIFORM_TOP_LEFT,
		.uniform_location = -1,
		.name = "topleft"
	},
	[UNIFORM_BOTTOM_RIGHT] = {
		.uniform = UNIFORM_BOTTOM_RIGHT,
		.uniform_location = -1,
		.name = "bottomright"
	}
};

u8 temp_bitmap[512 * 512];
u8 ttf_buffer[1 << 20];
stbtt_bakedchar cdata[96];

static void SetClearColor(GLRenderer* renderer, vec4 color) {
	memcpy(renderer->clearcolor, color, sizeof(vec4));
}

static void BeginRender(GLRenderer* renderer) {
	// Setup rendering commands
	// Sort them by texture ids
	// Push them into render queue

	glClearColor(renderer->clearcolor[0],
		renderer->clearcolor[1],
		renderer->clearcolor[2],
		renderer->clearcolor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void EndRender(GLRenderer* renderer) {
}

static IndexedStaticMesh CreateIndexedStaticQuad() {
	IndexedStaticMesh mesh;

	// Create buffer arrays
	glGenVertexArrays(1, &mesh.mesh_vao);
	glGenBuffers(1, &mesh.mesh_vbo);
	glGenBuffers(1, &mesh.mesh_ibo);
	mesh.num_indices = 6;

	// Set vertex array and index array.
	glBindVertexArray(mesh.mesh_vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(QuadVertex),
		k_quad_vtx_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mesh_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(u32),
		k_quad_indices, GL_STATIC_DRAW);

	// Set vertex attribute pointers
	// Vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(QuadVertex), (void*)0);
	// Textures
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(QuadVertex), (void*)offsetof(QuadVertex, uv));
	glBindVertexArray(0);

	return mesh;
}

static void ReleaseIndexedStaticMesh(IndexedStaticMesh* mesh) {
	glDeleteVertexArrays(1, &mesh->mesh_vao);
	glDeleteBuffers(1, &mesh->mesh_ibo);
	glDeleteBuffers(1, &mesh->mesh_vbo);
}

static void ApplyShader(GLShaderProgram program) {
	glUseProgram(program.handle);
}

static void ApplyTileTexture(GLRenderer* renderer, s8 tile_id) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->tile_textures[tile_id].texture_id);
}

static void Render(GLRenderer* renderer, IndexedStaticMesh* mesh) {
	glBindVertexArray(mesh->mesh_vao);
	glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, 0);
}

static void RenderQuad(GLRenderer* renderer) {
	Render(renderer, &renderer->internal_quad_mesh);
}

static void RenderTiledQuad(GLRenderer* renderer, GLShaderProgram* shader,
							mat4 ortho,
							f32 tile_scale_x, f32 tile_scale_y,
							s32 col_ind, s32 row_ind,
							f32 size_x, f32 size_y,
							f32 pos_x, f32 pos_y,
							f32 scalar_x, f32 scalar_y) {
	glUniformMatrix4fv(shader->m_uniforms[UNIFORM_ORTHO].uniform_location, 1, GL_FALSE, (GLfloat*)ortho);
	glUniform2f(shader->m_uniforms[UNIFORM_TILE_SCALAR].uniform_location, tile_scale_x, tile_scale_y);
	glUniform2i(shader->m_uniforms[UNIFORM_ROW_COL_IDX].uniform_location, col_ind, row_ind);
	glUniform2f(shader->m_uniforms[UNIFORM_SIZE_2D].uniform_location, size_x, size_y);
	glUniform2f(shader->m_uniforms[UNIFORM_POS_2D].uniform_location, pos_x, pos_y);
	glUniform2f(shader->m_uniforms[UNIFORM_SCALAR].uniform_location, scalar_x, scalar_y);
	RenderQuad(renderer);
}

static void RenderString(GLRenderer* renderer, mat4 ortho, char* str, f32 posx, f32 posy, f32 scale) {

	u32 shader_handle = renderer->font_shader.handle;
	ApplyShader(renderer->font_shader);

	GLShaderProgram* shader = &renderer->font_shader;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->font_texture.texture_id);

	while (*str) {
		char c = *str;
		f32 top = renderer->character_table[c].top;
		f32 left = renderer->character_table[c].left;
		f32 bottom = renderer->character_table[c].bottom;
		f32 right = renderer->character_table[c].right;

		glUniform1f(shader->m_uniforms[UNIFORM_FONT_SIZE].uniform_location, scale);
		glUniform2f(shader->m_uniforms[UNIFORM_TOP_LEFT].uniform_location, left, top);
		glUniform2f(shader->m_uniforms[UNIFORM_BOTTOM_RIGHT].uniform_location, right, bottom);
		glUniformMatrix4fv(shader->m_uniforms[UNIFORM_ORTHO].uniform_location, 1, GL_FALSE, (GLfloat*)ortho);
		glUniform2f(shader->m_uniforms[UNIFORM_POS_2D].uniform_location, posx, posy);
		glUniform2f(shader->m_uniforms[UNIFORM_SCALAR].uniform_location, scale, scale);
		posx += scale;
		RenderQuad(renderer);
		++str;
	}
}

// This function assumes image_buffer passed in fits perfectly with the tile size
static s32 AppendToTileTexture(GLRenderer* renderer, s8 tile_id, s8* image_buffer) {
	u32 row_ind;
	u32 col_ind;
	s32 xoffset;
	s32 yoffset;

	const u32 max_num_tiles = renderer->tile_textures[tile_id].num_cols * renderer->tile_textures[tile_id].num_rows;
	u32 next_num_tiles = renderer->tile_textures[tile_id].cur_num_tiles + 1;
	
	if (next_num_tiles > max_num_tiles) {
		LOG_ERROR("Tiled texture %d is full.\n", tile_id);
		return -1;
	}

	col_ind = renderer->tile_textures[tile_id].cur_num_tiles % renderer->tile_textures[tile_id].num_cols;
	row_ind = renderer->tile_textures[tile_id].cur_num_tiles / renderer->tile_textures[tile_id].num_cols;

	xoffset = col_ind * renderer->tile_textures[tile_id].tile_width;
	yoffset = row_ind * renderer->tile_textures[tile_id].tile_height;

	glBindTexture(GL_TEXTURE_2D, renderer->tile_textures[tile_id].texture_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset,
		renderer->tile_textures[tile_id].tile_width, renderer->tile_textures[tile_id].tile_height,
		GL_RGB, GL_UNSIGNED_BYTE, image_buffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	renderer->tile_textures[tile_id].cur_num_tiles = next_num_tiles;
	return next_num_tiles - 1;
}

static const TileTexture* GetTileTextureInfo(GLRenderer* renderer, s8 tile_id) {
	TileTexture* textures = &renderer->tile_textures[tile_id];
	return textures;
}

static GLShaderProgram CompileShader(GLRenderer* renderer, GLShaderSource* source) {
	GLShaderProgram program;
	u32 vtx_shader_id, frag_shader_id;
	char output_log[256];
	s32 compiled;

	vtx_shader_id = glCreateShader(GL_VERTEX_SHADER);
	frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vtx_shader_id, 1, &source->vertex_src, NULL);
	glCompileShader(vtx_shader_id);

	glShaderSource(frag_shader_id, 1, &source->fragment_src, NULL);
	glCompileShader(frag_shader_id);

	glGetShaderiv(vtx_shader_id, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		glGetShaderInfoLog(vtx_shader_id, 512, NULL, output_log);
		LOG_ERROR("Shader failed to compile %s", output_log);
	}
	output_log[0] = '\0';
	glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		glGetShaderInfoLog(frag_shader_id, 512, NULL, output_log);
		LOG_ERROR("Shader failed to compile %s", output_log);
	}

	program.handle = glCreateProgram();
	glAttachShader(program.handle, vtx_shader_id);
	glAttachShader(program.handle, frag_shader_id);

	glLinkProgram(program.handle);

	glUseProgram(program.handle);

	for (UniformSlot slot = UNIFORM_ORTHO; slot < NUM_UNIFORM_SLOTS; ++slot) {
		program.m_uniforms[slot].name = k_uniform_defs[slot].name;
		program.m_uniforms[slot].uniform_location = glGetUniformLocation(program.handle, program.m_uniforms[slot].name);
	}

	glUseProgram(0);

	glDeleteShader(frag_shader_id);
	glDeleteShader(vtx_shader_id);

	return program;
}

static void ReleaseShader(GLShaderProgram program) {
	glDeleteProgram(program.handle);
}

void InitGLRenderer(GLRenderer* renderer, const TextureSize tile_sizes[MAX_TILE_TEXTURES]) {
	u32 fontid;

	renderer->BeginRender = BeginRender;
	renderer->EndRender = EndRender;
	renderer->CreateIndexedStaticQuad = CreateIndexedStaticQuad;
	renderer->ReleaseIndexedStaticMesh = ReleaseIndexedStaticMesh;
	renderer->SetClearColor = SetClearColor;
	renderer->Render = Render;
	renderer->RenderQuad = RenderQuad;
	renderer->ApplyShader = ApplyShader;
	renderer->ApplyTileTexture = ApplyTileTexture;
	renderer->AppendToTileTexture = AppendToTileTexture;
	renderer->GetTileTextureInfo = GetTileTextureInfo;
	renderer->CompileShader = CompileShader;
	renderer->ReleaseShader = ReleaseShader;
	renderer->RenderString = RenderString;
	renderer->RenderTiledQuad = RenderTiledQuad;

	renderer->internal_quad_mesh = renderer->CreateIndexedStaticQuad();

	renderer->info.max_tex_height = renderer->info.max_tex_width = MAX_TEXTURE_SIZE;
	u32 tex_size = renderer->info.max_tex_width;
	u32 tex_buffer_size = tex_size * tex_size * 3;

	// Create a image cache.
	for (s32 i = 0; i < MAX_TILE_TEXTURES; ++i) {
		renderer->tile_textures[i].cur_num_tiles = 0;

		u32* tex_id = &renderer->tile_textures[i].texture_id;
		renderer->tile_textures[i].tile_width = tile_sizes[i].width;
		renderer->tile_textures[i].tile_height = tile_sizes[i].height;
		renderer->tile_textures[i].max_width = tex_size;
		renderer->tile_textures[i].max_height = tex_size;
		renderer->tile_textures[i].num_rows = tex_size / renderer->tile_textures[i].tile_height;
		renderer->tile_textures[i].num_cols = tex_size / renderer->tile_textures[i].tile_width;

		glGenTextures(1, tex_id);
		glBindTexture(GL_TEXTURE_2D, *tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_size, tex_size,
			0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Load font texture first
	memset(renderer->character_table, 0, sizeof(renderer->character_table));
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	FILE* file = fopen("squre_stone.ttf", "rb");
	if (file) {
		fread(ttf_buffer, 1, 1 << 20, file);
		stbtt_BakeFontBitmap(ttf_buffer, 0, 32.0f, temp_bitmap, 512, 512, 32, 96, cdata);
		glGenTextures(1, &fontid);
		renderer->font_texture.texture_id = fontid;

		glBindTexture(GL_TEXTURE_2D, fontid);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		for (s32 c = 32; c < 128; ++c) {
			f32 x, y;
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(cdata, 512, 512, c - 32, &x, &y, &q, 1);

			// TODO: s, t values are bit off figure out why that is.
			renderer->character_table[c].top = q.t0 + 0.003f; // Some hack alighn texture coordinate correctly
			renderer->character_table[c].left = q.s0 + 0.003f;
			renderer->character_table[c].bottom = q.t1 + 0.003f;
			renderer->character_table[c].right = q.s1 + 0.003f;
			renderer->character_table[c].width = q.s1 - q.s0;
			renderer->character_table[c].height = q.t1 - q.t0;
		}
		fclose(file);
	}

	GLShaderSource source;
	source.vertex_src = FONT_VERTEX_SHADER_SRC;
	source.fragment_src = FONT_FRAGMENT_SHADER_SRC;

	// Compile font shader.
	renderer->font_shader = CompileShader(renderer, &source);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ReleaseGLRenderer(GLRenderer* renderer) {
	renderer->ReleaseIndexedStaticMesh(&renderer->internal_quad_mesh);

	renderer->BeginRender = NULL;
	renderer->EndRender = NULL;
	renderer->CreateIndexedStaticQuad = NULL;
	renderer->ReleaseIndexedStaticMesh = NULL;
	renderer->SetClearColor = NULL;
	renderer->Render = NULL;
	renderer->RenderQuad = NULL;
	renderer->ApplyShader = NULL;
	renderer->ApplyTileTexture = NULL;
	renderer->AppendToTileTexture = NULL;
	renderer->GetTileTextureInfo = NULL;
	renderer->CompileShader = NULL;
	renderer->ReleaseShader = NULL;
	renderer->RenderString = NULL;
	renderer->RenderTiledQuad = NULL;

	for (s32 i = 0; i < MAX_TILE_TEXTURES; ++i) {
		glDeleteTextures(1, &renderer->tile_textures[i].texture_id);
	}
	glDeleteTextures(1, &renderer->font_texture.texture_id);
}

#ifdef __cplusplus
}
#endif