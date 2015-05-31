#ifndef _SHADER130_C
#define _SHADER130_C

const char *Texture_VShader[64]={
	"#version 130\n"

	"in vec3 Position;\n"
	"in vec2 TexCoord;\n"
	"in vec4 Color;\n"

	"out vec2 TexCoord2;\n"
	"out vec4 colore;\n"

	"void main()\n"
	"{\n"
	   "gl_Position = vec4(Position, 1.0);\n"
	   "TexCoord2 = TexCoord;\n"
	   "colore = Color;\n"
	"};\n"
};

const char *Texture_FShader[64]={
	"#version 130\n"

	"uniform sampler2D texu;\n"

	"uniform float normal;"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		"vec4 NormalColor = vec4(0.501, 0.501, 1.0, 1.0);\n"

		"vec4 Diffuse = texture(texu, TexCoord2);\n"
		
		"if(normal == 0 || normal == 1)\n"
			"FColor = Diffuse;\n"
		"else\n"
		"if(normal == 2)\n"
			"FColor = NormalColor;\n"
	"};"

};

const char *TextureNoT_FShader[64]={
	"#version 130\n"

	"uniform sampler2D texu;\n"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		"vec4 Diffuse = texture(texu, TexCoord2) * colore;\n"

		"if(Diffuse.a < 1.0)\n"
			"discard;\n"
		"else\n"
		"FColor = Diffuse;\n"
	"};\n"

};

const char *TextureT_FShader[64]={
	"#version 130\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		"vec4 Diffuse = texture(texu, TexCoord2) * colore;\n"
		"vec4 Bckg = texture(texu2, TexCoord2);\n"

		"if(Diffuse.a == 1.0)\n"
			"discard;\n"
		//"else\n"
		//"if(Diffuse.a < 0.5)\n"
			//"discard;\n"
		"else\n"
		//"if(Diffuse.a < 1.0 && Diffuse.a > 0.4)\n"
		"{\n"
			"FColor = vec4((Diffuse.a * Diffuse.rgb) + ((1.0 - Diffuse.a) * Bckg.rgb), 1.0);\n"
		"}\n"

		
	"};\n"

};

const char *Blend_FShader[64]={
	"#version 130\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"void main()\n"
	"{\n"
		"vec4 Diffuse = texture(texu, TexCoord2);\n"
		"vec4 Alpha = texture(texu2, TexCoord2);\n"

		"FColor = vec4(Alpha.rgb * Diffuse.rgb,1.0);\n"
	"};\n"

};

/*
const char *Normal_FShader[128]={
	"#version 130\n"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"uniform sampler2D texu;\n"

	//"uniform sampler2D texu2;\n"

	"uniform vec3 LightPos;\n"

	"uniform vec4 LightColor;\n"

	"uniform float Falloff;\n"

	"uniform vec2 Screen;\n"

	"void main()\n"
	"{\n"
		"vec4 DiffuseColor = texture(texu, TexCoord2);\n"

		//"vec3 NormalMap = texture(texu2, TexCoord2).rgb;\n"

		"vec3 LightDir = vec3(LightPos.xy - (gl_FragCoord.xy / Screen.xy), LightPos.z);\n"

		"LightDir.x *= Screen.x / Screen.y;\n"

		"float D = length(LightDir);\n"

		"vec3 L = normalize(LightDir);\n"

		//"vec3 N = normalize(NormalMap * 2.0 - 1.0);\n"

		"float Att = 1.0 / (Falloff*D);\n"

		"vec3 df = (LightColor.rgb * LightColor.a);\n" //* max(dot(N, L), 0.0);\n"

		"vec3 it = (DiffuseColor.rgb + df) * Att;\n"

		"vec3 FinalC = DiffuseColor.rgb * it;\n"

		"FColor = vec4(FinalC,DiffuseColor.a);\n"
	"}\n"
};

const char *Lighting_FShader[196]={
	"#version 130\n"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"uniform vec3 LightPos;\n"

	"uniform vec4 LightColor;\n"

	"uniform float Falloff;\n"

	"uniform vec2 Screen;\n"

	"uniform float Radius;\n"

	"void main()\n"
	"{\n"
		"vec3 LightDir = vec3(LightPos.xy - (gl_FragCoord.xy / Screen.xy), LightPos.z);\n"

		"LightDir.x *= Screen.x / Screen.y;\n"

		"float D = length(LightDir);\n"

		"vec4 DiffuseColor = texture(texu, TexCoord2);\n"

		"vec4 LightMap = texture(texu2, TexCoord2);\n"

		"float Att = 1.0 / (Falloff*D);\n"

		"vec3 df = LightColor.rgb * LightColor.a;\n"

		"vec3 it =   df * Att;\n"

		"vec3 FinalC = DiffuseColor.rgb * (it + LightMap.rgb);\n"

		"FColor = vec4(FinalC,DiffuseColor.a);\n"
	"}\n"
};
*/

const char *Lightmap_FShader[128]={
	"#version 130\n"
	
	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"uniform sampler2D texu3;\n"

	"void main()\n"
	"{\n"
		"vec4 DiffuseColor = texture(texu, TexCoord2);\n"

		"vec4 Lightmap = texture(texu2, TexCoord2);\n"

		"vec3 NormalMap = texture(texu3, TexCoord2).rgb;\n"

		"vec3 L = normalize(Lightmap.rgb);\n"

		"vec3 N = normalize(NormalMap * 2.0 - 1.0);\n"

		"FColor = DiffuseColor * (Lightmap);\n" //* max(dot(N, L), 0.0));\n"

	"}\n"
};

#endif