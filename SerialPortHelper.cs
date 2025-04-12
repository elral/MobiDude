using System.IO.Ports;

namespace MobiDude_V2
{
    public static class SerialPortHelper
    {
        public static string ReadUntilSemicolon(SerialPort port)
        {
            string result = "";
            while (true)
            {
                int data = port.ReadChar();
                if (data == -1) break;
                char c = (char)data;
                result += c;
                if (c == ';') break;
            }
            return result;
        }

        public static string ReadUntilLengthOrSemicolon(SerialPort port, int maxLength)
        {
            string result = "";
            for (int i = 0; i < maxLength; i++)
            {
                try
                {
                    int data = port.ReadChar();
                    if (data == -1) break;
                    char c = (char)data;
                    result += c;
                    if (c == ';') break;
                }
                catch (TimeoutException)
                {
                    break;
                }
            }
            return result;
        }
    }
}
