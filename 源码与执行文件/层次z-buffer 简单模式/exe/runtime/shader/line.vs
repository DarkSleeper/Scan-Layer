layout (location = 0) in vec3 aPos;

uniform mat4 view_to_clip_matrix;
uniform mat4 world_to_view_matrix;
uniform mat4 model;

void main()
{
    gl_Position = view_to_clip_matrix * world_to_view_matrix * model * vec4(aPos, 1.0f); 
}