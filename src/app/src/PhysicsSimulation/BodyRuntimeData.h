#ifndef BODYRUNTIMEDATA_H
#define  BODYRUNTIMEDATA_H

#include <iostream>

enum EBodyType
{
    Primary,
    Clone
};

class BodyRuntimeData final
{
public:
    std::string GetBodyTypeAsString();

    void UpdateBodyType(EBodyType newBodyType)
    {
        bodyType = newBodyType;
    }

public:
    EBodyType bodyType = EBodyType::Primary;
};


#endif