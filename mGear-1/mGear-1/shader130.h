#ifndef SHADER130_H
#define SHADER130_H

static const char *Texture_VShader[64]={
	"#version 330\n"

	"in vec3 Position;\n"
	"in vec2 TexCoord;\n"
	"in vec4 Color;\n"
	"in vec2 TexLight;\n"

	"out vec2 TexCoord2;\n"
	"out vec4 colore;\n"
	"out vec2 TexLight2;\n"
	"out vec3 Pos;\n"

	"void main()\n"
	"{\n"
		"gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);\n"
		"TexCoord2 = TexCoord;\n"
		"colore = Color;\n"
		"TexLight2 = TexLight;\n"
		"Pos = Position;\n"
	"}\n"
};

static const char *Texture_FShader[64]={
	"#version 130\n"

	"precision mediump float;\n"

	"uniform sampler2D texu;\n"

	"uniform int light_type;"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"out vec4 FColor;\n"

	"void main()\n"
	"{\n"
	"if( light_type == 0)\n"
		"FColor += texture(texu, TexCoord2);\n"
	"else\n"
	"if( light_type == 1)\n"
		"FColor += texture(texu, TexCoord2) * 2.0;\n"
	"else\n"
	"if( light_type == 2)\n"
		"FColor += texture(texu, TexCoord2) * 4.0;\n"
	"}"

};

static const char *TextureNoT_FShader[64]={
	"#version 130\n"

	"precision mediump float;\n"

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
	"}\n"

};

static const char *TextureT_FShader[64]={
	"#version 130\n"

	"precision mediump float;\n"

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

		
	"}\n"

};

