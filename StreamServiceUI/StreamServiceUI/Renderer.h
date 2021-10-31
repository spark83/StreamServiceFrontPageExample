#ifndef RENDERER_H
#define RENDERER_H

#define MAX_TILE_TEXTURES  1
#define MAX_TEXTURE_SIZE   8192 //4096
#define MAX_NUM_CHARACTERS 128

#ifdef __cplusplus
extern "C" {
#endif

// Tile specific vertex. No need for normals since no normal based special effect is needed.
typedef struct {
	vec3 vertex;
	vec2 uv;
} QuadVertex;

// This will be used primarly.
typedef struct {
	QuadVertex vertices[4];
	u32 indices[6]; // Only 6 indices
} IndexedQuadMeshData;

// Some generic opengl info stuff to make sure which version to use
// currently uses opengl 3.2 
// Nothing is done here for now other than getting the info.
// TODO: Based on the info gathered from here, use supported opengl
// version later.
typedef struct {
	s8* vender;
	s8* version;
	s8* shading_language_version;

	u32 max_tex_width;
	u32 max_tex_height;
} GLInfoData;

typedef struct {
	u32 mesh_vao;
	u32 mesh_vbo;
	u32 mesh_ibo;
	u32 num_indices;
} IndexedStaticMesh;

typedef struct {
	u32 texture_id;
} FontTexture;

typedef struct {
	f32 top, left;
	f32 bottom, right;
	f32 width, height;
} FontCharacter;

typedef struct {
	u32 width, height;
} TextureSize;

// Append all texture into one texture as long as size permits it
// and use that to display image using offset of the texture.
typedef struct {
	u32 texture_id;
	u32 tile_width, tile_height;
	u32 max_width, max_height;
	u32 cur_num_tiles;
	u32 num_cols;
	u32 num_rows;
} TileTexture;

typedef struct {
	u32 texture_id;
} Texture;

// These are the only uniforms being used.
// No need to know if there is something more than this.
// No need to go cause there is nothing more than this.
typedef enum {
	UNIFORM_ORTHO = 0,
	UNIFORM_TILE_SCALAR,
	UNIFORM_ROW_COL_IDX,
	UNIFORM_SIZE_2D,
	UNIFORM_POS_2D,
	UNIFORM_SCALAR,
	UNIFORM_FONT_SIZE,
	UNIFORM_TOP_LEFT,
	UNIFORM_BOTTOM_RIGHT,
	UNIFORM_OPACITY,
	NUM_UNIFORM_SLOTS
} UniformSlot;

// No need for geometry shader.
typedef struct {
	s8* vertex_src;
	s8* fragment_src;
} GLShaderSource;

typedef struct {
	UniformSlot uniform;
	s32 uniform_location;
	const s8* name;
} UniformInfo;

typedef struct {
	u32 handle;  // Shader program id.
	UniformInfo m_uniforms[NUM_UNIFORM_SLOTS];
} GLShaderProgram;

typedef struct _GLRenderer {
	GLInfoData info;
	vec4 clearcolor;

	u8 num_tile_textures;
	TileTexture tile_textures[MAX_TILE_TEXTURES];

	// Quad mesh that is reused in all quad related rendering.
	IndexedStaticMesh internal_quad_mesh;

	// Only font texture in use
	FontTexture font_texture;
	GLShaderProgram font_shader;
	// TODO: Add support for internationalization
	FontCharacter character_table[MAX_NUM_CHARACTERS];

	void (*SetClearColor)(struct _GLRenderer*, vec4 color);
	IndexedStaticMesh (*CreateIndexedStaticQuad)();
	void (*ReleaseIndexedStaticMesh) (IndexedStaticMesh*);
	void (*BeginRender) (struct _GLRenderer*);
	void (*EndRender) (struct _GLRenderer*);

	void (*Render) (struct _GLRenderer*, IndexedStaticMesh*);

	void (*RenderQuad) (struct _GLRenderer*, GLShaderProgram*, 
						mat4, f32, f32, f32, f32, f32, f32, f32);

	void (*BeginRenderTileQuad) (struct _GLRenderer*, GLShaderProgram*, s8);
	void (*RenderTiledQuad) (struct _GLRenderer*, GLShaderProgram*,
							 mat4,
							 f32, f32,
							 s32, s32,
							 f32, f32,
							 f32, f32,
							 f32, f32);
	void (*BeginRenderString) (struct _GLRenderer*);
	void (*RenderString) (struct _GLRenderer*, mat4, char*, f32, f32, f32);

	void (*CreateTexture) (Texture*, u8*, s32, s32);

	void (*ApplyShader) (GLShaderProgram);
	void (*ApplyTileTexture) (struct _GLRenderer*, s8);
	void (*ApplyTexture) (Texture*);

	s32 (*AppendToTileTexture) (struct _GLRenderer*, s8, s8*);
	s8 (*IsTileTextureFull) (struct _GLRenderer*, s8);
	const TileTexture* (*GetTileTextureInfo) (struct _GLRenderer*, s8);

	GLShaderProgram (*CompileShader) (struct _GLRenderer*, GLShaderSource*);
	void (*ReleaseShader) (GLShaderProgram);

} GLRenderer;

void InitGLRenderer(GLRenderer* renderer, const TextureSize tile_sizes[MAX_TILE_TEXTURES]);
void ReleaseGLRenderer(GLRenderer* renderer);

#ifdef __cplusplus
}
#endif

#endif
