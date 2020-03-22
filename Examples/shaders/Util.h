// ==============================================
// Convenience functions for SRGB transformations
// ==============================================
// These are unoptimized, ideally we would let the GPU
// handle the gamma correction by using SRGB back buffer
// (which we actually do)

float fromSRGB(float srgbCol) {
    float linearCol;
    if (srgbCol <= 0.04045)
        linearCol = srgbCol / 12.92;
    else
        linearCol = pow((srgbCol + 0.055) / 1.055, 2.4);

    return linearCol;
}

float toSRGB(float linearCol) {
    float srgbCol;
    if (linearCol <= 0.0031308)
        srgbCol = linearCol * 12.92;
    else
        srgbCol = 1.055 * pow(linearCol, 1.0 / 2.4) - 0.055;

    return srgbCol;
}

float4 colFromSRGB(float4 srgbCol) {
    return float4(fromSRGB(srgbCol.r), fromSRGB(srgbCol.g), fromSRGB(srgbCol.b), srgbCol.a);
}

float4 colToSRGB(float4 srgbCol) {
    return float4(toSRGB(srgbCol.r), toSRGB(srgbCol.g), toSRGB(srgbCol.b), srgbCol.a);
}