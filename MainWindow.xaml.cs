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

        public MainWindow()
        {
            InitializeComponent();
            StartUsbDeviceWatcher();
            LoadBoardList();
            RefreshSerialPorts();
        }

        private void ExitButton_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
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

            var uploadWindow = new UploadWindow();
            await FirmwareUploader.StartUpload(selectedFilePath, selectedBoard, selectedPort, uploadWindow);
        }

    }

}
