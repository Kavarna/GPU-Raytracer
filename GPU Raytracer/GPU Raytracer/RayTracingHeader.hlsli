
#define MAX_SPHERES 240
#define MAX_TRIANGLES 16
#define MAX_SHAPES 256

#define MAX_POINT_LIGHTS 2

#define EPSILON 0.0001f

// Miscellaneous
class Camera
{
    float4 CamPosition;
    float4 TopLeftCorner;
    float4 TopRightCorner;
    float4 BottomLeftCorner;
};

class Ray
{
    float3 Origin;
    float3 Direction;
    float length;
};

class HitInfo
{
	float3 hitPoint;
	float3 normal;
	float Reflectivity;
	float4 Color;
};

bool isPointInTriangle(float3 A, float3 B, float3 C, float3 Point)
{
    float3 AC = C - A;
	float3 AB = B - A;
    float3 AP = Point - A;

    float dot00 = dot(AC, AC);
    float dot01 = dot(AC, AB);
    float dot02 = dot(AC, AP);
    float dot11 = dot(AB, AB);
    float dot12 = dot(AB, AP);
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u, v;
    u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    return u >= 0 && v >= 0 && u + v < 1;
}

// Models
class Sphere // 48 bytes
{
    float3 Position; // 12 bytes
    float Radius; // 4 bytes
    float4 Color; // 16 bytes
    float Reflectivity; // 4 bytes
    float3 pad; // 12 bytes
    bool Intersect(inout Ray r)
    {
        bool bReturn = true;
        float3 OriginToSphere = Position - r.Origin;
        float projection = dot(OriginToSphere, r.Direction);
        float3 distanceVector = OriginToSphere - projection * r.Direction;
        float distanceVectorLengthSQ = dot(distanceVector, distanceVector);
        float radiusSQ = Radius * Radius;
        if (distanceVectorLengthSQ > radiusSQ)
        {
            bReturn = false;
        }
		
		if (bReturn)
		{
        
            float newLength = projection - sqrt(radiusSQ - distanceVectorLengthSQ);
            if (newLength < 0.0f || newLength > r.length)
            {
                bReturn = false;
            }

			if (bReturn)
            {
                r.length = newLength;
            }
        }
        return bReturn;
    }
};

class Triangle
{
	float3 A;
	float pad0;
	float3 B;
	float pad1;
	float3 C;
	float pad2;
	float Reflectivity;
	float3 pad3;
	float4 Color; 
    bool Intersect(inout Ray r)
    {
		float3 Normal = GetNormal();
        float3 AB, AC;
        AB = B - A;
        AC = C - A;
		bool bRet = false;
        if (abs(dot(r.Direction, Normal)) <= EPSILON) // Ray's direction and Plane Normal are orthogonal
			bRet = false;
        float D = dot(Normal, A);
        float t = -(dot(Normal, r.Origin) + D) / dot(Normal, r.Direction);
        if (isPointInTriangle(A, B, C, r.Origin + t * r.Direction))
        {
            if (t > 0 && t <= r.length) // The triangle is in front of the ray
            {
                bRet = true;
                r.length = t / (1.0f + EPSILON);
            }
        }

		return bRet;

    }
	float3 GetNormal()
    {
        float3 AB, AC;
        AB = B - A;
        AC = C - A;
        float3 normal;
        normal = cross(AB, AC);
        normal = normalize(normal);
		return normal;
    }
};


// Lights
struct DirectionalLight
{
    float3 Direction;
    float pad;
    float4 Diffuse;
    float4 Ambient;
};

struct PointLight
{
    float3 Position;
    float pad;
    float4 Diffuse;
};