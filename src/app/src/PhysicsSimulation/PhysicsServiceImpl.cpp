#include "PhysicsServiceImpl.h"
#include <ctime>
#include <cstdlib>

void PhysicsServiceImpl::InitPhysicsSystem
	(const std::string initializationActorsInfo)
{
    std::cout << "Initializing physics system...\n";
    std::cout << "InitializationInfo:\n" << initializationActorsInfo << '\n';

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
	const uint cMaxBodies = 128000;

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

	// Split actors info from initialization into lines
	std::stringstream initializationStringStream(initializationActorsInfo);
    std::vector<std::string> initializationActorsInfoLines;

	std::string line;
    while (std::getline(initializationStringStream, line)) 
	{
        initializationActorsInfoLines.push_back(line);
    }

	// for each line, create a new body with according to the
	// body's type, id and initial location
	for(int i = 0; i < initializationActorsInfoLines.size(); i++)
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
		if(actorInfoList.size() < 6)
		{
        	std::cout << "Error on parsing addBody message info. Line with "
			"less than 6 params: " << initializationActorsInfoLines[i] << '\n';
			continue;
		}

		// Get the actor's type to be creates
		const std::string actorType { actorInfoList[0] };

		// Get the actor ID from the init info
		const int actorId { std::stoi(actorInfoList[1])} ;
		const BodyID newBodyID(actorId);

		// Get the new body type as string
		const std::string newBodyTypeAsString { actorInfoList[2] };

		// Set the new body type
		EBodyType newBodyType {};
		if(newBodyTypeAsString == "primary")
		{
			newBodyType = EBodyType::Primary;
		}
		else if(newBodyTypeAsString == "clone")
		{
			newBodyType = EBodyType::Clone;
		}
		else
		{
			std::cout << "Unknown body type: " << newBodyTypeAsString << '\n';
		}

		// Get actor initial pos
		const double initialPosX { std::stod(actorInfoList[3]) };
		const double initialPosY { std::stod(actorInfoList[4]) };
		const double initialPosZ { std::stod(actorInfoList[5]) };

		const RVec3 bodyInitialPosition { RVec3(initialPosX, initialPosY,
			initialPosZ) };

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
			AddNewSphereToPhysicsWorld(newBodyID, newBodyType, 
				bodyInitialPosition, RVec3(), RVec3());
			continue;
		}
	}

	// Seed the random number generator with the current time
	srand(static_cast<unsigned int>(time(0)));

	// Optional step: Before starting the physics simulation you can optimize 
	// the broad phase. This improves collision detection performance 
	// (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. 
	// streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to 
	// keep the broad phase efficient.
	//physics_system->OptimizeBroadPhase();

	// Reset the step physics time measurement 
    physicsStepSimulationTimeMeasure = "";

	bIsInitialized = true;

    std::cout << "Physics world has been initialized and is running.\n";
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

	// response string
	std::string stepPhysicsResponse = "";

	// Foreach body:
	/*		
	for(auto& bodyId : BodyIdList)
	{

		BodyLockWrite lockWrite(physics_system->GetBodyLockInterface(), 
			bodyId);
		if(lockWrite.Succeeded())
		{
			
			// Seed the random number generator with the current time
			srand(static_cast<unsigned int>(time(0)));

			// Use a hash function to map the ID to a random seed
			size_t hashValue = std::hash<int>{}(bodyId.GetIndex());

			// Seed the random number generator with the hash value
			srand(static_cast<unsigned int>(hashValue));

			// Generate a random number between 0 and 3
			int randomNum = rand() % 4;

			Body& body = lockWrite.GetBody();
			if(randomNum == 0)
				body.SetLinearVelocity(Vec3(0.f, 300.f, 0.f));
			else if (randomNum == 1)
				body.SetLinearVelocity(Vec3(300.f, 0.0f, 0.f));
			else if (randomNum == 2)
				body.SetLinearVelocity(Vec3(0.0f, -300.f, 0.f));
			else
				body.SetLinearVelocity(Vec3(-300.f, 0.0f, 0.f));
			

			lockWrite.ReleaseLock();
		}
	}
	*/

    // Get pre step physics time
    std::chrono::steady_clock::time_point preStepPhysicsTime = 
		std::chrono::steady_clock::now();

	// Step the world
	std::cout << "Stepping physics...\n";
	physics_system->Update(cDeltaTime, cCollisionSteps, cIntegrationSubSteps, 
		temp_allocator, job_system);
	std::cout << "Physics stepping finished.\n";

    // Get post physics communication time
    std::chrono::steady_clock::time_point postStepPhysicsTime = 
		std::chrono::steady_clock::now();

    // Calculate the microsseconds all step physics simulation
    // (considering communication )took
    std::stringstream ss;
    ss << std::chrono::duration_cast<std::chrono::microseconds>
		(postStepPhysicsTime - preStepPhysicsTime).count();
    const std::string elapsedTime = ss.str();

    // Append the delta time to the current step measurement
    physicsStepSimulationTimeMeasure += elapsedTime + "\n";

	std::cout << "(Step:" << stepPhysicsCounter++ << ")\n";

	// For each body on the physics system:
	for(auto& bodyId : BodyIdList)
	{
		std::string bodyStepResultInfo {};

		// Apend the body Id as the first info on the body physics response
		bodyStepResultInfo += std::to_string(bodyId.GetIndex()) + ";";

		// Output current position of the sphere
		RVec3 position = body_interface->GetCenterOfMassPosition(bodyId);

		const std::string actorStepPhysicsPositionResult = 
			std::to_string(position.GetX()) + ";" 
			+ std::to_string(position.GetY()) 
			+ ";" + std::to_string(position.GetZ());

		// Append the body's physics position result
		bodyStepResultInfo += actorStepPhysicsPositionResult + ";";
		
		// Output current rotation of the sphere
		RVec3 rotation = body_interface->GetRotation(bodyId).GetEulerAngles();

		const std::string actorStepPhysicsRotationResult = 
			std::to_string(rotation.GetX()) + ";"
			+ std::to_string(rotation.GetY()) + ";" 
			+ std::to_string(rotation.GetZ());

		// Append the the body's physics rotation result
		bodyStepResultInfo += actorStepPhysicsRotationResult + ";";

		// Get the linear and angular velocity
		const auto linearVelocity { body_interface->GetLinearVelocity(bodyId) };
		const auto angularVelocity 
			{ body_interface->GetAngularVelocity(bodyId) };

		// Create the velocities string
		const std::string actorStepPhysicsVelocitiesResult = 
			std::to_string(linearVelocity.GetX()) + ";"
			+ std::to_string(linearVelocity.GetY()) + ";" 
			+ std::to_string(linearVelocity.GetZ()) + ";" 
			+ std::to_string(angularVelocity.GetX()) + ";" 
			+ std::to_string(angularVelocity.GetY()) + ";" 
			+ std::to_string(angularVelocity.GetZ()); 

		// Append the the body's physics velocity result
		bodyStepResultInfo += actorStepPhysicsVelocitiesResult + '\n';

		// Create the bodyType variable
		std::string bodyTypeAsString {};

		// Get the body lock
		BodyLockWrite lockWrite(physics_system->GetBodyLockInterface(), 
			bodyId);
		if(lockWrite.Succeeded())
		{
			// Get the body
			Body& body = lockWrite.GetBody();

			// Access the body's user data
			uint64_t bodyRuntimeDataAddress = body.GetUserData();

			// Cast to BodyRuntimeData
			BodyRuntimeData* bodyRuntimeData = 
				reinterpret_cast<BodyRuntimeData*>(bodyRuntimeDataAddress);

			// Get the body type
			bodyTypeAsString = bodyRuntimeData->GetBodyTypeAsString();

			lockWrite.ReleaseLock();
		}

		// Print the body's result
		std::cout <<  "\t(" << bodyTypeAsString << ")" << bodyStepResultInfo;

		// Append the body step result info to the step physics response
		stepPhysicsResponse += bodyStepResultInfo;
	}

	/*
	std::cout << "(Step:" << stepPhysicsCounter++ << ")" 
		<< "StepPhysics response:\n" << stepPhysicsResponse << "\n";
	*/
	
	return stepPhysicsResponse;
}

