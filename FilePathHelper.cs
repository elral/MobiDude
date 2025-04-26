using System;
using System.IO;
using System.Reflection;

namespace MobiDude_V2.Helpers
{
    public static class FilePathHelper
    {
        public static string GetBasePath()
        {
            var assemblyLocation = Assembly.GetExecutingAssembly().Location;
            if (string.IsNullOrEmpty(assemblyLocation))
            {
                return AppContext.BaseDirectory;
            }

            return Path.GetDirectoryName(assemblyLocation) ?? AppContext.BaseDirectory;
        }

        public static string GetDataFilePath()
        {
            return Path.Combine(GetBasePath(), "Data", "arduino_boards.json");
        }

        public static string GetToolPath(string toolName)
        {
            string toolDir;

            if (toolName.Equals("esptool.exe", StringComparison.OrdinalIgnoreCase))
            {
                toolDir = Path.Combine(GetBasePath(), "Tools", "ESP-tool");
            }
            else if (toolName.Equals("avrdude.exe", StringComparison.OrdinalIgnoreCase))
            {
                toolDir = Path.Combine(GetBasePath(), "Tools", "AVRDUDE");
            }
            else
            {
                // Fallback, for additional tools
                toolDir = Path.Combine(GetBasePath(), "Tools");
            }

            return Path.Combine(toolDir, toolName);
        }
    }
}
