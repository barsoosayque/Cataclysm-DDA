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
	GLuint id, u_projection, u_texture;
};

/// Facility to load, create and use GLSL shaders.
class shader_context {
	private:
		GLint old_shader_id;
		std::vector<std::shared_ptr<shader>> shaders_pool;

		// GL extensions loaded at runtime
		PFNGLCREATESHADERPROC glCreateShader;
		PFNGLDELETEPROGRAMPROC glDeleteProgram;
		PFNGLSHADERSOURCEPROC glShaderSource;
		PFNGLCOMPILESHADERPROC glCompileShader;
		PFNGLGETSHADERIVPROC glGetShaderiv;
		PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
		PFNGLDELETESHADERPROC glDeleteShader;
		PFNGLATTACHSHADERPROC glAttachShader;
		PFNGLCREATEPROGRAMPROC glCreateProgram;
		PFNGLLINKPROGRAMPROC glLinkProgram;
		PFNGLVALIDATEPROGRAMPROC glValidateProgram;
		PFNGLUSEPROGRAMPROC glUseProgram;
		PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
		PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
		PFNGLUNIFORM1IPROC glUniform1i;
        PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
        PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
        PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv;
        PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
        PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
        PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
        PFNGLGENBUFFERSPROC glGenBuffers;
        PFNGLDELETEBUFFERSPROC glDeleteBuffers;
        PFNGLBINDBUFFERPROC glBindBuffer;
        PFNGLBUFFERDATAPROC glBufferData;
        PFNGLBUFFERSUBDATAPROC glBufferSubData;

		GLuint compile_shader(const char *source, GLuint type);
        
        enum {
            ATTRIBUTE_POSITION = 0,
            ATTRIBUTE_TEXCOORD = 1,
        };

        GLint u_projection,
              u_texture;
        std::array<std::array<GLfloat, 4>, 4> projection;
        std::array<GLint, 4> tmp;
        std::array<GLfloat, 16> shader_data;
        
        unsigned int current_vbo;
        std::array<GLuint, 8> vbo;
        std::array<int, 8> vbo_sizes;

	public:
        shader_context();
        ~shader_context();

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
		bool bind(const std::shared_ptr<shader> &program);

		/// Restore previous shader usage.
		bool unbind();

		/// Utility method to mimic SDL renderer copy method but
		/// perform the draw with a shader.
		bool copy_with_shader(SDL_Texture *tex, const std::shared_ptr<shader> &program,
                              const SDL_Rect* srcrect, const SDL_Rect* dstrect);
};

#endif
