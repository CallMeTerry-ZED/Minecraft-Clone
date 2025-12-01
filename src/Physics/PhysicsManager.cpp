/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Physics/PhysicsManager.h"
#include "Physics/CharacterController.h"
#include "World/World.h"
#include "World/Chunk.h"
#include "World/BlockType.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    PhysicsManager::PhysicsManager() : m_characterController(nullptr)
    {
    }

    PhysicsManager::~PhysicsManager()
    {
        Shutdown();
    }

    bool PhysicsManager::Initialize()
    {
        spdlog::info("PhysicsManager::Initialize() - Starting...");
        
        // Create collision configuration
        spdlog::info("PhysicsManager::Initialize() - Creating collision configuration...");
        m_collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
        spdlog::info("PhysicsManager::Initialize() - Collision configuration created");

        // Create dispatcher
        spdlog::info("PhysicsManager::Initialize() - Creating dispatcher...");
        m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfiguration.get());
        spdlog::info("PhysicsManager::Initialize() - Dispatcher created");

        // Create broadphase
        spdlog::info("PhysicsManager::Initialize() - Creating broadphase...");
        m_broadphase = std::make_unique<btDbvtBroadphase>();
        spdlog::info("PhysicsManager::Initialize() - Broadphase created");

        // Create solver
        spdlog::info("PhysicsManager::Initialize() - Creating solver...");
        m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        spdlog::info("PhysicsManager::Initialize() - Solver created");

        // Create dynamics world
        spdlog::info("PhysicsManager::Initialize() - Creating dynamics world...");
        m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
            m_dispatcher.get(),
            m_broadphase.get(),
            m_solver.get(),
            m_collisionConfiguration.get()
        );
        spdlog::info("PhysicsManager::Initialize() - Dynamics world created");

        // Set gravity (negative Y is down)
        spdlog::info("PhysicsManager::Initialize() - Setting gravity...");
        m_dynamicsWorld->setGravity(btVector3(0, -20.0f, 0)); // 20 units/sec^2
        spdlog::info("PhysicsManager::Initialize() - Gravity set");

        spdlog::info("PhysicsManager initialized successfully");
        return true;
    }

    void PhysicsManager::Shutdown()
    {
        // Remove character controller
        if (m_characterController)
        {
            delete m_characterController;
            m_characterController = nullptr;
        }

        // Remove all chunk bodies
        for (auto& [coord, body] : m_chunkBodies)
        {
            if (body)
            {
                m_dynamicsWorld->removeRigidBody(body);
                delete body->getMotionState();
                delete body->getCollisionShape();
                delete body;
            }
        }
        m_chunkBodies.clear();

        // Clean up Bullet objects (in reverse order of creation)
        m_dynamicsWorld.reset();
        m_solver.reset();
        m_broadphase.reset();
        m_dispatcher.reset();
        m_collisionConfiguration.reset();
    }

    void PhysicsManager::Update(float deltaTime)
    {
        if (!m_dynamicsWorld)
        {
            return;
        }

        // Step simulation
        if (m_characterController)
        {
            // Step simulation with fixed timestep for stability
            // Use smaller fixed timestep to prevent jitter
            const float fixedTimeStep = 1.0f / 60.0f;  // 60 FPS fixed timestep
            m_dynamicsWorld->stepSimulation(deltaTime, 10, fixedTimeStep);
            
            // Update character controller
            m_characterController->Update(deltaTime);
        }
    }

    void PhysicsManager::AddChunkCollision(Chunk* chunk, int chunkX, int chunkZ, World* world)
    {
        if (!chunk || !world)
        {
            return;
        }

        auto key = std::make_pair(chunkX, chunkZ);
        if (m_chunkBodies.find(key) != m_chunkBodies.end())
        {
            // Already added
            return;
        }

        // Create collision mesh from chunk blocks (only exposed faces)
        btTriangleMesh* mesh = new btTriangleMesh();

        // Helper function to check if a face should have collision
        auto ShouldAddFaceCollision = [&](int x, int y, int z, int faceIndex) -> bool {
            int neighborX = x;
            int neighborY = y;
            int neighborZ = z;
            int neighborChunkX = chunkX;
            int neighborChunkZ = chunkZ;

            switch (faceIndex)
            {
                case 0: // Front (+Z)
                    neighborZ++;
                    if (neighborZ >= CHUNK_SIZE_Z)
                    {
                        neighborZ = 0;
                        neighborChunkZ++;
                    }
                    break;
                case 1: // Back (-Z)
                    neighborZ--;
                    if (neighborZ < 0)
                    {
                        neighborZ = CHUNK_SIZE_Z - 1;
                        neighborChunkZ--;
                    }
                    break;
                case 2: // Left (-X)
                    neighborX--;
                    if (neighborX < 0)
                    {
                        neighborX = CHUNK_SIZE_X - 1;
                        neighborChunkX--;
                    }
                    break;
                case 3: // Right (+X)
                    neighborX++;
                    if (neighborX >= CHUNK_SIZE_X)
                    {
                        neighborX = 0;
                        neighborChunkX++;
                    }
                    break;
                case 4: // Top (+Y)
                    neighborY++;
                    if (neighborY >= CHUNK_SIZE_Y)
                    {
                        return true; // Always add collision at world height limit
                    }
                    break;
                case 5: // Bottom (-Y)
                    neighborY--;
                    if (neighborY < 0)
                    {
                        return true; // Always add collision at world bottom
                    }
                    break;
            }

            // Check if neighbor is in different chunk
            if (neighborChunkX != chunkX || neighborChunkZ != chunkZ)
            {
                glm::ivec3 worldPos = Chunk::LocalToWorld(neighborChunkX, neighborChunkZ, neighborX, neighborY, neighborZ);
                const Block& neighborBlock = world->GetBlock(worldPos.x, worldPos.y, worldPos.z);
                return neighborBlock.IsAir() || neighborBlock.IsTransparent();
            }
            else
            {
                const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
                return neighborBlock.IsAir() || neighborBlock.IsTransparent();
            }
        };

        // Iterate through all blocks in chunk
        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < CHUNK_SIZE_Z; z++)
            {
                for (int x = 0; x < CHUNK_SIZE_X; x++)
                {
                    const Block& block = chunk->GetBlock(x, y, z);
                    if (block.IsAir() || !block.IsSolid())
                    {
                        continue;
                    }

                    // Convert to world coordinates
                    glm::ivec3 worldPos = Chunk::LocalToWorld(chunkX, chunkZ, x, y, z);
                    float wx = static_cast<float>(worldPos.x);
                    float wy = static_cast<float>(worldPos.y);
                    float wz = static_cast<float>(worldPos.z);

                    // Only add faces that are exposed (similar to mesh generation)
                    // Front face (+Z)
                    if (ShouldAddFaceCollision(x, y, z, 0))
                    {
                        mesh->addTriangle(
                            btVector3(wx, wy, wz + 1.0f),
                            btVector3(wx + 1.0f, wy, wz + 1.0f),
                            btVector3(wx + 1.0f, wy + 1.0f, wz + 1.0f)
                        );
                        mesh->addTriangle(
                            btVector3(wx, wy, wz + 1.0f),
                            btVector3(wx + 1.0f, wy + 1.0f, wz + 1.0f),
                            btVector3(wx, wy + 1.0f, wz + 1.0f)
                        );
                    }

                    // Back face (-Z)
                    if (ShouldAddFaceCollision(x, y, z, 1))
                    {
                        mesh->addTriangle(
                            btVector3(wx + 1.0f, wy, wz),
                            btVector3(wx, wy, wz),
                            btVector3(wx, wy + 1.0f, wz)
                        );
                        mesh->addTriangle(
                            btVector3(wx + 1.0f, wy, wz),
                            btVector3(wx, wy + 1.0f, wz),
                            btVector3(wx + 1.0f, wy + 1.0f, wz)
                        );
                    }

                    // Left face (-X)
                    if (ShouldAddFaceCollision(x, y, z, 2))
                    {
                        mesh->addTriangle(
                            btVector3(wx, wy, wz),
                            btVector3(wx, wy, wz + 1.0f),
                            btVector3(wx, wy + 1.0f, wz + 1.0f)
                        );
                        mesh->addTriangle(
                            btVector3(wx, wy, wz),
                            btVector3(wx, wy + 1.0f, wz + 1.0f),
                            btVector3(wx, wy + 1.0f, wz)
                        );
                    }

                    // Right face (+X)
                    if (ShouldAddFaceCollision(x, y, z, 3))
                    {
                        mesh->addTriangle(
                            btVector3(wx + 1.0f, wy, wz + 1.0f),
                            btVector3(wx + 1.0f, wy, wz),
                            btVector3(wx + 1.0f, wy + 1.0f, wz)
                        );
                        mesh->addTriangle(
                            btVector3(wx + 1.0f, wy, wz + 1.0f),
                            btVector3(wx + 1.0f, wy + 1.0f, wz),
                            btVector3(wx + 1.0f, wy + 1.0f, wz + 1.0f)
                        );
                    }

                    // Top face (+Y)
                    if (ShouldAddFaceCollision(x, y, z, 4))
                    {
                        mesh->addTriangle(
                            btVector3(wx, wy + 1.0f, wz),
                            btVector3(wx, wy + 1.0f, wz + 1.0f),
                            btVector3(wx + 1.0f, wy + 1.0f, wz + 1.0f)
                        );
                        mesh->addTriangle(
                            btVector3(wx, wy + 1.0f, wz),
                            btVector3(wx + 1.0f, wy + 1.0f, wz + 1.0f),
                            btVector3(wx + 1.0f, wy + 1.0f, wz)
                        );
                    }

                    // Bottom face (-Y)
                    if (ShouldAddFaceCollision(x, y, z, 5))
                    {
                        mesh->addTriangle(
                            btVector3(wx, wy, wz + 1.0f),
                            btVector3(wx, wy, wz),
                            btVector3(wx + 1.0f, wy, wz)
                        );
                        mesh->addTriangle(
                            btVector3(wx, wy, wz + 1.0f),
                            btVector3(wx + 1.0f, wy, wz),
                            btVector3(wx + 1.0f, wy, wz + 1.0f)
                        );
                    }
                }
            }
        }

        // Only create body if mesh has triangles
        if (mesh->getNumTriangles() > 0)
        {
            btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(mesh, true);
            shape->setMargin(0.0f);

            btTransform transform;
            transform.setIdentity();

            btDefaultMotionState* motionState = new btDefaultMotionState(transform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, shape);
            btRigidBody* body = new btRigidBody(rbInfo);
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

            m_dynamicsWorld->addRigidBody(body);
            m_chunkBodies[key] = body;
        }
        else
        {
            delete mesh;
        }
    }

    void PhysicsManager::RemoveChunkCollision(int chunkX, int chunkZ)
    {
        auto key = std::make_pair(chunkX, chunkZ);
        auto it = m_chunkBodies.find(key);
        if (it != m_chunkBodies.end())
        {
            btRigidBody* body = it->second;
            m_dynamicsWorld->removeRigidBody(body);
            delete body->getMotionState();
            delete body->getCollisionShape();
            delete body;
            m_chunkBodies.erase(it);
        }
    }

    void PhysicsManager::UpdateChunkCollision(Chunk* chunk, int chunkX, int chunkZ, World* world)
    {
        // Remove old collision and add new one
        RemoveChunkCollision(chunkX, chunkZ);
        AddChunkCollision(chunk, chunkX, chunkZ, world);
    }

    CharacterController* PhysicsManager::CreateCharacterController(const glm::vec3& position)
    {
        if (m_characterController)
        {
            delete m_characterController;
        }

        m_characterController = new CharacterController(m_dynamicsWorld.get(), position);
        return m_characterController;
    }

    void PhysicsManager::RemoveCharacterController()
    {
        if (m_characterController)
        {
            delete m_characterController;
            m_characterController = nullptr;
        }
    }
}

