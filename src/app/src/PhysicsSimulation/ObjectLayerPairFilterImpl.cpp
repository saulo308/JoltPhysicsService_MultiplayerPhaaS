#include "ObjectLayerPairFilterImpl.h"

bool ObjectLayerPairFilterImpl::ShouldCollide(ObjectLayer inObject1, 
	ObjectLayer inObject2) const
{
	switch (inObject1)
	{
	case Layers::NON_MOVING:
		// Non moving only collides with moving
		return inObject2 == Layers::MOVING; 
	case Layers::MOVING:
		// Moving collides with everything
		return true; 
	default:
		return false;
	}
}