static const char *Blend_FShader[64]={
	"#version 130\n"

	"precision mediump float;\n"

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
	"}\n"

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
/*
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
*/

static const char *Lightmap_FShader[128]={
	"#version 330\n"

	"precision mediump float;\n"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"in vec2 TexLight2;\n"

	"in vec3 Pos;\n"

	"uniform vec3 Lightpos;\n"

	"layout (location = 0) out vec4 FColor;\n"
	"layout (location = 1) out vec4 NColor;\n"
	"layout (location = 2) out vec4 Amb;\n"
	"layout (location = 3) out vec4 Mask;\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"uniform sampler2D texu3;\n"

	"uniform sampler2D texu4;\n"

	"uniform sampler2D texu5;\n"

	"uniform float normal;\n"

	"uniform vec3 falloff;\n"
	"uniform vec2 res;\n"
	"uniform float spotcos;\n"
	"uniform float spotinnercos;\n"
	"uniform vec2 spotdir;\n"

	"uniform vec2 camp;\n"
	"uniform vec2 cams;\n"

	"uniform float shadow;\n"
	"uniform float sector;\n"
	"uniform float sector_y;\n"
	"uniform float sector_y_2;\n"
	"uniform int circle;\n"

	"void main()\n"
	"{\n"
		"if(normal == 0.0)\n"
		"{\n"
			"vec4 alpha = vec4(1.0);\n"

			"if(circle == 1)\n"
			"{\n"
				"vec2 d = 2.0 * TexCoord2.xy - 1.0;\n"
				"float r = dot(d, d);\n"
				"float delta = fwidth(r);\n"
				"alpha = vec4(vec3(1.0), 1.0 - smoothstep(1.0 - delta, 1.0 + delta, r));\n"
			"}\n"
			
			"FColor = texture(texu, TexCoord2) * colore * alpha;\n"
		"}\n"
		"else\n"
		"if(normal == 1.0)\n"
		"{\n"
			"vec4 Lightmap = texture(texu2, TexLight2) * 4.0;\n"

			"vec3 L = normalize(Lightmap.rgb);\n"

			"vec3 N = normalize(texture(texu3, TexCoord2).rgb * 2.0 - 1.0);\n"

			"FColor = texture(texu, TexCoord2) * colore + (Lightmap * max(dot(N, L), 0.0));\n"
		"}\n"
		"else\n"
		"if(normal == 2.0)\n"
		"{\n"
		//"vec4 Lightmap = texture(texu2, TexLight2);\n"
			"vec4 Diff = texture(texu, TexCoord2);\n"
			"FColor = vec4(Diff.rgb, Diff.a * colore.a);\n"
			"NColor = vec4(texture(texu3, TexCoord2).rgb, texture(texu, TexCoord2).a);\n"
			"Amb = colore;\n"
		"}\n"
		"else\n"
		"if(normal == 3.0)\n"
		"{\n"
			"NColor = vec4(texture(texu3, TexCoord2).rgb, texture(texu, TexCoord2).a);\n"
			"FColor = texture(texu, TexCoord2);\n"
			"Mask = vec4(0, 0, 0, texture(texu, TexCoord2).a);\n"
		"}\n"
		"else\n"
		"if(normal == 4.0)\n"
		"{\n"
			"vec3 L = normalize(texture(texu4, TexCoord2).rgb);\n"

			"vec3 N = normalize(texture(texu3, TexCoord2).rgb * 2.0 - 1.0);\n"
			"FColor = texture(texu, TexCoord2) * (texture(texu2, TexCoord2) + ((texture(texu4, TexCoord2)) * max(dot(N, L), 0.0)));\n"
		"}\n"
		"else\n"
		"if(normal == 5.0)\n"
		"{\n"
			"vec3 LightDir = vec3(Lightpos.xy - gl_FragCoord.xy, Lightpos.z);\n"
			//"LightDir.y *= res.x / res.y;\n"

			"float D = length(LightDir);\n"

			"vec3 Ambient = texture(texu2, TexLight2).rgb;\n"

			//calculate attenuation
			//"float Attenuation = clamp((1.0 / (0.01 + (falloff.x * D))) - 0.01, 0.0, 10);\n"
			"float Attenuation = 1.0f / (((D / (falloff.x * cams.x)) + 1.0f) * (256.0f / falloff.y));\n"
			"Attenuation = clamp(((Attenuation - falloff.z) / (1.0f - Attenuation)), 0.0, 2.0);\n"
			//"Attenuation = clamp(falloff.y / pow((D / falloff.x) + 1, 2), 0.0, falloff.y); \n"
			//"float Attenuation = pow(smoothstep(falloff.x, 0, D), falloff.y);\n"

			//the calculation which brings it all together
			//"vec3 Intensity = Ambient + Attenuation;\n"
			"vec3 FinalColor = Attenuation * colore.rgb;\n"

			"FColor = vec4(FinalColor, 1.0);\n"
			//"NColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"}\n"
		"else\n"
		"if(normal == 6.0)\n"
		"{\n"
			//"NColor = vec4(0.0, 0.0 , 0.0, 1.0);\n"
			//"Amb = vec4(1.0, 0.0 , 0.0, 1.0);\n"

			"if(sector == 1 && gl_FragCoord.y < sector_y - 1) discard;\n"
			"if(sector == 2 && gl_FragCoord.y < sector_y - 1) discard;\n"
			"if(sector == 2 && gl_FragCoord.y > sector_y_2) discard;\n"

			"if(shadow == 0)\n"
				"FColor = vec4(0, 0, 0, clamp(texture(texu5, TexCoord2, 1) - texture(texu4, TexLight2).a, 0.0, 1.0));\n"
			"else\n"
				"FColor = vec4(0, 0, 0, clamp(texture(texu5, TexCoord2, 1) - texture(texu4, TexLight2).a, 0.0, 1.0));\n"
		"}\n"
		"else\n"
		"if(normal == 7.0)\n"
		"{\n"
			"FColor = vec4(0, 0, 0, 1.0 - texture(texu4, TexCoord2).a);\n"
		"}\n"
		"else\n"
		"if(normal == 8.0)\n"
		"{\n"
			"vec3 LightDir = vec3(Lightpos.xy - gl_FragCoord.xy, Lightpos.z);\n"
			"vec3 SpotLightDir = vec3(Lightpos.xy - spotdir.xy, Lightpos.z);\n"

			"float SpotEffect = dot(normalize(LightDir), normalize(SpotLightDir));\n"

			"if(SpotEffect > spotcos)\n"
			"{\n"

				"float SpotAtt = spotinnercos - spotcos;\n"//smoothstep(0.7, 0.99, SpotEffect);\n"
				"SpotAtt = clamp((SpotEffect - spotcos) / SpotAtt, 0.0, 1.0);\n"

				"float D = length(LightDir);\n"

				"vec3 Ambient = texture(texu2, TexLight2).rgb;\n"

				//calculate attenuation
				"float Attenuation = SpotAtt * (1.0 / (((D / (falloff.x * cams.x)) + 1.0f) * (256.0f / falloff.y)));\n"
				"Attenuation = clamp(((Attenuation / falloff.z) / (1.0f - Attenuation)), 0.0, 2.0);\n"
				//"Attenuation = clamp(falloff.y / pow((D / falloff.x) + 1, 2), 0.0, falloff.y); \n"

				//the calculation which brings it all together
				"vec3 FinalColor = Attenuation * colore.rgb;\n"

				"FColor = vec4(FinalColor, 1.0);\n"
			"}\n"
		"}\n"
	"}\n"
};

#endif