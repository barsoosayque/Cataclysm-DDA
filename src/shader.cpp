#include <fstream>
#include <iostream>
#include "shader.h"
#include "debug.h"

#define dbg(x) DebugLog((x),D_SDL) << __FILE__ << ":" << __LINE__ << ": "

shader_context::shader_context() 
    : u_projection(-1)
    , u_texture(-1)
    , projection {{
        {  0.f,  0.f,  0.f,  0.f },
        {  0.f,  0.f,  0.f,  0.f },
        {  0.f,  0.f,  0.f,  0.f },
        { -1.f,  1.f,  0.f,  1.f }
    }} {}

shader_context::~shader_context() {
    glDeleteBuffers(vbo.size(), vbo.data());
    for (std::shared_ptr<shader>& shader : shaders_pool) {
        glDeleteProgram(shader->id);
    }
}

bool shader_context::init() {
	glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(SDL_GL_GetProcAddress("glCreateShader"));
	glDeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(SDL_GL_GetProcAddress("glDeleteProgram"));
	glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(SDL_GL_GetProcAddress("glShaderSource"));
	glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(SDL_GL_GetProcAddress("glCompileShader"));
	glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(SDL_GL_GetProcAddress("glGetShaderiv"));
	glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(SDL_GL_GetProcAddress("glGetShaderInfoLog"));
	glDeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>(SDL_GL_GetProcAddress("glDeleteShader"));
	glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(SDL_GL_GetProcAddress("glAttachShader"));
	glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(SDL_GL_GetProcAddress("glCreateProgram"));
	glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(SDL_GL_GetProcAddress("glLinkProgram"));
	glValidateProgram = reinterpret_cast<PFNGLVALIDATEPROGRAMPROC>(SDL_GL_GetProcAddress("glValidateProgram"));
	glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(SDL_GL_GetProcAddress("glUseProgram"));
	glBindAttribLocation = reinterpret_cast<PFNGLBINDATTRIBLOCATIONPROC>(SDL_GL_GetProcAddress("glBindAttribLocation"));
	glGetUniformLocation = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(SDL_GL_GetProcAddress("glGetUniformLocation"));
    glUniform1i = reinterpret_cast<PFNGLUNIFORM1IPROC>(SDL_GL_GetProcAddress("glUniform1i"));
    glUniformMatrix4fv = reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(SDL_GL_GetProcAddress("glUniformMatrix4fv"));
    glVertexAttribPointer = reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(SDL_GL_GetProcAddress("glVertexAttribPointer"));
    glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
                                                    SDL_GL_GetProcAddress("glEnableVertexAttribArray"));
    glDisableVertexAttribArray = reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(
                                                    SDL_GL_GetProcAddress("glDisableVertexAttribArray"));
    glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(SDL_GL_GetProcAddress("glGenVertexArrays"));
    glGenBuffers = reinterpret_cast<PFNGLGENBUFFERSPROC>(SDL_GL_GetProcAddress("glGenBuffers"));
    glDeleteBuffers = reinterpret_cast<PFNGLDELETEBUFFERSPROC>(SDL_GL_GetProcAddress("glDeleteBuffers"));
    glBindBuffer= reinterpret_cast<PFNGLBINDBUFFERPROC>(SDL_GL_GetProcAddress("glBindBuffer"));
    glBufferSubData= reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(SDL_GL_GetProcAddress("glBufferSubData"));
    glBufferData= reinterpret_cast<PFNGLBUFFERDATAPROC>(SDL_GL_GetProcAddress("glBufferData"));
    glGetVertexAttribiv = reinterpret_cast<PFNGLGETVERTEXATTRIBIVPROC>(SDL_GL_GetProcAddress("glGetVertexAttribiv"));

    glGenBuffers(vbo.size(), vbo.data());
    current_vbo = 0;
    vbo_sizes = { 0, 0, 0, 0, 0, 0, 0, 0 };

	return glCreateShader && glDeleteProgram && glShaderSource && glCompileShader
        && glGetShaderiv && glGetShaderInfoLog && glDeleteShader && glAttachShader
        && glCreateProgram && glLinkProgram && glValidateProgram && glUseProgram
        && glBindAttribLocation && glGetUniformLocation && glUniform1i
        && glUniformMatrix4fv && glVertexAttribPointer && glEnableVertexAttribArray
        && glDisableVertexAttribArray && glGenVertexArrays && glGenBuffers
        && glDeleteBuffers && glBindBuffer && glBufferSubData && glBufferData
        && glGetVertexAttribiv;
}

GLuint shader_context::compile_shader(const char *source, GLuint type) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint is_compiled;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &is_compiled );
	if( is_compiled != GL_TRUE ) {
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			GLchar *log = new GLchar[length];
			glGetShaderInfoLog(shader, length, &length, log);
            dbg( D_ERROR ) << "Shader failed to compile. Log:" << log;
			delete[] log;
		}
		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}

