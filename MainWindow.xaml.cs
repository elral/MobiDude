using System;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Text.Json;
using System.Threading;
using System.Windows;
using Microsoft.Win32;
using System.Management;
using System.Windows.Media;

namespace MobiDude_V2
{
    public partial class MainWindow : Window
    {
        private List<ArduinoBoard> boardList = new();
        private UploadWindow? uploadWindow;

        public MainWindow()
        {
            InitializeComponent();
            StartUsbDeviceWatcher();
            LoadBoardList();
            RefreshSerialPorts();
            this.Closed += MainWindow_Closed;
        }

        private ManagementEventWatcher insertWatcher;
        private ManagementEventWatcher removeWatcher;

        protected override void OnClosed(EventArgs e)
        {
            insertWatcher?.Stop();
            removeWatcher?.Stop();
            insertWatcher?.Dispose();
            removeWatcher?.Dispose();

            base.OnClosed(e);
        }

        private void StartUsbDeviceWatcher()
        {
            try
            {
                WqlEventQuery insertQuery = new WqlEventQuery("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2");
                WqlEventQuery removeQuery = new WqlEventQuery("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 3");

                insertWatcher = new ManagementEventWatcher(insertQuery);
                insertWatcher.EventArrived += (s, e) => Dispatcher.Invoke(RefreshSerialPorts);
                insertWatcher.Start();

                removeWatcher = new ManagementEventWatcher(removeQuery);
                removeWatcher.EventArrived += (s, e) => Dispatcher.Invoke(RefreshSerialPorts);
                removeWatcher.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error starting USB device watcher: {ex.Message}");
            }
        }

        private void MainWindow_Closed(object? sender, EventArgs e)
        {
            uploadWindow?.Close();
        }

        private void ExitButton_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void ShowUploadWindow()
        {
            if (uploadWindow == null || !uploadWindow.IsVisible)
            {
                uploadWindow = new UploadWindow();
                uploadWindow.Show();
            }
            else
            {
                uploadWindow.Activate(); // Falls schon offen, bring nach vorne
            }
        }

        private void LoadBoardList()
        {
            try
            {
                boardList = ArduinoBoardLoader.LoadBoards();
                if (boardList == null || boardList.Count == 0)
                {
                    MessageBox.Show("No boards are loaded. Please check arduino_boards.json.");
                    return;
                }
                ArduinoBoardComboBox.ItemsSource = boardList;
                ArduinoBoardComboBox.SelectedIndex = 0;
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failure during loading of board definitions: {ex.Message}");
            }
        }

        private void RefreshSerialPorts()
        {
            //SerialPortComboBox.ItemsSource = SerialPort.GetPortNames();
            //SerialPortComboBox.SelectedIndex = 0;
            var ports = SerialPort.GetPortNames();
            SerialPortComboBox.ItemsSource = ports;
            if (ports.Length > 0)
                SerialPortComboBox.SelectedItem = ports[0];
        }

        private void OpenFileButton_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog
            {
                Filter = "Firmware Files (*.hex;*.bin;*.uf2)|*.hex;*.bin;*.uf2|All files (*.*)|*.*"
            };
            if (ofd.ShowDialog() == true)
            {
                SelectedFileText.Text = ofd.FileName;
            }
        }

        private void TerminalButton_Click(object sender, RoutedEventArgs e)
        {
            if (SerialPortComboBox.SelectedItem is not string selectedPort)
            {
                MessageBox.Show("Please choose a COM-Port.");
                return;
            }

            string puttyPath = Path.Combine("Tools", "PuTTYPortable", "PuTTYPortable.exe");
            if (!File.Exists(puttyPath))
            {
                MessageBox.Show("PuTTYPortable wasn't found.");
                return;
            }

            var psi = new System.Diagnostics.ProcessStartInfo
            {
                FileName = puttyPath,
                Arguments = $"-serial {selectedPort} -sercfg 115200,8,n,1,N"
            };
            System.Diagnostics.Process.Start(psi);
        }

