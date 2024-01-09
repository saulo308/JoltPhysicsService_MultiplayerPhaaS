#include "BodyRuntimeData.h"

std::string BodyRuntimeData::GetBodyTypeAsString()
{
    // Switch on the current body type and return a string from the enum type
    switch(currentBodyType)
    {
        case EBodyType::Primary:
            return "Primary";
        case EBodyType::Clone:
            return "Clone";
        default:
            return "";
    }
}
