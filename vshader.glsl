#version 410

layout (location = 0) in vec4 vPosition;
out vec4 v2fcolor;

uniform vec4 translation;

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main()
{
    float scaling = (translation.z - 1) / 10;
    gl_Position = (vPosition + translation) * vec4(scaling, scaling, scaling, 1);

    float red = abs(translation.x) * random(translation.xz);
    float green = abs(translation.y) * random(translation.yz);
    float blue = abs(translation.z) * random(translation.xy);

    vec4 color = vec4(red, green, blue, 1);
    v2fcolor = color;
}
