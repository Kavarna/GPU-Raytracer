#pragma once



#include "commonincludes.h"
#include "Scene.h"


class Physics
{
private:
	btCollisionConfiguration *mCollisionConfiguration;
	btDispatcher *mDispatcher;
	btConstraintSolver *mSolver;
	btBroadphaseInterface *mBroadphase;
	btDynamicsWorld *mWorld;

private:
	static Physics * mInstance;
public:
	Physics( );
	~Physics( );
public:
	void Update( float frameTime );
	btRigidBody* CreateSphere( float mass, float radius, btVector3& coords, btVector3& eulerYPR, bool localInertia );
	btRigidBody* CreateBox( float mass, btVector3& extents, btVector3& coords, btVector3& eulerYPR, bool localInertia );
	btRigidBody* CreateCustomStaticRigidBody( std::vector<DirectX::XMFLOAT3>& vertices, std::vector<DWORD>& indices,
		btVector3& EulerYPRRotation, btVector3& Coords);
public:
	void DeleteRigidBody( btRigidBody * body );
public:
	static void CreateInstance( );
	static Physics* GetInstance( );
	static void DestroyInstance( );
};

