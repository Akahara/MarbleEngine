#version 330 core

out vec4 color;
uniform sampler2D u_texture;

uniform float u_amount = 0.3f;

void main() {

  vec2 texSize   = textureSize(u_texture, 0).xy;
  vec2 fragCoord = gl_FragCoord.xy;
  vec2 texCoord  = fragCoord / texSize;

  float neighbor = u_amount * -1.0;
  float center   = u_amount *  4.0 + 1.0;

  color.xyz =
        texture(u_texture, (fragCoord + vec2( 0,  1)) / texSize).rgb
      * neighbor

      + texture(u_texture, (fragCoord + vec2(-1,  0)) / texSize).rgb
      * neighbor
      + texture(u_texture, (fragCoord + vec2( 0,  0)) / texSize).rgb
      * center
      + texture(u_texture, (fragCoord + vec2( 1,  0)) / texSize).rgb
      * neighbor

      + texture(u_texture, (fragCoord + vec2( 0, -1)) / texSize).rgb
      * neighbor
      ;

  color.a = 1;
}