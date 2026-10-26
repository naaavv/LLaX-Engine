namespace LLaX
{
    public class Entity
    {
        public ulong ID { get; internal set; }

        public Entity()
        {
            ID = 0;
        }

        internal Entity(ulong id)
        {
            ID = id;
        }

        public Vector3 Translation
        {
            get
            {
                if (InternalCalls.GetTranslation != null)
                {
                    InternalCalls.GetTranslation(ID, out Vector3 result);
                    return result;
                }
                return Vector3.Zero;
            }
            set
            {
                InternalCalls.SetTranslation?.Invoke(ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                if (InternalCalls.GetRotation != null)
                {
                    InternalCalls.GetRotation(ID, out Vector3 result);
                    return result;
                }
                return Vector3.Zero;
            }
            set
            {
                InternalCalls.SetRotation?.Invoke(ID, ref value);
            }
        }

        public virtual void OnCreate() {}
        public virtual void OnUpdate(float ts) {}
        public virtual void OnDestroy() {}
    }
}
