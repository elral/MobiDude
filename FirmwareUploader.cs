using System;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace MobiDude_V2
{
    public class FirmwareUploader
    {
        private readonly ArduinoBoard selectedBoard;
        private readonly string selectedPort;
        private readonly string filePath;
        private readonly UploadWindow uploadWindow;

        public FirmwareUploader(ArduinoBoard board, string port, string file, UploadWindow window)
        {
            selectedBoard = board;
            selectedPort = port;
            filePath = file;
            uploadWindow = window;
        }

        public static async Task StartUpload(string filePath, ArduinoBoard selectedBoard, string selectedPort, UploadWindow uploadWindow)
        {
            string tool = selectedBoard.Tool;
            string mcu = selectedBoard.MCU;
            string protocol = selectedBoard.Protocol;
            string baudrate = selectedBoard.Baudrate;
            string finalPort = selectedPort;

            uploadWindow.Show();
            uploadWindow.AppendLine($"Board: {selectedBoard.Name}");
            uploadWindow.AppendLine($"Datei: {Path.GetFileName(filePath)}");
            uploadWindow.AppendLine($"Port: {selectedPort}");
            uploadWindow.AppendLine("");

            if (tool == "ESP32tool" && !filePath.ToLower().Contains("merged"))
            {
                uploadWindow.AppendLine("❌ Please choose a merged .bin File!");
                return;
            }

            string[] drivesBefore = DriveInfo.GetDrives().Select(d => d.Name).ToArray();
            string[] portsBefore = SerialPort.GetPortNames();

            if (tool == "ESP32tool" || mcu == "atmega32u4" || tool == "Picotool")
            {
                try
                {
                    using (SerialPort sp = new SerialPort(selectedPort, 1200))
                    {
                        sp.Open();
                        //sp.Close();
                    }
                }
                catch (Exception ex)
                {
                    uploadWindow.AppendLine($"⚠️ Failure opening COM port: {ex.Message}");
                }
                await Task.Delay(1500); 
            }

            if (tool == "Picotool")
            {
                string[] drivesAfter = DriveInfo.GetDrives().Select(d => d.Name).ToArray();
                string newDrive = drivesAfter.Except(drivesBefore).FirstOrDefault();

                if (newDrive == null)
                {
                    uploadWindow.AppendLine("❌ New drive not found!");
                    return;
                }

                uploadWindow.AppendLine($"✅ New drive found: {newDrive}");
                string destFile = Path.Combine(newDrive, Path.GetFileName(filePath));

                try
                {
                    File.Copy(filePath, destFile, true);
                    uploadWindow.AppendLine("✅ File copied successfully.");
                }
                catch (Exception ex)
                {
                    uploadWindow.AppendLine($"❌ Failure during copying file: {ex.Message}");
                }
                uploadWindow.AppendLine("\n\rYou can close this window now.");
                return;
            }

            // ESP32 or atmega32u4 → check for new COM-Port
            if (tool == "ESP32tool" || mcu == "atmega32u4")
            {
                string[] portsAfter = SerialPort.GetPortNames();
                finalPort = portsAfter.Except(portsBefore).FirstOrDefault();

                if (finalPort == null)
                {
                    uploadWindow.AppendLine("❌ New COM port after activating bootloader not found.");
                    uploadWindow.AppendLine($"Original COM-Port {selectedPort} will be used.");
                    finalPort = selectedPort;
                }
                else
                {
                    uploadWindow.AppendLine($"✅ New COM-Port: {finalPort}");
                }
            }

            string toolPath = "";
            string args = "";

            if (tool == "ESP32tool")
            {
                toolPath = Path.Combine("Tools", "ESP-tool", "esptool.exe");
                args = $"--port {finalPort} write_flash 0x0000 \"{filePath}\"";
            }
            else if (tool == "AVRDude")
            {
                toolPath = Path.Combine("Tools", "AVRDude", "avrdude.exe");
                args = $"-v -p {mcu} -c {protocol} -P {finalPort} -b {baudrate} -D -U flash:w:\"{filePath}\":a";
            }

            if (!File.Exists(toolPath))
            {
                uploadWindow.AppendLine($"❌ Flash tool not found: {toolPath}");
                return;
            }

            uploadWindow.AppendLine($"⚙️ Starte {Path.GetFileName(toolPath)}...");
            await RunProcessAsync(toolPath, args, uploadWindow, tool);
        }


        public static async Task RunProcessAsync(string filePath, string arguments, UploadWindow uploadWindow, string tool)
        {
            try
            {
                ProcessStartInfo startInfo = new ProcessStartInfo
                {
                    FileName = filePath,
                    Arguments = arguments,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                using (Process process = Process.Start(startInfo))
                {
                    if (process == null)
                        return;

                    process.OutputDataReceived += (sender, e) =>
                    {
                        if (e.Data != null)
                        {
                            if (e.Data.Contains("error") || e.Data.Contains("failed") || e.Data.Contains("unexpected"))
                            {
                                uploadWindow.AppendLine($"[Failure] {e.Data}");
                            }
                            else
                            {
                                uploadWindow.AppendLine(e.Data);
                            }
                        }
                    };

                    process.ErrorDataReceived += (sender, e) =>
                    {
                        if (e.Data != null)
                        {
                            if (e.Data.Contains("error") || e.Data.Contains("failed") || e.Data.Contains("unexpected"))
                            {
                                uploadWindow.AppendLine($"[Failure] {e.Data}");
                            }
                            else
                            {
                                uploadWindow.AppendLine(e.Data);
                            }
                        }
                    };

                    process.BeginOutputReadLine();
                    process.BeginErrorReadLine();

                    await process.WaitForExitAsync();

                    if (process.ExitCode == 0)
                    {
                        uploadWindow.AppendLine("\n✅ Upload successfull finished!");
                    }
                    else
                    {
                        uploadWindow.AppendLine("\n❌ Upload failure.");
                        uploadWindow.AppendLine($"Failure code: {process.ExitCode}");
                    }
                    uploadWindow.AppendLine("\n\rYou can close this window now.");
                }
            }
            catch (Exception ex)
            {
                uploadWindow.AppendLine($"❌ Failure during start of process: {ex.Message}");
            }
        }
    }
}
