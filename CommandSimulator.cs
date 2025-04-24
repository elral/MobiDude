using System;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace MobiDude_V2
{
    public class CommandSender
    {
        private CancellationTokenSource? _cancellationTokenSource;

        public async Task SendCommandsFromFileAsync(string filePath, string comPort, bool repeat)
        {
            if (!File.Exists(filePath))
            {
                MessageBox.Show("File not found.");
                return;
            }

            _cancellationTokenSource = new CancellationTokenSource();
            var token = _cancellationTokenSource.Token;

            try
            {
                using SerialPort sp = new SerialPort(comPort, 115200);
                sp.DtrEnable = true;
                sp.Open();

                var fileLines = File.ReadAllLines(filePath);
                StringBuilder messageLog = new();

                Task messageBoxTask = Task.Run(() =>
                {
                    MessageBox.Show(messageLog.ToString(), "Sending...", MessageBoxButton.OK);
                    _cancellationTokenSource?.Cancel();
                });

                do
                {
                    foreach (var line in fileLines)
                    {
                        if (token.IsCancellationRequested)
                            break;

                        string trimmed = line.Trim();

                        if (string.IsNullOrEmpty(trimmed))
                            continue;

                        int semicolonIndex = trimmed.IndexOf(';');
                        if (semicolonIndex == -1 || semicolonIndex == trimmed.Length - 1)
                            continue;

                        string command = trimmed[..(semicolonIndex + 1)];
                        string delayString = trimmed[(semicolonIndex + 1)..].Trim();

                        // Zeichen einzeln senden
                        foreach (char c in command)
                        {
                            if (char.IsWhiteSpace(c))
                                continue;

                            sp.Write(c.ToString());
                            messageLog.Append(c);
                        }

                        if (int.TryParse(delayString, out int delay))
                        {
                            messageLog.Append($"  (Wait {delay} ms)\n");
                            await Task.Delay(delay, token);
                        }
                        else
                        {
                            messageLog.Append("  (Invalid wait time)\n");
                        }
                    }
                }
                while (repeat && !token.IsCancellationRequested);

                sp.Close();
            }
            catch (OperationCanceledException)
            {
                // Senden wurde abgebrochen
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error: {ex.Message}");
            }
        }

        public void CancelSending()
        {
            _cancellationTokenSource?.Cancel();
        }
    }
}
