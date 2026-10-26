namespace LLaX
{
    public static class Input
    {
        public static bool IsKeyDown(KeyCode keyCode)
        {
            if (InternalCalls.IsKeyDown != null)
                return InternalCalls.IsKeyDown((int)keyCode);
            return false;
        }
    }

    public static class Log
    {
        public static void Info(string message)
        {
            InternalCalls.LogMessage?.Invoke(0, message);
        }

        public static void Warn(string message)
        {
            InternalCalls.LogMessage?.Invoke(1, message);
        }

        public static void Error(string message)
        {
            InternalCalls.LogMessage?.Invoke(2, message);
        }
    }
}
