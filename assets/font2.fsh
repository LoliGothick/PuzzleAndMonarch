//
// フォント描画用
// Signed distance field
// SOURCE https://github.com/libgdx/libgdx/wiki/Distance-field-fonts
//
$version$
$precision$

uniform sampler2D	uTex0;

float u_buffer = 0.5;
float u_gamma  = 0.5;

in vec4 Color;
in vec2 TexCoord0;

out vec4 oColor;


void main(void)
{
  float dist  = texture(uTex0, TexCoord0).r;
  float alpha = smoothstep(u_buffer - u_gamma, u_buffer + u_gamma, dist);
  oColor = vec4(Color.rgb, Color.a * alpha);
}
