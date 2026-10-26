using System;
using LLaX;

namespace Sandbox
{
    public class Player : Entity
    {
        public float Speed = 5.0f;

        public override void OnCreate()
        {
            Log.Info($"[Player.cs] Player entity {ID} initialized in C#!");
        }

        public override void OnUpdate(float ts)
        {
            Vector3 translation = Translation;

            if (Input.IsKeyDown(KeyCode.A) || Input.IsKeyDown(KeyCode.Left))
                translation.X -= Speed * ts;
            if (Input.IsKeyDown(KeyCode.D) || Input.IsKeyDown(KeyCode.Right))
                translation.X += Speed * ts;
            if (Input.IsKeyDown(KeyCode.W) || Input.IsKeyDown(KeyCode.Up))
                translation.Y += Speed * ts;
            if (Input.IsKeyDown(KeyCode.S) || Input.IsKeyDown(KeyCode.Down))
                translation.Y -= Speed * ts;

            Translation = translation;
        }

        public override void OnDestroy()
        {
            Log.Info($"[Player.cs] Player entity {ID} destroyed.");
        }
    }

    public class Rotator : Entity
    {
        public float RotationSpeed = 90.0f; // degrees per second

        public override void OnCreate()
        {
            Log.Info($"[Rotator.cs] Rotator entity {ID} initialized in C#!");
        }

        public override void OnUpdate(float ts)
        {
            Vector3 rotation = Rotation;
            rotation.Z += (RotationSpeed * (float)Math.PI / 180.0f) * ts;
            Rotation = rotation;
        }
    }
}
