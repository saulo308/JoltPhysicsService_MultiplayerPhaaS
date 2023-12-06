#include "PhysicsServiceImpl.h"

void PhysicsServiceImpl::InitPhysicsSystem
	(const std::string initializationActorsInfo)
{
    std::cout << "Initializing physics system...\n";
    std::cout << "InitializationInfo:\n" << initializationActorsInfo << "\n";

	// If physics system is already initialized, clear the last initialization
	if(bIsInitialized)
	{
		ClearPhysicsSystem();
	}

	// Register allocation hook
	RegisterDefaultAllocator();

	// Install callbacks
	//Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Create a factory
	Factory::sInstance = new Factory();

	// Register all Jolt physics types
	RegisterTypes();

	// We need a temp allocator for temporary allocations during the physics 
	// update. We're pre-allocating 10 * 1024 * 1024 MB to avoid having to do 
	// allocations during the physics update. 
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc 
	// to fall back to malloc/free.
	temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. 
	// Typically you would implement the JobSystem interface yourself and let 
	// Jolt Physics run on top of your own job scheduler. JobSystemThreadPool 
	// is an example implementation.
	job_system = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers,
		thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics 
	// system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real 
	// project use something in the order of 65536.
	const uint cMaxBodies = 65536;

	// This determines how many mutexes to allocate to protect rigid bodies 
	// from concurrent access. Set it to 0 for the default settings.
	const uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time 
	// (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a 
	// queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to
	// do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real 
	// project use something in the order of 65536.
	const uint cMaxBodyPairs = 65536;

	// This is the maximum size of the contact constraint buffer. If more 
	// contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start 
	// interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real 
	// project use something in the order of 10240.
	const uint cMaxContactConstraints = 10240;

	// Now we can create the actual physics system.
	physics_system = new PhysicsSystem();
	physics_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, 
		cMaxContactConstraints, broad_phase_layer_interface, 
		object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

	// Define the physics world settings
	PhysicsSettings physicsSettingsData;
	physicsSettingsData.mNumVelocitySteps = 10;
	physicsSettingsData.mNumPositionSteps = 2;
	physicsSettingsData.mBaumgarte = 0.2f;

	physicsSettingsData.mSpeculativeContactDistance = 0.02f;
	physicsSettingsData.mPenetrationSlop = 0.02f;
	physicsSettingsData.mMinVelocityForRestitution = 1.0f;
	physicsSettingsData.mTimeBeforeSleep = 0.5f;
	physicsSettingsData.mPointVelocitySleepThreshold = 0.03f;

	physicsSettingsData.mDeterministicSimulation = true;
	physicsSettingsData.mConstraintWarmStart = true;
	physicsSettingsData.mUseBodyPairContactCache = true;
	physicsSettingsData.mUseManifoldReduction = true;
	physicsSettingsData.mUseLargeIslandSplitter = true;
	physicsSettingsData.mAllowSleeping = true;
	physicsSettingsData.mCheckActiveEdges = true;

	// Set the new physics settings
	physics_system->SetPhysicsSettings(physicsSettingsData);

	// Define gravity to act on the z-axis 
	// (so it is the same as Unreal's gravity)
	Vec3Arg newGravity(0.f, 0.f, -980.f);
	physics_system->SetGravity(newGravity);

	// A body activation listener gets notified when bodies activate and go 
	// to sleep
	// Note that this is called from a job so whatever you do here needs to be 
	// thread safe.
	// Registering one is entirely optional.
	physics_system->SetBodyActivationListener(body_activation_listener);

	// A contact listener gets notified when bodies (are about to) collide, 
	// and when they separate again.
	// Note that this is called from a job so whatever you do here needs to 
	// be thread safe.
	// Registering one is entirely optional.
	physics_system->SetContactListener(contact_listener);

	// The main way to interact with the bodies in the physics system is 
	// through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though 
	// we're not planning to access bodies from multiple threads)
	body_interface = &physics_system->GetBodyInterface();

	/*
	// Next we can create a rigid body to serve as the floor, we make a large 
	// box
	// Create the settings for the collision volume (the shape). 
	// Note that for simple shapes (like boxes) you can also directly construct 
	// a BoxShape.
	BoxShapeSettings floor_shape_settings(Vec3(1000.0f, 1000.f, 100.0f));

	// Create the shape
	ShapeSettings::ShapeResult floor_shape_result = 
		floor_shape_settings.Create();

	// We don't expect an error here, but you can check floor_shape_result for 
	// HasError() / GetError()
	ShapeRefC floor_shape = floor_shape_result.Get(); 

	// Create the settings for the body itself. Note that here you can also set 
	// other properties like the restitution / friction.
	BodyCreationSettings floor_settings(floor_shape,
		RVec3(0.0_r, 0.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, 
		Layers::NON_MOVING);

	// Create the actual rigid body
	// Note that if we run out of bodies this can return nullptr
	Body* floor = body_interface->CreateBodyWithID(BodyID(9998), 
		floor_settings); 

	// Set the floor's friction and add a small rotation on y-axis
    floor->SetFriction(1.0f);
	floor->AddRotationStep(RVec3(0.f, -0.01f, 0.f));

	// Add it to the world
	body_interface->AddBody(floor->GetID(), EActivation::DontActivate);

	// Create the second floor settings with new position
	BodyCreationSettings floor_settings2(floor_shape,
		RVec3(0.0_r, 2010.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, 
		Layers::NON_MOVING);

	// Create the second floor
	Body* floor2 = body_interface->CreateBodyWithID(BodyID(9999), 
		floor_settings2); 

	// Set the floor's friction, add a small rotation on y-axis and translate 
    floor2->SetFriction(1.0f);
	floor2->AddRotationStep(RVec3(0.f, -0.01f, 0.f));

	// Add it to the world
	body_interface->AddBody(floor2->GetID(), EActivation::DontActivate);
	*/

	// Split actors info from initialization into lines
	std::stringstream initializationStringStream(initializationActorsInfo);
    std::vector<std::string> initializationActorsInfoLines;

	std::string line;
    while (std::getline(initializationStringStream, line)) 
	{
        initializationActorsInfoLines.push_back(line);
    }

	// for each line (begin from "1" as first is only "Init" into "size() - 1"
	// as the last is "EndMessage"), create a new body with according to the
	// body's type, id and initial location
	for(int i = 1; i < initializationActorsInfoLines.size() - 1; i++)
	{
		// Split info with ";" delimiter
		std::stringstream actorInfoStringStream
			(initializationActorsInfoLines[i]);
		std::vector<std::string> actorInfoList;

		std::string actorInfoData;
		while (std::getline(actorInfoStringStream, actorInfoData, ';')) 
		{
			actorInfoList.push_back(actorInfoData);
		}

		// Check for errors
		if(actorInfoList.size() < 5)
		{
			std::cout 
				<< "Error on parsing initialization actor info. Line with less than 4 params: " 
				<< initializationActorsInfoLines[i]
				<< "\n";
			continue;
		}

		// Get the actor's type to be creates
		const std::string actorType = actorInfoList[0];

		// Get the actor ID from the init info
		const int actorId = std::stoi(actorInfoList[1]);
		const BodyID newBodyID(actorId);

		// Get actor initial pos
		const double initialPosX = std::stod(actorInfoList[2]);
		const double initialPosY = std::stod(actorInfoList[3]);
		const double initialPosZ = std::stod(actorInfoList[4]);

		const RVec3 bodyInitialPosition = RVec3(initialPosX, initialPosY,
			initialPosZ);

		// Check if we should create a floor
		if(actorType.find("floor") != std::string::npos)
		{
			// Add new floor to the physics world
			AddNewFloorToPhysicsSystem(newBodyID, bodyInitialPosition);
			continue;
		}

		// Check if we should create a sphere
		if(actorType.find("sphere") != std::string::npos)
		{
			// Add new sphere to the physics world
			AddNewSphereToPhysicsWorld(newBodyID, bodyInitialPosition);
			continue;
		}
	}

	// Optional step: Before starting the physics simulation you can optimize 
	// the broad phase. This improves collision detection performance 
	// (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. 
	// streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to 
	// keep the broad phase efficient.
	//physics_system->OptimizeBroadPhase();

	bIsInitialized = true;

    std::cout << "Physics world has been initialized and is running.\n\n";
}