std::shared_ptr<shader> shader_context::compile_from_string(const char *vertex_source,
												            const char *fragment_source) {
	GLuint id = glCreateProgram(),
	       vertex_id = compile_shader(vertex_source, GL_VERTEX_SHADER),
	       fragment_id = compile_shader(fragment_source, GL_FRAGMENT_SHADER);

	if(vertex_id && fragment_id) {
		glAttachShader(id, vertex_id);
		glAttachShader(id, fragment_id);
        glBindAttribLocation(id, ATTRIBUTE_POSITION, "a_position");
        glBindAttribLocation(id, ATTRIBUTE_TEXCOORD, "a_texCoord");
		glLinkProgram(id);
		glValidateProgram(id);
		glDeleteShader(vertex_id);
		glDeleteShader(fragment_id);
	}

	if(id) {
        GLuint u_projection = glGetUniformLocation(id, "u_projection"),
               u_texture = glGetUniformLocation(id, "u_texture");
		shaders_pool.emplace_back(new shader{id, u_projection, u_texture});
		return shaders_pool.back();
	} else {
		return {};
	}
}

std::shared_ptr<shader> shader_context::compile_from_file(std::string vertex_path,
														  std::string fragment_path) {
	std::ifstream vertex_stream(vertex_path);
	std::string vertex_source(std::istreambuf_iterator<char>(vertex_stream),
							  (std::istreambuf_iterator<char>()));

	std::ifstream fragment_stream(fragment_path);
	std::string fragment_source(std::istreambuf_iterator<char>(fragment_stream),
   							    (std::istreambuf_iterator<char>()));

	return compile_from_string(vertex_source.data(), fragment_source.data());
}

bool shader_context::bind(const std::shared_ptr<shader> &program) {
	if(program->id) {
		glGetIntegerv(GL_CURRENT_PROGRAM, &old_shader_id);
        glGetIntegerv(GL_VIEWPORT, tmp.data());
        projection[0][0] = 2.0f / tmp[2];
        // mul [1][1] and [3][1] by -1 to switch FBO/Screen modes
        projection[1][1] = 2.0f / tmp[3];
        projection[3][1] = -1.0f;

		glUseProgram(program->id);
        glUniform1i(program->u_texture, 0);
        glUniformMatrix4fv(program->u_projection, 1, GL_FALSE, reinterpret_cast<GLfloat*>(projection.data()));

		return true;
	}
	return false;
}

bool shader_context::unbind() {
	if(old_shader_id) {
		glUseProgram(old_shader_id);
	}
	return true;
}

bool shader_context::copy_with_shader(SDL_Texture *tex, const std::shared_ptr<shader> &program,
					 				  const SDL_Rect* srcrect, const SDL_Rect* dstrect) {

	if( SDL_GL_BindTexture(tex, NULL, NULL) ) {
        dbg( D_ERROR ) << "Texture binding failed: " << SDL_GetError();
        return false;
    }
	if( !bind(program) ) {
        dbg( D_ERROR ) << "Shader binding failed";
        return false;
	}

	// Create source rect in UV coords
	GLfloat u1 = 0.0f, v1 = 0.0f, u2 = 1.0f, v2 = 1.0f;

	// Create destination rect in window coords
	GLfloat x1 = dstrect->x,      y1 = dstrect->y,
			x2 = x1 + dstrect->w, y2 = y1 + dstrect->h;
    shader_data[0] = x1; shader_data[1] = y1;
    shader_data[2] = x2; shader_data[3] = y1;
    shader_data[4] = x1; shader_data[5] = y2;
    shader_data[6] = x2; shader_data[7] = y2;
	
	if (srcrect) {
		int tex_w, tex_h;
		SDL_QueryTexture(tex, NULL, NULL, &tex_w, &tex_h);

		u1 = static_cast<float>(srcrect->x) / tex_w;
		v1 = static_cast<float>(srcrect->y) / tex_h;
		u2 = static_cast<float>(srcrect->x + srcrect->w) / tex_w;
		v2 = static_cast<float>(srcrect->y + srcrect->h) / tex_h;
	}
    shader_data[8]  = u1; shader_data[9]  = v1;
    shader_data[10] = u2; shader_data[11] = v1;
    shader_data[12] = u1; shader_data[13] = v2;
    shader_data[14] = u2; shader_data[15] = v2;
    
	// Render
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableClientState(GL_VERTEX_ARRAY);

    int vertsize = shader_data.size() * sizeof(GLfloat);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[current_vbo]);
    if (vbo_sizes[current_vbo] < vertsize) {
        glBufferData(GL_ARRAY_BUFFER, vertsize, shader_data.data(), GL_STREAM_DRAW);
        vbo_sizes[current_vbo] = vertsize;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertsize, shader_data.data());
    }

    current_vbo++;
    if (current_vbo >= vbo.size()) {
        current_vbo = 0;
    }

    // Since SDL renderer might use attributes with the same index, we must 
    // check them and keep them enabled because SDL renderer will not re-enable them
    GLint was_position_enabled, was_texcoord_enabled;
    glGetVertexAttribiv(ATTRIBUTE_POSITION, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &was_position_enabled);
    glGetVertexAttribiv(ATTRIBUTE_TEXCOORD, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &was_texcoord_enabled);

    glVertexAttribPointer(ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(ATTRIBUTE_POSITION);
    glVertexAttribPointer(ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(ATTRIBUTE_TEXCOORD);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (!was_position_enabled) {
        glDisableVertexAttribArray(ATTRIBUTE_POSITION);
    }
    if (!was_texcoord_enabled) {
        glDisableVertexAttribArray(ATTRIBUTE_TEXCOORD);
    }

	unbind();	
	SDL_GL_UnbindTexture(tex);
	return true;
}
