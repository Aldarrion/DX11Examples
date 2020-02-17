#pragma once
#include <d3d11.h>
#include <vector>

namespace Layouts {

using VertexLayout_t = std::vector<D3D11_INPUT_ELEMENT_DESC>;

const extern VertexLayout_t POS_LAYOUT;
const extern VertexLayout_t POS_UV_LAYOUT;
const extern VertexLayout_t POS_NORM_UV_LAYOUT;
const extern VertexLayout_t POS_NORM_COL_LAYOUT;
const extern VertexLayout_t TEXTURED_LAYOUT;
const extern VertexLayout_t TEXTURED_LAYOUT_TANGENT_BITANGENT;
}
