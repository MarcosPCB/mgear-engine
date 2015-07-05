#ifndef _SHADER110_C
#define _SHADER110_C

const char *Texture_VShader110[64]={
	"#version 110\n"

	"varying in vec3 Position;\n"
	"varying in vec2 TexCoord;\n"
	"varying in vec4 Color;\n"

	"varying out vec2 TexCoord2;\n"
	"varying out vec4 colore;\n"

	"void main()\n"
	"{\n"
	   "gl_Position = vec4(Position, 1.0);\n"
	   "TexCoord2 = TexCoord;\n"
	   "colore = Color;\n"
	"};\n"
};

const char *Texture_FShader110[64]={
	"#version 110\n"

	"uniform sampler2D texu;\n"

	"uniform float normal;\n"

	"varying in vec2 TexCoord2;\n"

	"varying in vec4 colore;\n"

	"varying out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		//"vec4 NormalColor = vec4(0.501, 0.501, 1.0, 1.0);\n"

		"if(normal == 0.0 || normal == 1.0)\n"
			"FColor = texture2D(texu,TexCoord2) * colore;\n"
/*
		"else\n"
		"if(normal == 2.0)\n"
			"FColor = NormalColor;\n"
			*/
	"};\n"

};

const char *TextureNoT_FShader110[64]={
	"#version 110\n"

	"uniform sampler2D texu;\n"

	"varying in vec2 TexCoord2;\n"

	"varying in vec4 colore;\n"

	"varying out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		"vec4 Diffuse = texture2D(texu, TexCoord2);\n"

		"if(Diffuse.a < 1.0)\n"
			"discard;\n"
		//"else\n"
		"FColor = Diffuse;\n"
	"};\n"

};

const char *TextureT_FShader110[64]={
	"#version 110\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"varying in vec2 TexCoord2;\n"

	"varying in vec4 colore;\n"

	"varying out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		"vec4 Diffuse = texture2D(texu, TexCoord2) * colore;\n"
		"vec4 Bckg = texture2D(texu2, TexCoord2) * colore;\n"

		"if(Diffuse.a == 1.0)\n"
			"discard;\n"
		//"else\n"
		"if(Diffuse.a < 1.0 && Diffuse.a > 0.5)\n"
		"{\n"
			"FColor = vec4((Diffuse.a * Diffuse.rgb) * Bckg.rgb, 1.0);\n"
		"}\n"

		
	"};\n"

};

const char *Blend_FShader110[64]={
	"#version 110\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"varying in vec2 TexCoord2;\n"

	"varying in vec4 colore;\n"

	"varying out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		"vec4 Diffuse = texture2D(texu, TexCoord2);\n"
		"vec4 Alpha = texture2D(texu2, TexCoord2);\n"

		"FColor = vec4(Alpha.rgb * Diffuse.rgb,1.0);\n"
	"};\n"

};

const char *Lightmap_FShader110[128]={
	"#version 110\n"
	
	"varying in vec2 TexCoord2;\n"

	"varying in vec4 colore;\n"

	"varying out vec4 FColor;\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"uniform sampler2D texu3;\n"

	"uniform float normal;\n"

	"void main()\n"
	"{\n"
		"vec4 Lightmap = texture2D(texu2, TexCoord2);\n"

		"vec3 L = normalize(Lightmap.rgb);\n"

		"if(normal == 1.0)\n"
		"{\n"
			"vec3 N = normalize((texture2D(texu3, TexCoord2).rgb) * 2.0 - 1.0);\n"

			"FColor = (texture2D(texu, TexCoord2) * colore) * ((Lightmap) * max(dot(N, L), 0.0));\n"
		"}\n"
		"else\n"
		"{\n"
			"vec3 NormalColor = vec3(0.501, 0.501, 1.0);\n"

			"FColor = (texture2D(texu, TexCoord2) * colore) * ((Lightmap) * max(dot(NormalColor, L), 0.0));\n"
		"}\n"
	"}\n"
};

#endif