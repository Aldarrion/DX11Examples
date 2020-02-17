#pragma once

/**
 * \brief Class indicating that deriving class holds a DirectX resource. 
 * making it move only types since it holds one or more pointers to inner 
 * DirectX resources which can not be copied and need to be released exactly once.
 */
class ResourceHolder {
public:
    ResourceHolder() = default;

    ResourceHolder(const ResourceHolder&) = delete;
    ResourceHolder& operator=(const ResourceHolder&) = delete;
};