std::string PhysicsServiceImpl::AddNewSphereToPhysicsWorld
	(BodyID newBodyId, EBodyType newBodyType, RVec3 newBodyInitialPosition,
    RVec3 newBodyInitialLinearVelocity, RVec3 newBodyInitialAngularVelocity)
{
	std::cout << "NewSphere addition to physics world requested.\n";

	// Check if body interface is valid
	if(!body_interface)
	{
		std::cout << "No body interface valid when adding new sphere to world.\n";
		return "No body interface valid when adding new sphere to world.\n";
	}

	// Create the settings for the body itself
	BodyCreationSettings sphere_settings(new SphereShape(50.f),
		newBodyInitialPosition, Quat::sIdentity(), 
		EMotionType::Dynamic, Layers::MOVING);

	// Set the sphere's restitution 
	sphere_settings.mRestitution = 1.f;
	sphere_settings.mMassPropertiesOverride.mMass = 10.f;

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

	// Create the new body runtime data
	auto newBodyRuntimeData = new BodyRuntimeData();

	// Set the new body type
	newBodyRuntimeData->SetBodyType(newBodyType);

	// Cast to a uint64_t so we can store it on user data
	uint64_t bodyRuntimeDataAsInt = reinterpret_cast<uint64_t>
		(newBodyRuntimeData);
	newSphereBody->SetUserData(bodyRuntimeDataAsInt);

	// Add the body's ID to the list of IDs
	BodyIdList.push_back(newBodyId);

	// Set the body's linear velocity
	newSphereBody->SetLinearVelocity(newBodyInitialLinearVelocity);

	// Set the body's angular velocity
	newSphereBody->SetAngularVelocity(newBodyInitialAngularVelocity);

	// Add the new sphere to the world
	body_interface->AddBody(newSphereBody->GetID(), EActivation::Activate);

	return "New sphere body created successfully.";
}

