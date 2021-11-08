#include "Physics.h"

Physics* Physics::mInstance = nullptr;

void Physics::CreateInstance( )
{
	if ( !mInstance )
		mInstance = new Physics( );
}

void Physics::DestroyInstance( )
{
	delete mInstance;
}

Physics* Physics::GetInstance( )
{
	return mInstance;
}

Physics::Physics( )
{
	try
	{
		mCollisionConfiguration = new btDefaultCollisionConfiguration( );
		mDispatcher = new btCollisionDispatcher( mCollisionConfiguration );
		mSolver = new btSequentialImpulseConstraintSolver( );
		mBroadphase = new btDbvtBroadphase( );
		mWorld = new btDiscreteDynamicsWorld( mDispatcher, mBroadphase, mSolver, mCollisionConfiguration );
		mWorld->setGravity( btVector3( 0, -10, 0 ) );
	}
	CATCH;
}


Physics::~Physics( )
{
	delete mWorld;
	delete mBroadphase;
	delete mSolver;
	delete mDispatcher;
	delete mCollisionConfiguration;
}

btRigidBody* Physics::CreateSphere( float mass, float radius, btVector3& coords, btVector3& eulerYPR, bool localInertia )
{
	btCollisionShape * shape = new btSphereShape( radius );
	btMotionState * motionState = new btDefaultMotionState( );
	btMatrix3x3 rotationMatrix;
	rotationMatrix.setEulerYPR( eulerYPR.x( ), eulerYPR.y( ), eulerYPR.z( ) );
	motionState->setWorldTransform( btTransform( rotationMatrix, coords ) );
	btVector3 inertia = btVector3( 0, 0, 0 );
	if ( localInertia )
		shape->calculateLocalInertia( mass, inertia );

	btRigidBody::btRigidBodyConstructionInfo CI( mass, motionState, shape, inertia );

	btRigidBody * body = new btRigidBody( CI );
	mWorld->addRigidBody( body );
	if ( mass == 0 && radius != Scene::GROUND_RADIUS )
	{
		body->setFriction( 0.0f );
		body->setRollingFriction( 0.0f );
		body->setRestitution( 0.0f );
	}
	else
	{
		body->setFriction( 1.0f );
		body->setRollingFriction( 1.0f );
		body->setRestitution( 0.5f );
	}

	return body;
}

btRigidBody * Physics::CreateBox( float mass, btVector3 & extents, btVector3 & coords, btVector3 & eulerYPR, bool localInertia )
{
	btCollisionShape * shape = new btBoxShape( extents );
	btMotionState * motionState = new btDefaultMotionState( );
	btMatrix3x3 rotationMatrix;
	rotationMatrix.setEulerYPR( eulerYPR.x( ), eulerYPR.y( ), eulerYPR.z( ) );
	motionState->setWorldTransform( btTransform( rotationMatrix, coords ) );
	btVector3 inertia = btVector3( 0, 0, 0 );
	if ( localInertia )
		shape->calculateLocalInertia( mass, inertia );

	btRigidBody::btRigidBodyConstructionInfo CI( mass, motionState, shape, inertia );

	btRigidBody * body = new btRigidBody( CI );
	mWorld->addRigidBody( body );
	body->setFriction( btScalar( 1.0f ) );
	body->setRollingFriction( btScalar( 1.0f ) );

	return body;
}

btRigidBody * Physics::CreateCustomStaticRigidBody( std::vector<DirectX::XMFLOAT3>& vertices, std::vector<DWORD>& indices,
	btVector3 & EulerYPRRotation, btVector3 & Coords )
{
	btTriangleMesh * Mesh = new btTriangleMesh( false, false );
	for ( unsigned int i = 0; i < indices.size( ) / 3; ++i )
	{
		DWORD Index1 = indices[ i * 3 + 0 ];
		DWORD Index2 = indices[ i * 3 + 1 ];
		DWORD Index3 = indices[ i * 3 + 2 ];
		Mesh->addTriangle(
			btVector3( vertices[ Index1 ].x, vertices[ Index1 ].y, vertices[ Index1 ].z ),
			btVector3( vertices[ Index2 ].x, vertices[ Index2 ].y, vertices[ Index2 ].z ),
			btVector3( vertices[ Index3 ].x, vertices[ Index3 ].y, vertices[ Index3 ].z )
		);
	}
	btBvhTriangleMeshShape * BodyShape = new btBvhTriangleMeshShape( Mesh, true );
	btMatrix3x3 Rotation;
	Rotation.setEulerYPR( EulerYPRRotation.x( ), EulerYPRRotation.y( ), EulerYPRRotation.z( ) );
	btMotionState* State = new btDefaultMotionState(
		btTransform( Rotation, Coords )
	);
	btRigidBody::btRigidBodyConstructionInfo CI( 0, State, BodyShape );
	btRigidBody* Body = new btRigidBody( CI );
	mWorld->addRigidBody( Body );
	Body->setRestitution( 1.0f );
	return Body;
}

void Physics::Update( float frameTime )
{
	mWorld->stepSimulation( frameTime );
}

void Physics::DeleteRigidBody( btRigidBody * body )
{
	mWorld->removeRigidBody( body );
}
