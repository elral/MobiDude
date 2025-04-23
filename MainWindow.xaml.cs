using System;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Text.Json;
using System.Threading;
using System.Windows;
using Microsoft.Win32;

namespace MobiDude_V2
{
    public partial class MainWindow : Window
    {
        private List<ArduinoBoard> boardList = new();

        public MainWindow()
        {
            InitializeComponent();
            LoadBoardList();
            RefreshSerialPorts();
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
            SerialPortComboBox.ItemsSource = SerialPort.GetPortNames();
            SerialPortComboBox.SelectedIndex = 0;
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

        private void RefreshComPortListButton_Click(object sender, RoutedEventArgs e)
        {
            var comPorts = SerialPort.GetPortNames();
            SerialPortComboBox.ItemsSource = comPorts;
            if (comPorts.Length > 0 && SerialPortComboBox.SelectedItem == null)
            {
                SerialPortComboBox.SelectedIndex = 0;
            }
        }

        private string secondFilePath;

        private void OpenSecondFileButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFileDialog();
            if (dialog.ShowDialog() == true)
            {
                secondFilePath = dialog.FileName;
                SecondSelectedFileText.Text = Path.GetFileName(secondFilePath);
            }
        }

        private void SendButton_Click(object sender, RoutedEventArgs e)
        {
            if (!string.IsNullOrEmpty(secondFilePath))
            {
                // TODO: Auslagern in eigene Klasse
                // Beispiel: SendFileHandler.Send(secondFilePath);
                MessageBox.Show($"Sende: {secondFilePath}");
            }
            else
            {
                MessageBox.Show("Bitte zuerst eine Datei auswählen.");
            }
        }

    }

}
