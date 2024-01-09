#ifndef BODYRUNTIMEDATA_H
#define  BODYRUNTIMEDATA_H

#include <iostream>

/** 
* This enum stores the body type. The body can be from type "primary", which 
* means that the physics service is actively controlling the body on the game; 
* or from type "clone", which means that he only exists on this physics service
* as a way maintain physics simulation consistency. Thus, this actual body will 
* be driven by another pyhsics service instead.
*/
enum EBodyType
{
    Primary,
    Clone
};

/** 
* The body's runtime data. This is custom to this physics service and may store
* any body data that should exist during runtime. Thus, this is an extension
* of the Body.h class, which allows us to store and data we want.
*/
class BodyRuntimeData final
{
public:
    /** 
    * Set this body's current type.
    * 
    * @param newBodyType The new body type
    */
    void SetBodyType(EBodyType newBodyType)
    {
        currentBodyType = newBodyType;
    }

    /**
    * Getter to the body's type as a string.
    * 
    * @return The body's type as a string
    */
    std::string GetBodyTypeAsString();

    /** Getter to the current body type */
    EBodyType GetBodyType() { return currentBodyType; }

private:
    /**
    * The current body type. The body can be from type "primary", which 
    * means that the physics service is actively controlling the body on the 
    * game; or from type "clone", which means that he only exists on this 
    * physics service as a way maintain physics simulation consistency. Thus, 
    * this actual body will be driven by another pyhsics service instead.
    *
    * @see EBodyType
    */
    EBodyType currentBodyType = EBodyType::Primary;
};

#endif
