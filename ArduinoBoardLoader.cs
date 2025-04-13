using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using MobiDude_V2.Helpers;

namespace MobiDude_V2
{
    public class ArduinoBoard
    {
        public string Name { get; set; }
        public string MCU { get; set; }
        public string Tool { get; set; }
        public string Protocol { get; set; }
        public string Baudrate { get; set; }

        public override string ToString() => Name;
    }

    public static class ArduinoBoardLoader
    {
        private static readonly string BoardJsonPath = FilePathHelper.GetDataFilePath();
        public static List<ArduinoBoard> LoadBoards()
        {
            if (!File.Exists(BoardJsonPath))
            {
                throw new FileNotFoundException($"Board definitions not found at {BoardJsonPath}");
            }

            string jsonContent = File.ReadAllText(BoardJsonPath);
            return JsonSerializer.Deserialize<List<ArduinoBoard>>(jsonContent);
        }
    }
}
