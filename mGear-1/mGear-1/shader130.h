#ifndef SHADER130_H
#define SHADER130_H

static const char *Texture_VShader[128]={
	"#version 330\n"

	"in vec3 Position;\n"
	"in vec2 TexCoord;\n"
	"in vec4 Color;\n"
	"in vec2 TexLight;\n"
	"in float LBlock;\n"

	"out vec2 TexCoord2;\n"
	"out vec4 colore;\n"
	"out vec2 TexLight2;\n"
	"out vec3 Pos;\n"
	"out float Bck;\n"

	"uniform vec3 Lightpos;\n"

	"uniform float normal;\n"

	"void main()\n"
	"{\n"
		"if(normal == 12.0 || normal == 15.0) {\n"
			"vec2 pnorm = (Position.xy + 1.0) * 0.5;\n"
			"vec2 lnorm = (Lightpos.xy + 1.0) * 0.5;\n"
			"vec2 dist = vec2(0.5 - (lnorm.x - pnorm.x), lnorm.y - pnorm.y + 0.5);\n"
			//"dist = (lnorm - pnorm) - dist;\n"
			"dist = (dist / 0.5) - 1.0;\n"
			"gl_Position = vec4(dist.x, dist.y, Position.z, 1.0); }\n"
		"else\n"
			"gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);\n"

		"TexCoord2 = TexCoord;\n"
		"colore = Color;\n"
		"TexLight2 = TexLight;\n"
		"Pos = Position;\n"
		"Bck = LBlock;\n"
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

static const char *Lightmap_FShader[256]={
	"#version 330\n"

	"float PI;\n"

	"precision mediump float;\n"

	"in vec2 TexCoord2;\n"

	"in vec4 colore;\n"

	"in vec2 TexLight2;\n"

	"in vec3 Pos;\n"

	"in float Bck;\n"

	"uniform vec3 Lightpos;\n"

	"layout (location = 0) out vec4 FColor;\n"
	"layout (location = 1) out vec4 NColor;\n"
	"layout (location = 2) out vec4 Amb;\n"
	"layout (location = 3) out vec4 Mask;\n"
	"layout (location = 4) out vec4 Lights;\n"
	"layout (location = 5) out vec4 Blockers;\n"
	"layout (location = 6) out vec4 Shadows;\n"

	"uniform sampler2D texu;\n"

	"uniform sampler2D texu2;\n"

	"uniform sampler2D texu3;\n"

	"uniform sampler2D texu4;\n"

	"uniform sampler2D texu5;\n"

	"uniform sampler2D texu6;\n"

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

	"float sample(vec2 coord, float r) {\n"
		"return step(r, texture(texu6, coord).r);\n"
	"}\n"

	"void main()\n"
	"{\n"
		"PI = 3.1415;\n"
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
			//"Blockers = vec4(0, 0, 0, texture(texu, TexCoord2).a) + Bck;\n"
		"}\n"
		"else\n"
		"if(normal == 3.0)\n"
		"{\n"
			"NColor = vec4(texture(texu3, TexCoord2).rgb, texture(texu, TexCoord2).a);\n"
			"FColor = texture(texu, TexCoord2);\n"
			"Mask = vec4(0, 0, 0, texture(texu, TexCoord2).a);\n"
			//"Blockers = vec4(0, 0, 0, texture(texu, TexCoord2).a) + Bck;\n"
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
			
			"Lights = vec4(FinalColor * texture(texu6, TexLight2).a, 1.0);\n"
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

			"float blurw = 100.0f * (1.0f - (falloff.y / 256.0f));\n"
			"vec4 blur = texture(texu5, TexCoord2);\n"
			"vec2 bluroffset = 1.0f / res.xy;\n"

			"for(int i = 1; i <= 100; ++ i)\n"
			"{\n"
				"if (float (i) >= blurw)\n"
					"break;\n"

				"float weight = 1.0 - float(i) / blurw;\n"
				"weight = weight * weight * (3.0 - 2.0 * weight);\n"
				"vec4 samplec1 = texture(texu5, TexCoord2 + vec2(bluroffset.x, 0.0f) * float(i));\n"
				"vec4 samplec2 = texture(texu5, TexCoord2 - vec2(bluroffset.x, 0.0f) * float(i));\n"
				"blur += vec4(samplec1.rgb + samplec2.rgb, 2.0f) * weight;\n"
			"}\n"

			"blur = vec4(blur.rgb / blur.w, blur.a);\n"

			"for(int i = 1; i <= 100; ++ i)\n"
			"{\n"
				"if (float (i) >= blurw)\n"
					"break;\n"

				"float weight = 1.0 - float(i) / blurw;\n"
				"weight = weight * weight * (3.0 - 2.0 * weight);\n"

				"vec4 samplec1 = texture(texu5, TexCoord2 + vec2(0.0f, bluroffset.y) * float(i));\n"
				"vec4 samplec2 = texture(texu5, TexCoord2 - vec2(0.0f, bluroffset.y) * float(i));\n"
				"blur += vec4(samplec1.rgb + samplec2.rgb, 2.0f) * weight;\n"
			"}\n"

			"Lights = vec4(0.0, 0.0, 0.0, clamp((blur.r / blur.w) / 1.5f - texture(texu4, TexLight2).a, 0.0, 1.0));\n"
		"}\n"
		"else\n"
		"if(normal == 9.0)\n"
		"{\n"
			//"NColor = vec4(0.0, 0.0 , 0.0, 1.0);\n"
			//"Amb = vec4(1.0, 0.0 , 0.0, 1.0);\n"

			"if(sector == 1 && gl_FragCoord.y < sector_y - 1) discard;\n"
			"if(sector == 2 && gl_FragCoord.y < sector_y - 1) discard;\n"
			"if(sector == 2 && gl_FragCoord.y > sector_y_2) discard;\n"

			"vec3 LightDir = vec3(Lightpos.xy - gl_FragCoord.xy, Lightpos.z);\n"
			"vec3 SpotLightDir = vec3(Lightpos.xy - spotdir.xy, Lightpos.z);\n"

			"float SpotEffect = dot(normalize(LightDir), normalize(SpotLightDir));\n"

			"if(SpotEffect > spotcos)\n"
			"{\n"

				"float SpotAtt = spotinnercos - spotcos;\n"
				"SpotAtt = clamp((SpotEffect - spotcos) / SpotAtt, 0.0, 1.0) * 2.0f;\n"

				"float blurw = 100.0f * (1.0f - (falloff.y / 256.0f));\n"
				"vec4 blur = texture(texu5, TexCoord2);\n"
				"vec2 bluroffset = 1.0f / res.xy;\n"

				"for(int i = 1; i <= 100; ++ i)\n"
				"{\n"
					"if (float (i) >= blurw)\n"
					"break;\n"

					"float weight = 1.0 - float(i) / blurw;\n"
					"weight = weight * weight * (3.0 - 2.0 * weight);\n"
					"vec4 samplec1 = texture(texu5, TexCoord2 + vec2(bluroffset.x, 0.0f) * float(i));\n"
					"vec4 samplec2 = texture(texu5, TexCoord2 - vec2(bluroffset.x, 0.0f) * float(i));\n"
					"blur += vec4(samplec1.rgb + samplec2.rgb, 2.0f) * weight;\n"
				"}\n"

				"blur = vec4(blur.rgb / blur.w, blur.a);\n"

				"for(int i = 1; i <= 100; ++ i)\n"
				"{\n"
					"if (float (i) >= blurw)\n"
					"break;\n"

					"float weight = 1.0 - float(i) / blurw;\n"
					"weight = weight * weight * (3.0 - 2.0 * weight);\n"

					"vec4 samplec1 = texture(texu5, TexCoord2 + vec2(0.0f, bluroffset.y) * float(i));\n"
					"vec4 samplec2 = texture(texu5, TexCoord2 - vec2(0.0f, bluroffset.y) * float(i));\n"
					"blur += vec4(samplec1.rgb + samplec2.rgb, 2.0f) * weight;\n"
				"}\n"

				//"blur.r *= SpotAtt;\n"

				"Lights = vec4(0, 0, 0, SpotAtt * clamp((blur.rgb / blur.w) / 1.5f - texture(texu4, TexLight2).a, 0.0, 1.0));\n"
			"}\n"
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

				"Lights = vec4(FinalColor * texture(texu6, TexLight2).a, 1.0);\n"
			"}\n"
		"}\n"
		"else\n"
		"if(normal == 10.0)\n"
		"{\n"
			"FColor = texture(texu5, TexCoord2);\n"
		"}\n"
		"else\n"
		"if(normal == 11.0)\n"
		"{\n"
			"float distance = 1.0;\n"

			"for (float y = 0.0; y < res.y; y += 1.0)\n"
			"{\n"
				//rectangular to polar filter
				//"vec2 ln = Lightpos.xy * 2.0 - 1.0;\n"
				"vec2 norm = vec2(TexCoord2.s, (y / res.y)) * 2.0 - 1.0;\n"
				"float theta = PI * 1.5 + norm.x * PI;\n"
				"float r = (1.0 + norm.y) * 0.5;\n"

				//coord which we will sample from occlude map
				"vec2 coord = vec2(-r * sin(theta), -r * cos(theta)) / 2.0 + 0.5;\n"

				//sample the occlusion map
				"vec4 data = texture(texu, coord);\n"

				//the current distance is how far from the top we've come
				"float dst = y / res.y;\n"

				//if we've hit an opaque fragment (occluder), then get new distance
				//if the new distance is below the current, then we'll use that for our ray
				"if (data.a > 0.75)\n"
					"distance = min(distance, dst);\n"
					//NOTE: we could probably use "break" or "return" here
			"}\n"
			//"gl_FragColor = vec4(vec3(distance), 1.0);\n"
			
			"FColor = vec4(vec3(distance), 1.0);\n"
		"}\n"
		"else\n"
		"if(normal == 14.0)\n"
		"{\n"

			"vec2 lp = vec2(Lightpos.x, res.y - Lightpos.y);\n"
			"vec3 LightDir = vec3(lp.xy - gl_FragCoord.xy, Lightpos.z);\n"
			"vec3 SpotLightDir = vec3(lp.xy - spotdir.xy, Lightpos.z);\n"

			"float SpotEffect = dot(normalize(LightDir), normalize(SpotLightDir));\n"

			"if(SpotEffect > spotcos)\n"
			"{\n"

				"vec2 norm = TexCoord2.st * 2.0 - 1.0;\n"

				"float ax = 1.0f / (res.x / 2.0f);\n"
				"float ay = 1.0f / (res.y / 2.0f);\n"
				"ay *= -1.0;\n"

				"lp = vec2(Lightpos.x * ax - 1.0, Lightpos.y * ay + 1.0);\n"

				"norm = norm.xy - lp.xy;\n"

				"float theta = atan(norm.y, norm.x);\n"
				"float r = length(norm);\n"
				"float coord = (theta + PI) / (2.0 * PI);\n"

				//the tex coord to sample our 1D lookup texture	
				//always 0.0 on y axis
				"vec2 tc = vec2(coord, 0.0);\n"

				//the center tex coord, which gives us hard shadows
				"float center = sample(tc, r);\n"


				//we multiply the blur amount by our distance from center
				//this leads to more blurriness as the shadow "fades away"
				"float blur = ((1.0 / res.x)  * smoothstep(0.0, 1.0, r)) * (256.0f / falloff.y);\n"

				//now we use a simple gaussian blur
				"float sum = 0.0;\n"

				"sum += sample(vec2(tc.x - 4.0*blur, tc.y), r) * 0.05;\n"
				"sum += sample(vec2(tc.x - 3.0*blur, tc.y), r) * 0.09;\n"
				"sum += sample(vec2(tc.x - 2.0*blur, tc.y), r) * 0.12;\n"
				"sum += sample(vec2(tc.x - 1.0*blur, tc.y), r) * 0.15;\n"

				"sum += center * 0.16;\n"

				"sum += sample(vec2(tc.x + 1.0*blur, tc.y), r) * 0.15;\n"
				"sum += sample(vec2(tc.x + 2.0*blur, tc.y), r) * 0.12;\n"
				"sum += sample(vec2(tc.x + 3.0*blur, tc.y), r) * 0.09;\n"
				"sum += sample(vec2(tc.x + 4.0*blur, tc.y), r) * 0.05;\n"

				"Shadows = vec4(0.0, 0.0, 0.0, clamp(sum * smoothstep(1.0, 0.0, r) - (-1.0 * texture(texu4, TexCoord2).a), 0.0, 1.0));\n"
			"}\n"
		"}\n"
		"else\n"
		"if(normal == 12.0)\n"
		"{\n"
			"Blockers = vec4(0, 0, 0, clamp(texture(texu, TexCoord2).a * colore.a * Bck, 0.0, 1.0));\n"
		"}\n"
		"else\n"
		"if(normal == 13.0)\n"
		"{\n"
			"vec2 norm = TexCoord2.st * 2.0 - 1.0;\n"

			"float ax = 1.0f / (res.x / 2.0f);\n"
			"float ay = 1.0f / (res.y / 2.0f);\n"
			"ay *= -1.0;\n"

			"vec2 lp = vec2(Lightpos.x * ax - 1.0f, Lightpos.y * ay + 1.0f);\n"

			"norm = norm.xy - lp.xy;\n"
			
			"float theta = atan(norm.y, norm.x);\n"
			"float r = length(norm);\n"
			"float coord = (theta + PI) / (2.0 * PI);\n"

			//the tex coord to sample our 1D lookup texture	
			//always 0.0 on y axis
			"vec2 tc = vec2(coord, 0.0);\n"

			//the center tex coord, which gives us hard shadows
			"float center = sample(tc, r);\n"


			//we multiply the blur amount by our distance from center
			//this leads to more blurriness as the shadow "fades away"
			"float blur = ((1.0 / res.x)  * smoothstep(0.0, 1.0, r)) * (256.0f / falloff.y);\n"

			//now we use a simple gaussian blur
			"float sum = 0.0;\n"

			"sum += sample(vec2(tc.x - 4.0*blur, tc.y), r) * 0.05;\n"
			"sum += sample(vec2(tc.x - 3.0*blur, tc.y), r) * 0.09;\n"
			"sum += sample(vec2(tc.x - 2.0*blur, tc.y), r) * 0.12;\n"
			"sum += sample(vec2(tc.x - 1.0*blur, tc.y), r) * 0.15;\n"

			"sum += center * 0.16;\n"

			"sum += sample(vec2(tc.x + 1.0*blur, tc.y), r) * 0.15;\n"
			"sum += sample(vec2(tc.x + 2.0*blur, tc.y), r) * 0.12;\n"
			"sum += sample(vec2(tc.x + 3.0*blur, tc.y), r) * 0.09;\n"
			"sum += sample(vec2(tc.x + 4.0*blur, tc.y), r) * 0.05;\n"

			"Shadows = vec4(0.0, 0.0, 0.0, clamp(sum * smoothstep(1.0, 0.0, r) - (-1.0 * texture(texu4, TexCoord2).a), 0.0, 1.0));\n"
		"}\n"
		"else\n"
		"if(normal == 15.0)\n"
		"{\n"
			"vec4 alpha = vec4(1.0);\n"

			"vec2 d = 2.0 * TexCoord2.xy - 1.0;\n"
			"float r = dot(d, d);\n"
			"float delta = fwidth(r);\n"
			"alpha = vec4(vec3(1.0), 1.0 - smoothstep(1.0 - delta, 1.0 + delta, r));\n"

			"Blockers = texture(texu, TexCoord2) * alpha;\n"
		"}\n"
	"}\n"
};

#endif