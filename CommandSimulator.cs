using System;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace MobiDude_V2
{
    public static class CommandSimulator
    {
        public static async Task SendCommandsFromFileAsync(
            string filePath,
            string comPort,
            bool repeat,
            CancellationToken cancellationToken,
            UploadWindow uploadWindow)
        {
            if (!File.Exists(filePath))
                throw new FileNotFoundException("Command file not found.", filePath);
            if (string.IsNullOrWhiteSpace(comPort))
            {
                uploadWindow.AppendLine("No COM port selected.\n");
                return;
            }


            using var serialPort = new SerialPort(comPort, 9600, Parity.None, 8, StopBits.One)
            {
                Encoding = Encoding.ASCII,
                NewLine = "\n"
            };

            serialPort.Open();
            uploadWindow.AppendLine($"Opened COM port: {comPort}");

            string[] lines = await File.ReadAllLinesAsync(filePath, cancellationToken);
            string command = "";

            do
            {
                foreach (string line in lines)
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    string trimmedLine = line.Trim();
                    if (string.IsNullOrWhiteSpace(trimmedLine))
                        continue;

                    int semicolonIndex = trimmedLine.IndexOf(';');
                    if (semicolonIndex == -1 || semicolonIndex == trimmedLine.Length - 1)
                        continue; // Ungültiges Format

                    string commandPart = trimmedLine.Substring(0, semicolonIndex);
                    string waitPart = trimmedLine.Substring(semicolonIndex + 1).Trim();

                    // Zeichenweise senden (ohne Whitespace)
                    foreach (char c in commandPart)
                    {
                        if (!char.IsWhiteSpace(c))
                        {
                            serialPort.Write(c.ToString());
                            command += c;
                        }

                        cancellationToken.ThrowIfCancellationRequested();
                    }

                    // Semikolon ebenfalls senden
                    serialPort.Write(";");
                    command += command;

                    // Warten, wenn Zahl korrekt
                    if (int.TryParse(waitPart, out int waitTime))
                    {
                        command += $"  → Wait {waitTime} ms";
                        await Task.Delay(waitTime, cancellationToken);
                    }
                    else
                    {
                        command += "  (Invalid wait time)";
                    }
                    uploadWindow.AppendLine(command);
                    command = "";
                }
            } while (repeat && !cancellationToken.IsCancellationRequested);

            serialPort.Close();
            uploadWindow.AppendLine("Command sending finished.");
            uploadWindow.AppendLine("You can close this window now.");
        }
    }
}
