void main()
{
  o_pos = (u_M * vec4(i_position,1)).xyz;

  o_texId = i_texId;
  o_color = i_color;

  o_uv = i_uv;
  o_normal = i_normal;

  gl_Position = u_VP * u_M * vec4(i_position, +1.0);



}
