#include "Simulator.h"

namespace ScriptingSystem
{
public ref class MPhysicsEngine
    {
    public:
        static Simulator^ TokSimulator;

        static MPhysicsEngine()
        {
            TokSimulator = gcnew Simulator(TokamakBody::TokSimulator);
        }

        static int CreateTokMaterial(float friction,float restitution)
        {
            return PhysicsEngine::Instance()->CreateTokMaterial(friction,restitution);
        }
    };
}