std::string PhysicsServiceImpl::StepPhysicsSimulation()
{
	// If you take larger steps than 1 / 60th of a second you need to do 
	// multiple collision steps in order to keep the simulation stable. 
	// Do 1 collision step per 1 / 60th of a second (round up).
	const int cCollisionSteps = 1;

	// If you want more accurate step results you can do multiple sub steps 
	// within a collision step. Usually you would set this to 1.
	const int cIntegrationSubSteps = 1;

	// We simulate the physics world in discrete time steps. 60 Hz is a good 
	// rate to update the physics system.
	const float cDeltaTime = 1.0f / 60.f;


	// Step the world
	physics_system->Update(cDeltaTime, cCollisionSteps, cIntegrationSubSteps, 
		temp_allocator, job_system);

	// response string
	std::string stepPhysicsResponse = "";

	// Foreach body:
	for(auto& bodyId : BodyIdList)
	{
		// Output current position of the sphere
		RVec3 position = body_interface->GetCenterOfMassPosition(bodyId);

		const std::string actorStepPhysicsPositionResult = 
			std::to_string(position.GetX()) + ";" 
			+ std::to_string(position.GetY()) 
			+ ";" + std::to_string(position.GetZ());

		stepPhysicsResponse += std::to_string(bodyId.GetIndex()) 
			+ ";" + actorStepPhysicsPositionResult + ";";
		
		// Output current rotation of the sphere
		RVec3 rotation = body_interface->GetRotation(bodyId).GetEulerAngles();

		const std::string actorStepPhysicsRotationResult = 
			std::to_string(rotation.GetX()) + ";"
			+ std::to_string(rotation.GetY()) + ";" 
			+ std::to_string(rotation.GetZ());

		stepPhysicsResponse += actorStepPhysicsRotationResult + "\n";

		/*
		BodyLockWrite lockWrite(physics_system->GetBodyLockInterface(), 
			bodyId);
		if(lockWrite.Succeeded())
		{
			Body& body = lockWrite.GetBody();
			if(bodyId.GetIndex() % 2 == 0)
				body.SetLinearVelocity(Vec3(0.f, 1000.f, 0.f));
			else
				body.SetLinearVelocity(Vec3(0.f, -1000.f, 0.f));
			lockWrite.ReleaseLock();
		}*/
	}

	std::cout << "StepPhysics response:\n" << stepPhysicsResponse << "\n";
	return stepPhysicsResponse;
}

