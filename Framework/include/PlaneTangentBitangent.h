#pragma once
#include "DrawableObject.h"
#include "VertexTypes.h"

class PlaneTangentBitangent : public DrawableObject<VertexTypes::FullVertexTangentBitangent> {
public:
    explicit PlaneTangentBitangent(ID3D11Device* device);

    Layouts::VertexLayout_t getVertexLayout() const override;

    const wchar_t* getObjectName() const override {
        return L"PlaneTangentBitangent";
    }
};
