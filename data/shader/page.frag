#version 330 core

in vec3 Position0;
in vec2 TexCoord0;
in vec3 Normal0;

out vec3 Color;

uniform sampler2D Texture;

uniform sampler2D Shadow;
uniform mat4 ShadowVP;

vec3 transform(vec3 p, mat4 space)
{
	vec4 proj = space * vec4(p, 1.0);
	return (proj.xyz / proj.w);
}

bool depth_test(float z, vec2 coord, sampler2D depth)
{
	float target = texture(depth, coord).r;
	return (target > z);
}

float shadow(vec3 position)
{
	const int rings = 2;
	const float scale = 0.0003f;
	const float bias = 0.0005f;

	vec3 coord = transform(position, ShadowVP) * 0.5f + 0.5f;

	float total = 0.f;
	for (int x = -rings; x <= rings; ++x)
	{
		for (int y = -rings; y <= rings; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * scale;
			vec2 sample = coord.xy + offset;

			if (!depth_test(coord.z - bias, sample, Shadow))
				total += 1.f;
		}
	}

	float total_samples = pow(2 * rings + 1, 2);
	return (total / float(total_samples));
}

void main()
{
	vec3 dir = normalize(vec3(-0.5f, 1.f, -1.f));

	float ill = max(dot(Normal0, dir) / length(dir), 0.3f);
	float sh = max(1.f - (shadow(Position0) * 0.7f), 0.f) + 0.3f;

	Color = texture(Texture, TexCoord0).rgb * sh * ill;
}