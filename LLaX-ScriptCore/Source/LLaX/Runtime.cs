using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;

namespace LLaX
{
    public static class Runtime
    {
        private static readonly Dictionary<ulong, Entity> s_EntityInstances = new Dictionary<ulong, Entity>();
        private static readonly Dictionary<string, Type> s_EntityClasses = new Dictionary<string, Type>();

        [UnmanagedCallersOnly]
        public static void Init(
            IntPtr isKeyDownPtr,
            IntPtr getTranslationPtr,
            IntPtr setTranslationPtr,
            IntPtr getRotationPtr,
            IntPtr setRotationPtr,
            IntPtr logMessagePtr)
        {
            if (isKeyDownPtr != IntPtr.Zero)
                InternalCalls.IsKeyDown = Marshal.GetDelegateForFunctionPointer<InternalCalls.IsKeyDownFn>(isKeyDownPtr);

            if (getTranslationPtr != IntPtr.Zero)
                InternalCalls.GetTranslation = Marshal.GetDelegateForFunctionPointer<InternalCalls.GetTranslationFn>(getTranslationPtr);

            if (setTranslationPtr != IntPtr.Zero)
                InternalCalls.SetTranslation = Marshal.GetDelegateForFunctionPointer<InternalCalls.SetTranslationFn>(setTranslationPtr);

            if (getRotationPtr != IntPtr.Zero)
                InternalCalls.GetRotation = Marshal.GetDelegateForFunctionPointer<InternalCalls.GetRotationFn>(getRotationPtr);

            if (setRotationPtr != IntPtr.Zero)
                InternalCalls.SetRotation = Marshal.GetDelegateForFunctionPointer<InternalCalls.SetRotationFn>(setRotationPtr);

            if (logMessagePtr != IntPtr.Zero)
                InternalCalls.LogMessage = Marshal.GetDelegateForFunctionPointer<InternalCalls.LogMessageFn>(logMessagePtr);

            // Discover all Entity subclasses
            s_EntityClasses.Clear();
            var assembly = typeof(Runtime).Assembly;
            foreach (var type in assembly.GetTypes())
            {
                if (type.IsSubclassOf(typeof(Entity)) && !type.IsAbstract)
                {
                    s_EntityClasses[type.FullName ?? type.Name] = type;
                    s_EntityClasses[type.Name] = type;
                }
            }

            Log.Info($"[C# ScriptCore] Initialized with {s_EntityClasses.Count} registered entity types.");
        }

        [UnmanagedCallersOnly]
        public static void OnCreateEntity(ulong entityID, IntPtr classNameUtf8)
        {
            string? className = Marshal.PtrToStringUTF8(classNameUtf8);
            if (string.IsNullOrEmpty(className)) return;

            if (s_EntityClasses.TryGetValue(className, out Type? type))
            {
                try
                {
                    var instance = (Entity?)Activator.CreateInstance(type);
                    if (instance != null)
                    {
                        instance.ID = entityID;
                        s_EntityInstances[entityID] = instance;
                        instance.OnCreate();
                    }
                }
                catch (Exception ex)
                {
                    Log.Error($"[C# ScriptCore] Error creating entity {entityID} ({className}): {ex.Message}");
                }
            }
            else
            {
                Log.Warn($"[C# ScriptCore] Class '{className}' not found for entity {entityID}");
            }
        }

        [UnmanagedCallersOnly]
        public static void OnUpdateEntity(ulong entityID, float ts)
        {
            if (s_EntityInstances.TryGetValue(entityID, out Entity? instance))
            {
                try
                {
                    instance.OnUpdate(ts);
                }
                catch (Exception ex)
                {
                    Log.Error($"[C# ScriptCore] Error updating entity {entityID}: {ex.Message}");
                }
            }
        }

        [UnmanagedCallersOnly]
        public static void OnDestroyEntity(ulong entityID)
        {
            if (s_EntityInstances.TryGetValue(entityID, out Entity? instance))
            {
                try
                {
                    instance.OnDestroy();
                }
                catch (Exception ex)
                {
                    Log.Error($"[C# ScriptCore] Error destroying entity {entityID}: {ex.Message}");
                }
                finally
                {
                    s_EntityInstances.Remove(entityID);
                }
            }
        }
    }
}
