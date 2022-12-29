#version 330 core

out vec4 color;

in vec2 o_uv;

uniform float u_time;
uniform vec2 u_worldOffset;

float easeOut(float x) {
  return 1-(1-x)*(1-x);
}

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float hash(vec2 p) {
  return fract(1e4 * sin(dot(p, vec2(17., .1))) * (.1 + abs(sin(dot(p, vec2(13., 1.))))));
}

float noise(vec2 x) {
	vec2 i = floor(x);
	vec2 f = fract(x);

	float a = hash(i);
	float b = hash(i + vec2(1.0, 0.0));
	float c = hash(i + vec2(0.0, 1.0));
	float d = hash(i + vec2(1.0, 1.0));

	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main()
{
  vec2 uv = o_uv*2.-1.;
  float t = u_time * .1;
  vec2 o = uv*3 - t * .01 - u_worldOffset * .0003;
  float height = 0;
  float amplitude = .5;
  float lacunarity = 10;
  for(int i = 0; i < 5; i++) {
	height += noise(o * lacunarity + vec2(cos(i), sin(i))*t) * amplitude;
	amplitude *= .35;
	lacunarity *= 2.52;
  }
  float w = height;
  float l = max(0, 1-dot(uv, uv));
  color = vec4(1.);
  color.a = smoothstep(.3, .7, w) * easeOut(l);
}