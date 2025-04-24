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
        public static async Task SendCommandsFromFileAsync(string filePath, string serialPortName, bool repeat, CancellationToken cancellationToken)
        {
            if (!File.Exists(filePath))
            {
                MessageBox.Show("File not found: " + filePath);
                return;
            }

            try
            {
                using SerialPort serialPort = new SerialPort(serialPortName, 9600);
                serialPort.Encoding = Encoding.ASCII;
                serialPort.Open();

                var allLines = File.ReadAllLines(filePath);
                StringBuilder logBuilder = new StringBuilder();

                do
                {
                    foreach (string line in allLines)
                    {
                        if (cancellationToken.IsCancellationRequested)
                            return;

                        string trimmed = line.Trim();
                        if (string.IsNullOrEmpty(trimmed)) continue;

                        int semicolonIndex = trimmed.IndexOf(';');
                        if (semicolonIndex == -1 || semicolonIndex + 1 >= trimmed.Length)
                            continue;

                        string commandPart = trimmed.Substring(0, semicolonIndex).Trim();
                        string delayPart = trimmed.Substring(semicolonIndex + 1).Trim();

                        foreach (char c in commandPart)
                        {
                            if (char.IsWhiteSpace(c)) continue;

                            serialPort.Write(c.ToString());
                            logBuilder.Append(c);
                        }

                        // Send semicolon
                        serialPort.Write(";");
                        logBuilder.Append(";");

                        if (int.TryParse(delayPart, out int delayMs))
                        {
                            logBuilder.AppendLine($"   Wait {delayMs} ms");
                            await Task.Delay(delayMs, cancellationToken);
                        }
                        else
                        {
                            logBuilder.AppendLine("   Invalid delay");
                        }

                        Application.Current.Dispatcher.Invoke(() =>
                        {
                            MessageBox.Show(logBuilder.ToString(), "Sent Commands");
                            logBuilder.Clear();
                        });
                    }
                }
                while (repeat && !cancellationToken.IsCancellationRequested);
            }
            catch (OperationCanceledException)
            {
                MessageBox.Show("Command sending was canceled.");
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error sending commands: " + ex.Message);
            }
        }
    }
}
