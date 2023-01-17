 #version 330 core
 
 // These are set by the game engine.
 // The render target size is one-quarter the scene rendering size.
 
 sampler colorSampler;
 sampler depthSampler;

 const float2 dofEqWorld;
 const float2 dofEqWeapon;
 const float2 dofRowDelta;  

 // float2( 0, 0.25 / renderTargetHeight )
 const float2 invRenderTargetSize;
 const float4x4 worldViewProj;
 
 struct PixelInput {
    float4 position : POSITION;
    float2 tcColor0 : TEXCOORD0;
    float2 tcColor1 : TEXCOORD1;
    float2 tcDepth0 : TEXCOORD2;
    float2 tcDepth1 : TEXCOORD3;
    float2 tcDepth2 : TEXCOORD4;
    float2 tcDepth3 : TEXCOORD5;
};

PixelInput DofDownVS( float4 pos : POSITION, float2 tc : TEXCOORD0 ) {
        PixelInput pixel;
        pixel.position = mul( pos, worldViewProj );
        pixel.tcColor0 = tc + float2( -1.0, -1.0 ) * invRenderTargetSize;
        pixel.tcColor1 = tc + float2( +1.0, -1.0 ) * invRenderTargetSize;
        pixel.tcDepth0 = tc + float2( -1.5, -1.5 ) * invRenderTargetSize;
        pixel.tcDepth1 = tc + float2( -0.5, -1.5 ) * invRenderTargetSize;
        pixel.tcDepth2 = tc + float2( +0.5, -1.5 ) * invRenderTargetSize;
        pixel.tcDepth3 = tc + float2( +1.5, -1.5 ) * invRenderTargetSize;
        return pixel; 
} 
        
half4 DofDownPS( const PixelInput pixel ) : COLOR {
    half3 color;
    half maxCoc;
    float4 depth;
    half4 viewCoc;
    half4 sceneCoc;
    half4 curCoc;
    half4 coc;
    float2 rowOfs[4]; // "rowOfs" reduces how many moves PS2.0 uses to emulate swizzling.   
    rowOfs[0] = 0;
    rowOfs[1] = dofRowDelta.xy;
    rowOfs[2] = dofRowDelta.xy * 2;
    rowOfs[3] = dofRowDelta.xy * 3;// Use bilinear filtering to average 4 color samples for free.   
    color = 0;
    color += tex2D( colorSampler, pixel.tcColor0.xy + rowOfs[0] ).rgb;
    color += tex2D( colorSampler, pixel.tcColor1.xy + rowOfs[0] ).rgb;
    color += tex2D( colorSampler, pixel.tcColor0.xy + rowOfs[2] ).rgb;
    color += tex2D( colorSampler, pixel.tcColor1.xy + rowOfs[2] ).rgb;
    color /= 4;   // Process 4 samples at a time to use vector hardware efficiently.
    
    // The CoC will be 1 if the depth is negative, so use "min" to pick    
    // between "sceneCoc" and "viewCoc".
    
    depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[0] ).r;
    depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[0] ).r;
    depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[0] ).r;
    depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[0] ).r;
    viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );
    sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );
    curCoc = min( viewCoc, sceneCoc );
    coc = curCoc;

    depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[1] ).r;
    depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[1] ).r;
    depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[1] ).r;
    depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[1] ).r;
    viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );
    sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y ); 
    curCoc = min( viewCoc, sceneCoc );   coc = max( coc, curCoc ); 
    depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[2] ).r;
    depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[2] ).r; 
    depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[2] ).r; 
    depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[2] ).r; 
    viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y ); 
    sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  

    curCoc = min( viewCoc, sceneCoc ); 
    coc = max( coc, curCoc ); 

    depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[3] ).r;  
    depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[3] ).r;
    depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[3] ).r; 
    depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[3] ).r; 
    viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );
    sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  
    curCoc = min( viewCoc, sceneCoc ); 
    coc = max( coc, curCoc );  
    maxCoc = max( max( coc[0], coc[1] ), max( coc[2], coc[3] ) );
    return half4( color, maxCoc ); 
} 