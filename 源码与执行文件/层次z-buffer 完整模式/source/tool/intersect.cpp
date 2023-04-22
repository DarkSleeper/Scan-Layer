#include "intersect.h"
#include <math.h>

bool v3less(glm::vec3 const& a, glm::vec3 const& b) {
    return (a.x <= b.x && a.y <= b.y && a.z <= b.z);
}
bool v3great(glm::vec3 const& a, glm::vec3 const& b) {
    return (a.x >= b.x && a.y >= b.y && a.z >= b.z);
}

bool is_seperating_axis(float p0, float p1, float r)
{
    if (fmax(p0, p1) < -r || fmin(p0, p1) > r) return true;
    else return false;
}

/*
    The expression "n[dot]v" is the dot product of the two vectors.
    The expression "n[cross]v" is the cross production of the two vectors.
    Let n = axis[cross](v(i+1)-v(i)), pi = n[dot]v(i).
    For edge v(1)-v(0), p0 == p1. Because n is vertical to edge(v(1)-v(0))!
    For edge v(2)-v(1), p1 == p2.
    For edge v(0)-v(2), p2 == p0.
*/
bool test_axis_cross_edge(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& e, const Bound_Box& box)
{
    float fex, fey, fez, r;
    fex = fabs(e.x);
    fey = fabs(e.y);
    fez = fabs(e.z);
    float p1, p2;
    //e and Axis X
    //n = (1,0,0)[cross]e = (0, -e.z, e.y)
    p1 = -e.z * v1.y + e.y * v1.z;
    p2 = -e.z * v2.y + e.y * v2.z;
    r = fez * box.half_length[1] + fey * box.half_length[2];
    if (is_seperating_axis(p1, p2, r)) return false;
    //e and Axis Y
    //n = (0,1,0)[cross]e = (e.z, 0, -e.x)
    p1 = e.z * v1.x - e.x * v1.z;
    p2 = e.z * v2.x - e.x * v2.z;
    r = fez * box.half_length[0] + fex * box.half_length[2];
    if (is_seperating_axis(p1, p2, r)) return false;
    //e and Axis Z
    //n = (0,0,1)[cross]e = (-e.y, e.x, 0)
    p1 = -e.y * v1.x + e.x * v1.y;
    p2 = -e.y * v2.x + e.x * v2.y;
    r = fey * box.half_length[0] + fex * box.half_length[1];
    if (is_seperating_axis(p1, p2, r)) return false;

    return true;
}

bool is_intersect(Bound_Box box, Triangle tri) {
    auto tri_box_max = glm::vec3(fmax(tri.a.x, fmax(tri.b.x, tri.c.x)), fmax(tri.a.y, fmax(tri.b.y, tri.c.y)), fmax(tri.a.z, fmax(tri.b.z, tri.c.z)));
    auto tri_box_min = glm::vec3(fmin(tri.a.x, fmin(tri.b.x, tri.c.x)), fmin(tri.a.y, fmin(tri.b.y, tri.c.y)), fmin(tri.a.z, fmin(tri.b.z, tri.c.z)));

    glm::vec3 box_min = box.center - box.half_length;
    glm::vec3 box_max = box.center + box.half_length;
    if (v3less(tri_box_min, box_max) && v3great(tri_box_max, box_min)) {
        //intersect or contain for box
        if (v3less(box_min, tri_box_min) && v3great(box_max, tri_box_max)) {
            //contain
            return true;
        }

        //分离轴算法求交 (前面已经测了三个轴了)
        glm::vec3 v[3];
        v[0] = tri.a - box.center;
        v[1] = tri.b - box.center;
        v[2] = tri.c - box.center;

        //Test the 9 tests first
        //Edge e0
        const glm::vec3 e0 = v[1] - v[0];
        if (!test_axis_cross_edge(v[2], v[0], e0, box)) return false;
        //Edge e1
        const glm::vec3 e1 = v[2] - v[1];
        if (!test_axis_cross_edge(v[0], v[1], e1, box)) return false;
        //Edge e2
        const glm::vec3 e2 = v[2] - v[0];
        if (!test_axis_cross_edge(v[1], v[2], e2, box)) return false;

        //Test if the box intersects the plane of the triangle.
        //project triangle and box to triangle's normal
        glm::vec3 normal = glm::cross(v[1] - v[0], v[2] - v[0]);
        float d = glm::dot(normal, v[0]);
        float fex, fey, fez, r;
        fex = fabs(normal.x);
        fey = fabs(normal.y);
        fez = fabs(normal.z);
        r = fex * box.half_length[0] + fey * box.half_length[1] + fez * box.half_length[2];
        if (d < -r || d > r) return false;

        return true;
    }
    else {
        return false;
    }
}