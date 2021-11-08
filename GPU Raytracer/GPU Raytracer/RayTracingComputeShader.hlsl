#include "RayTracingHeader.hlsli"

SamplerState gWrapSampler : register(s0);
TextureCube gObjBackground : register(t0); // Not used right now
RWTexture2D<float4> gResult : register(u0);

cbuffer cbCamera : register(b0)
{
    Camera gCamera;
};

cbuffer cbGeneralInfo : register(b1)
{
    uint gWidth;
    uint gHeight;
    uint gMaxRecursion; // Not used right now
    float gRayMaxLength;
};

cbuffer cbModels : register(b2)
{
    uint gNumSpheres;
	uint gNumTriangles;
    float2 pad0;
    Sphere gSpheres[MAX_SPHERES];
    Triangle gTriangles[MAX_TRIANGLES];
};

cbuffer cbLights : register(b3)
{
    DirectionalLight gSun;
    uint gNumPointLights;
    float3 pad1;
    PointLight gPointLights[MAX_POINT_LIGHTS];
};

/// <summary>Will retrieve a HitInfo structure with information about intersection</summary>
/// <param name = "r">A ray to check for intersections</param>
/// <param name = "hi">Structure to fill with informations about intersection</param>
/// <param name = "Closest">If true will check againts every object in the scene; if false will return only the first intersection</param>
/// <return>Will return true only if the ray hits something</return>

bool ClosestHit(inout Ray r, out HitInfo hi, in bool Closest)
{
    bool bFound = false;
	uint maxValue = gNumSpheres > gNumTriangles ? gNumSpheres : gNumTriangles;
    for (uint i = 0; i < maxValue; ++i)
    {
		if ( i < gNumSpheres)
        {
            if (gSpheres[i].Intersect(r))
            {
                bFound = true;
                hi.hitPoint = r.Origin + r.Direction * r.length;
                hi.normal = normalize(hi.hitPoint - gSpheres[i].Position);
                hi.Color = gSpheres[i].Color;
                hi.Reflectivity = gSpheres[i].Reflectivity;
                if (!Closest)
                {
                    break;
                }
            }
        }
        if (i < gNumTriangles)
        {
            if (gTriangles[i].Intersect(r))
            {
				bFound = true;
                hi.hitPoint = r.Origin + r.Direction * r.length;
                hi.normal = gTriangles[i].GetNormal();
				hi.Color = gTriangles[i].Color;
				hi.Reflectivity = gTriangles[i].Reflectivity;
				if (!Closest)
					break;
            }
        }
    }
    return bFound;
}

void ApplySunLight(in float3 hitPoint, in float3 normal, inout float4 color)
{
    Ray newRay;
    newRay.Origin = hitPoint;
    newRay.length = gRayMaxLength;
    newRay.Direction = normalize(-gSun.Direction);
	// Check if the sun hits this pixel
    bool bHit = false;
    HitInfo hi;
    if (ClosestHit(newRay, hi, false))
    {
		bHit = true;
    }
    if (!bHit)
    { // If the sun hits this pixel, calculate light
        float howMuchLight = dot(normal, -gSun.Direction);
        if (howMuchLight > 0)
        {
            color += gSun.Diffuse * howMuchLight;
        }
    }
}

void ApplyPointLights(in float3 hitPoint, in float3 normal, inout float4 color)
{
    for (uint i = 0; i < gNumPointLights; ++i)
    {
        float3 toLight = normalize(gPointLights[i].Position - hitPoint);
        bool bIsShadow = false;
        Ray newRay;
        newRay.Origin = hitPoint;
        newRay.length = gRayMaxLength;
        newRay.Direction = toLight;
		HitInfo hi;
        if (ClosestHit(newRay, hi, false))
            bIsShadow = true;
		if (!bIsShadow)
        {
            float howMuchLight = dot(toLight, normal);
            if (howMuchLight > 0)
            {
                color += gPointLights[i].Diffuse * howMuchLight;
            }
        }
    }
}

void Trace(Ray r, inout float4 result)
{
    result = float4(0, 0, 0, 0);
    float frac = 1.0f;
    Ray currentRay;
    currentRay.Direction = r.Direction;
    currentRay.Origin = r.Origin;
    currentRay.length = r.length;
    for (uint i = 0; i < gMaxRecursion; ++i)
    {
		HitInfo hi;
        if (ClosestHit(currentRay, hi, true))
        {
            float4 local = gSun.Ambient;
            ApplySunLight(hi.hitPoint, hi.normal, local);
            ApplyPointLights(hi.hitPoint, hi.normal, local);
            local = saturate(local * hi.Color);
            result += local * (1.0f - hi.Reflectivity) * frac;
            frac *= (hi.Reflectivity);
            if (frac < EPSILON)
                break;
            currentRay.Direction = reflect(currentRay.Direction, hi.normal);
            currentRay.Origin = hi.hitPoint;
            currentRay.length = gRayMaxLength;
        }
		else
        {
            float4 local;
            local = gObjBackground.SampleLevel(gWrapSampler, currentRay.Direction.xyz, 0);
            result += local * (1.0f - hi.Reflectivity) * frac;
            break;
        }
    }

}

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID,
			uint3 GroupID : SV_GroupThreadID )
{
    float u = (float) DTid.x / (float) gWidth;
    float v = (float) DTid.y / (float) gHeight;

    float4 currentRayDirection = gCamera.TopLeftCorner + u * (gCamera.TopRightCorner - gCamera.TopLeftCorner)
										+ v * (gCamera.BottomLeftCorner - gCamera.TopLeftCorner);
    currentRayDirection.w = 0.0f;
    currentRayDirection = normalize(currentRayDirection);
    Ray r;
    r.Origin = gCamera.CamPosition.xyz;
    r.Direction = currentRayDirection.xyz;
    r.length = gRayMaxLength;

    float4 Color;
    Trace(r, Color);
    gResult[DTid.xy] = Color;
}