        private void GetInfoButton_Click(object sender, RoutedEventArgs e)
        {
            if (SerialPortComboBox.SelectedItem is not string selectedPort)
            {
                MessageBox.Show("Please choose a COM-Port.");
                return;
            }

            try
            {
                using var sp = new SerialPort(selectedPort, 115200, Parity.None, 8, StopBits.One)
                {
                    ReadTimeout = 2000,
                    WriteTimeout = 1000
                };

                sp.Open();
                sp.DtrEnable = true;
                Thread.Sleep(2000);
                sp.DiscardInBuffer();

                sp.Write("9;");
                string info = ReadUntilSemicolon(sp);

                sp.DiscardInBuffer();
                sp.Write("12;");
                string config = ReadUntilSemicolon(sp, 2000);

                var infoWindow = new InfoWindow(info, config);
                infoWindow.ShowDialog();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Read error while reading from COM port: {ex.Message}");
            }
        }

        private string ReadUntilSemicolon(SerialPort sp, int maxBytes = 1000)
        {
            StringBuilder sb = new();
            try
            {
                while (sb.Length < maxBytes)
                {
                    int c = sp.ReadChar();
                    if (c == -1) break;
                    char ch = (char)c;
                    sb.Append(ch);
                    if (ch == ';') break;
                }
            }
            catch { }
            return sb.ToString();
        }

        private async void UploadButton_Click(object sender, RoutedEventArgs e)
        {
            if (ArduinoBoardComboBox.SelectedItem is not ArduinoBoard selectedBoard)
            {
                MessageBox.Show("Please choose a board.");
                return;
            }

            if (SerialPortComboBox.SelectedItem is not string selectedPort)
            {
                MessageBox.Show("Please choose a COM-Port.");
                return;
            }

            string selectedFilePath = SelectedFileText.Text;
            if (string.IsNullOrEmpty(selectedFilePath) || !File.Exists(selectedFilePath))
            {
                MessageBox.Show("Please choose a valid firmware file.");
                return;
            }

            ShowUploadWindow();
            await FirmwareUploader.StartUpload(selectedFilePath, selectedBoard, selectedPort, uploadWindow!);
            uploadWindow!.AppendLine("\n\rYou can close this window now.");
        }

        private void RefreshComPortListButton_Click(object sender, RoutedEventArgs e)
        {
            var comPorts = SerialPort.GetPortNames();
            SerialPortComboBox.ItemsSource = comPorts;
            if (comPorts.Length > 0 && SerialPortComboBox.SelectedItem == null)
            {
                SerialPortComboBox.SelectedIndex = 0;
            }
        }

        private CancellationTokenSource cancellationTokenSource;

        private void OpenCommandFileButton_Click(object sender, RoutedEventArgs e)
        {
            // Hier kannst du den OpenFileDialog verwenden, um eine Datei auszuwählen
            var openFileDialog = new Microsoft.Win32.OpenFileDialog
            {
                Filter = "Text files (*.txt, *.csv)|*.txt;*.csv"
            };

            if (openFileDialog.ShowDialog() == true)
            {
                SelectedCommandFileText.Text = openFileDialog.FileName;
            }
        }

        private CancellationTokenSource? ctsCommand;

