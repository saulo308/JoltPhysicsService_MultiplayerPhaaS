#include "BodyRuntimeData.h"

std::string BodyRuntimeData::GetBodyTypeAsString()
{
    switch(bodyType)
    {
        case EBodyType::Primary:
            return "Primary";
        case EBodyType::Clone:
            return "Clone";
        default:
            return "";
    }
}
