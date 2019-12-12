#include <fstream>
#include <iostream>

#include "shader.h"
#include "debug.h"

bool shader_context::init() {
	glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(SDL_GL_GetProcAddress("glCreateShader"));
	glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(SDL_GL_GetProcAddress("glShaderSource"));
	glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(SDL_GL_GetProcAddress("glCompileShader"));
	glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(SDL_GL_GetProcAddress("glGetShaderiv"));
	glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(SDL_GL_GetProcAddress("glGetShaderInfoLog"));
	glDeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>(SDL_GL_GetProcAddress("glDeleteShader"));
	glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(SDL_GL_GetProcAddress("glAttachShader"));
	glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(SDL_GL_GetProcAddress("glCreateProgram"));
	glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(SDL_GL_GetProcAddress("glLinkProgram"));
	glValidateProgram = reinterpret_cast<PFNGLVALIDATEPROGRAMPROC>(SDL_GL_GetProcAddress("glValidateProgram"));
	glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(SDL_GL_GetProcAddress("glGetProgramiv"));
	glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(SDL_GL_GetProcAddress("glGetProgramInfoLog"));
	glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(SDL_GL_GetProcAddress("glUseProgram"));

	return glCreateShader && glShaderSource && glCompileShader && glGetShaderiv && 
		glGetShaderInfoLog && glDeleteShader && glAttachShader && glCreateProgram &&
		glLinkProgram && glValidateProgram && glGetProgramiv && glGetProgramInfoLog &&
		glUseProgram;
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
        	DebugLog( D_ERROR, D_SDL ) << "Shader failed to compile. Log:" << log << std::endl;
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
		glLinkProgram(id);
		glValidateProgram(id);
		glDeleteShader(vertex_id);
		glDeleteShader(fragment_id);
	}

	if(id) {
		shaders_pool.emplace_back(new shader{id});
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

bool shader_context::bind(const shader &program) {
	if(program.id) {
		glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&old_shader_id));
		glUseProgram(program.id);
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

bool shader_context::copy_with_shader(SDL_Texture *tex, const shader &program,
					 				  const SDL_Rect* srcrect, const SDL_Rect* dstrect) {

	if( !bind(program) ) {
		DebugLog( D_WARNING, D_SDL ) << "Shader binding failed";
	}

	// Create source rect in UV coords
	GLfloat u1 = 0, v1 = 0, u2 = 1, v2 = 1;
	
	if (srcrect) {
		int tex_w, tex_h;
		SDL_QueryTexture(tex, NULL, NULL, &tex_w, &tex_h);

		u1 = static_cast<float>(srcrect->x) / tex_w,
		v1 = static_cast<float>(srcrect->y) / tex_h,
		u2 = u1 + (static_cast<float>(srcrect->w) / tex_w), 
		v2 = v1 + (static_cast<float>(srcrect->h) / tex_h);
	}

	// Create destination rect in window coords
	GLfloat x1 = dstrect->x,      y1 = dstrect->y,
			x2 = x1 + dstrect->w, y2 = y1 + dstrect->h;

	// Render
	// glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	SDL_GL_BindTexture(tex, NULL, NULL);

	glBegin(GL_TRIANGLE_STRIP);

	glTexCoord2f(u1, v1);
	glVertex2f(x1, y1);
	glTexCoord2f(u2, v1);
	glVertex2f(x2, y1);

	glTexCoord2f(u1, v2);
	glVertex2f(x1, y2);
	glTexCoord2f(u2, v2);
	glVertex2f(x2, y2);

	glDisable(GL_TEXTURE_2D);
	glEnd();
	glFlush();
		
	unbind();	
	return true;
}
