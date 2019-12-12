#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <memory>

#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "sdl_wrappers.h"

/// Shader programm with associated vertex and fragment shaders.
struct shader {
	GLuint id;
};

/// Facility to load, create and use GLSL shaders.
/// todo: destruction
class shader_context {
	private:
		GLuint old_shader_id;
		std::vector<std::shared_ptr<shader>> shaders_pool;

		// GL extensions loaded at runtime
		PFNGLCREATESHADERPROC glCreateShader;
		PFNGLSHADERSOURCEPROC glShaderSource;
		PFNGLCOMPILESHADERPROC glCompileShader;
		PFNGLGETSHADERIVPROC glGetShaderiv;
		PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
		PFNGLDELETESHADERPROC glDeleteShader;
		PFNGLATTACHSHADERPROC glAttachShader;
		PFNGLCREATEPROGRAMPROC glCreateProgram;
		PFNGLLINKPROGRAMPROC glLinkProgram;
		PFNGLVALIDATEPROGRAMPROC glValidateProgram;
		PFNGLGETPROGRAMIVPROC glGetProgramiv;
		PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
		PFNGLUSEPROGRAMPROC glUseProgram;

		GLuint compile_shader(const char *source, GLuint type);

	public:

		/// @return false if GL extensions are not available
		/// or some extension functions are not defined.
		bool init();

		/// Compile a new shader program from provided strings
		std::shared_ptr<shader> compile_from_string(const char *vertex_source,
												    const char *fragment_source);

		/// Compile a new shader program from files content.
		std::shared_ptr<shader> compile_from_file(std::string vertex_path,
												  std::string fragment_path);

		/// Start using a shader.
		bool bind(const shader &program);

		/// Restore previous shader usage.
		bool unbind();

		/// Utility method to mimic SDL renderer copy method but
		/// perform the draw with a shader.
		bool copy_with_shader(SDL_Texture *tex, const shader &program,
                              const SDL_Rect* srcrect, const SDL_Rect* dstrect);
};

#endif