std::string PhysicsServiceImpl::AddNewFloorToPhysicsSystem
	(const BodyID newBodyId, const RVec3 newBodyInitialPosition)
{
	std::cout << "NewFloor addition to physics world requested.\n";

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

	// Check if floor was created successfully
	if(!floor)
	{
		std::string creationErrorString = "Fail in creation of body with ID: " 
			+ std::to_string(newBodyId.GetIndexAndSequenceNumber()) + '\n';
		return creationErrorString;
	}

	// Set the floor's friction and add a small rotation on y-axis
    floor->SetFriction(1.0f);
	//floor->AddRotationStep(RVec3(0.f, -0.01f, 0.f));

	// Add it to the world
	body_interface->AddBody(floor->GetID(), EActivation::DontActivate);

	return "New floor body created successfully.";
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

	// Remove the body by its ID and destroy it
	body_interface->RemoveBody(bodyToRemoveID);
	body_interface->DestroyBody(bodyToRemoveID);

	return "Body removal processed successfully";
}

std::string PhysicsServiceImpl::UpdateBodyType(BodyID bodyIdToUpdate,
	EBodyType newBodyType)
{
	// Get the body lock
	BodyLockWrite lockWrite(physics_system->GetBodyLockInterface(),
		bodyIdToUpdate);

	// Check if got the lock
	if (lockWrite.Succeeded())
	{
		// Get the body
		Body &body = lockWrite.GetBody();

		// Access the body's user data
		uint64_t bodyRuntimeDataAddress = body.GetUserData();

		// Cast to BodyRuntimeData
		BodyRuntimeData *bodyRuntimeData =
			reinterpret_cast<BodyRuntimeData *>(bodyRuntimeDataAddress);

		// Update the body type
		bodyRuntimeData->SetBodyType(newBodyType);

		lockWrite.ReleaseLock();
	}

	return "Body type updated successfully.";
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

std::string PhysicsServiceImpl::GetSimulationMeasures() const
{
	return physicsStepSimulationTimeMeasure;
}
