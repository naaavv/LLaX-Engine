using System;
using System.Runtime.InteropServices;

namespace LLaX
{
    public static class InternalCalls
    {
        public delegate bool IsKeyDownFn(int keyCode);
        public delegate void GetTranslationFn(ulong entityID, out Vector3 translation);
        public delegate void SetTranslationFn(ulong entityID, ref Vector3 translation);
        public delegate void GetRotationFn(ulong entityID, out Vector3 rotation);
        public delegate void SetRotationFn(ulong entityID, ref Vector3 rotation);
        public delegate void LogMessageFn(int level, string message);

        public static IsKeyDownFn? IsKeyDown;
        public static GetTranslationFn? GetTranslation;
        public static SetTranslationFn? SetTranslation;
        public static GetRotationFn? GetRotation;
        public static SetRotationFn? SetRotation;
        public static LogMessageFn? LogMessage;
    }
}
