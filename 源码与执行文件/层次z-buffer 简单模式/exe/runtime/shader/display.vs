layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 world_normal;
out vec4 obj_color;

uniform mat4 view_to_clip_matrix;
uniform mat4 world_to_view_matrix;
uniform mat4 inv_world_matrix;
uniform mat4 model;

void main()
{
    gl_Position = view_to_clip_matrix * world_to_view_matrix * model * vec4(aPos, 1.0f); 
    world_normal = normalize(vec3(vec4(aNormal, 0.0) * inv_world_matrix));
    obj_color = vec4(1.0);
}