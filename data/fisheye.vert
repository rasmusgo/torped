uniform vec4 post_adjustments; // = vec4(0.0, 0.0, 1.0, 4.0 / 3.0);

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;

    vec4 HPos = gl_ModelViewProjectionMatrix * gl_Vertex;
    vec4 EPos = gl_ModelViewMatrix * gl_Vertex;
    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );
    vec3 lightvec = normalize( (gl_LightSource[0].position - EPos).xyz );

    // Reflection mapping
    vec3 WPos = normalize(EPos).xyz * gl_NormalMatrix; // Position relative viewer without rotation
    vec3 n = normalize(gl_Normal);     // normal;
    vec3 u = normalize(WPos);            // normalize(EPos.xyz);
    vec3 f = u - 2.0*n*dot(n,u);
    float m = 2.0*sqrt(f.x*f.x + f.y*f.y + (f.z*f.z + 2.0*f.z + 1.0));

    //OUT.Tex2 = float4( f.xy/m + float2(0.5, 0.5), 0,1 );
    //OUT.Tex2 = float4( f.x, -f.z, f.y, 1 );
    gl_TexCoord[2] = vec4( f, 1 );

    // Simple lighting
    float diffuse = dot(normal, lightvec);
    gl_FrontColor = diffuse*gl_LightSource[0].diffuse + gl_LightSource[0].ambient;

    // Fisheye perspective
    float tmpz = (HPos.z / HPos.w)*2.0 - 1.0;
    HPos.xyz = normalize(HPos.xyz);
    float theta = acos(HPos.z);
    HPos.xy = normalize(HPos.xy) * (theta*0.75);
    HPos.zw = vec2(tmpz, 1.0);

    // Post adjustment for stereo 3d and aspect ratio
    HPos.x += post_adjustments.x + post_adjustments.x/tmpz; // depth
    HPos.x += post_adjustments.y; // focus
    HPos.xy *= post_adjustments.zw; // aspect (scale)

    gl_Position = HPos;
}