std::string PhysicsServiceImpl::AddNewSphereToPhysicsWorld
	(const BodyID newBodyId, const RVec3 newBodyInitialPosition)
{
	// Check if body interface is valid
	if(!body_interface)
	{
		return "No body interface valid when adding new sphere to world.\n";
	}

	// Create the settings for the body itself
	BodyCreationSettings sphere_settings(new SphereShape(50.f),
		newBodyInitialPosition, Quat::sIdentity(), 
		EMotionType::Dynamic, Layers::MOVING);

	// Set the sphere's restitution 
	sphere_settings.mRestitution = 1.f;

	// Create the actual rigid body
	// Note that if we run out of bodies this can return nullptr
	Body* newSphereBody = body_interface->CreateBodyWithID(newBodyId,
		sphere_settings);

	// Check for errors
	if(!newSphereBody)
	{
		std::string creationErrorString = "Fail in creation of body with ID: " 
			+ std::to_string(newBodyId.GetIndexAndSequenceNumber()) + '\n';
		return creationErrorString;
	}

	// Add the body's ID to the list of IDs
	BodyIdList.push_back(newBodyId);

	// Testing a small velocity on Y-axis
	newSphereBody->SetLinearVelocity(Vec3Arg(0.f, 1000.f, 0.f));

	// Add the new sphere to the world
	body_interface->AddBody(newSphereBody->GetID(), EActivation::Activate);
	return "New sphere body created succesfully.\n";
}

std::string PhysicsServiceImpl::AddNewFloorToPhysicsSystem
	(const BodyID newBodyId, const RVec3 newBodyInitialPosition)
{
	// Create the settings for the collision volume (the shape)
	BoxShapeSettings floor_shape_settings(Vec3(1000.0f, 1000.f, 100.0f));

	// Create the shape
	ShapeSettings::ShapeResult floor_shape_result = 
		floor_shape_settings.Create();

	// We don't expect an error here, but you can check floor_shape_result for 
	// HasError() / GetError()
	ShapeRefC floor_shape = floor_shape_result.Get(); 

	// Create the settings for the body itself. Note that here you can also set 
	// other properties like the restitution / friction.
	BodyCreationSettings floor_settings(floor_shape, newBodyInitialPosition, 
		Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	// Create the actual rigid body
	// Note that if we run out of bodies this can return nullptr
	Body* floor = body_interface->CreateBodyWithID(newBodyId, floor_settings); 

	// Check if floor was created successfuly
	if(!floor)
	{
		std::string creationErrorString = "Fail in creation of body with ID: " 
			+ std::to_string(newBodyId.GetIndexAndSequenceNumber()) + '\n';
		return creationErrorString;
	}

	// Set the floor's friction and add a small rotation on y-axis
    floor->SetFriction(1.0f);
	floor->AddRotationStep(RVec3(0.f, -0.01f, 0.f));

	// Add it to the world
	body_interface->AddBody(floor->GetID(), EActivation::DontActivate);

	return "New floor body created succesfully.\n";
}

std::string PhysicsServiceImpl::RemoveBodyByID(const BodyID bodyToRemoveID)
{
	std::cout << "Remove body by ID requested for id: " 
		+ std::to_string(bodyToRemoveID.GetIndex()) + "\n";

	// Check if body interface is valid
	if(!body_interface)
	{
		return "No body interface valid when removing body by ID.";
	}

	// Remove the ID from the list
	BodyIdList.erase(std::remove(BodyIdList.begin(), BodyIdList.end(), 
		bodyToRemoveID), BodyIdList.end());

	std::cout << "Removed from BodyIdList...\n";

	// Remove the body by its ID
	body_interface->RemoveBody(bodyToRemoveID);
	return "Body removal processed succesfully\n";
}

void PhysicsServiceImpl::ClearPhysicsSystem()
{
    std::cout << "Cleaning physics system...\n";

	for(auto& bodyId : BodyIdList)
	{
    	// Remove the sphere from the physics system. Note that the sphere 
		// itself keeps all of its state and can be re-added at any time.
		body_interface->RemoveBody(bodyId);

		// Destroy the sphere. After this the sphere ID is no longer valid.
		body_interface->DestroyBody(bodyId);
	}

	// Remove and destroy the floor
	//body_interface->RemoveBody(floor_id);
	//body_interface->DestroyBody(floor_id);

	// Unregisters all types with the factory and cleans up the default 
	// material
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;

	if(contact_listener) delete contact_listener;
	if(physics_system) delete physics_system;

	bIsInitialized = false;

    std::cout << "Physics system was cleared. Exiting process...\n";
}