        private async void SendCommandFileButton_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(SelectedCommandFileText.Text) || !File.Exists(SelectedCommandFileText.Text))
            {
                MessageBox.Show("Please select a valid command file first.");
                return;
            }

            if (SerialPortComboBox.SelectedItem == null)
            {
                MessageBox.Show("Please select a COM port.");
                return;
            }

            cancellationTokenSource = new CancellationTokenSource();
            var token = cancellationTokenSource.Token;
            bool repeat = RepeatCheckbox.IsChecked == true;

            ShowUploadWindow();

            try
            {
                await CommandSimulator.SendCommandsFromFileAsync(
                    SelectedCommandFileText.Text,
                    SerialPortComboBox.SelectedItem.ToString(),
                    repeat,
                    token,
                    uploadWindow!);

                uploadWindow!.Dispatcher.Invoke(() =>
                {
                    uploadWindow!.AppendLine("\nDone.");
                });
            }
            catch (OperationCanceledException)
            {
                uploadWindow!.Dispatcher.Invoke(() =>
                {
                    uploadWindow!.AppendLine("\nCanceled by user.");
                });
            }
            catch (Exception ex)
            {
                uploadWindow!.Dispatcher.Invoke(() =>
                {
                    uploadWindow!.AppendLine($"\nError: {ex.Message}");
                });
            }
        }

        private void CancelCommandFileButton_Click(object sender, RoutedEventArgs e)
        {
            if (cancellationTokenSource != null && !cancellationTokenSource.IsCancellationRequested)
            {
                cancellationTokenSource.Cancel();
            }
        }

        private async void DumpEEPROM_Button_Click(object sender, RoutedEventArgs e)
        {
            if (ArduinoBoardComboBox.SelectedItem is not ArduinoBoard selectedBoard)
            {
                MessageBox.Show("Please choose a board.");
                return;
            }

            if (SerialPortComboBox.SelectedItem is not string selectedPort)
            {
                MessageBox.Show("Please choose a COM-Port.");
                return;
            }

            // search in JSON for Dump filename
            string dumpFileName = selectedBoard.EEPROMdump;
            if (string.IsNullOrEmpty(dumpFileName))
            {
                MessageBox.Show("This board has no EEPROM dump file defined.");
                return;
            }

            // Dump-Hexfile is under Data/DumpEEPROM/
            string baseDir = AppContext.BaseDirectory;
            string dumpFilePath = Path.Combine(baseDir, "Data", "DumpEEPROM", dumpFileName);

            if (!File.Exists(dumpFilePath))
            {
                MessageBox.Show($"EEPROM dump file not found:\n{dumpFilePath}");
                return;
            }

            ShowUploadWindow();
            uploadWindow!.AppendLine("Starting EEPROM dump upload...");

            // Flash the hex file
            await FirmwareUploader.StartUpload(dumpFilePath, selectedBoard, selectedPort, uploadWindow!);

            uploadWindow!.AppendLine("---------------------------------------------------");
            uploadWindow!.AppendLine("EEPROM dump upload finished.");
            uploadWindow!.AppendLine("Connecting to board to capture dump output...");
            uploadWindow!.AppendLine("EEPROM content is: ");
            uploadWindow!.AppendLine("");

            // connect via serial connection and write output in DumpEEPROM.txt
            try
            {
                uploadWindow!.AppendLine("Connecting to board to capture dump output...");
                uploadWindow!.AppendLine("EEPROM content is: ");
                uploadWindow!.AppendLine("");
                using var sp = new SerialPort(selectedPort, 115200, Parity.None, 8, StopBits.One)
                {
                    ReadTimeout = 2000,
                    WriteTimeout = 1000
                };
                sp.Open();
                sp.DtrEnable = true;
                Thread.Sleep(2000);

                string logFile = Path.Combine(baseDir, "DumpEEPROM.txt");
                using var writer = new StreamWriter(logFile, false, Encoding.UTF8);

                while (true)
                {
                    string line = sp.ReadLine();
                    uploadWindow!.AppendLine(line);
                    writer.WriteLine(line);
                    writer.Flush();
                }
            }
            catch (TimeoutException)
            {
                uploadWindow!.AppendLine("---------------------------------------------------");
                uploadWindow!.AppendLine("\n\rEEPROM dump completed. Output written to DumpEEPROM.txt");
                uploadWindow!.AppendLine("\n\rYou can close this window now.");
            }
            catch (Exception ex)
            {
                uploadWindow!.AppendLine($"Error during EEPROM dump: {ex.Message}");
            }
        }


        private async void ResetEEPROM_Button_Click(object sender, RoutedEventArgs e)
        {
            if (ArduinoBoardComboBox.SelectedItem is not ArduinoBoard selectedBoard)
            {
                MessageBox.Show("Please choose a board first.");
                return;
            }

            if (SerialPortComboBox.SelectedItem is not string selectedPort)
            {
                MessageBox.Show("Please choose a COM-Port.");
                return;
            }

            string resetFileName = selectedBoard.EEPROMclear;
            if (string.IsNullOrEmpty(resetFileName))
            {
                MessageBox.Show("This board has no EEPROM clear file defined.");
                return;
            }

            string baseDir = AppContext.BaseDirectory;
            string resetFilePath = Path.Combine(baseDir, "Data", "ClearEEPROM", resetFileName);

            if (!File.Exists(resetFilePath))
            {
                MessageBox.Show($"EEPROM clear file not found:\n{resetFilePath}");
                return;
            }

            ShowUploadWindow();
            uploadWindow!.AppendLine("Starting EEPROM clear...");

            try
            {
                await FirmwareUploader.StartUpload(
                    resetFilePath,
                    selectedBoard,
                    selectedPort,
                    uploadWindow!
                );

                uploadWindow!.AppendLine("\n\rEEPROM clear finished.");
                uploadWindow!.AppendLine("\n\rYou can now close this window.");
            }
            catch (Exception ex)
            {
                uploadWindow!.AppendLine($"Error during EEPROM clear: {ex.Message}");
            }
        }
    }
}
