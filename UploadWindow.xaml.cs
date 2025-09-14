using System.IO;
using System.Windows;
using System.Windows.Shapes;

namespace MobiDude_V2
{
    public partial class UploadWindow : Window
    {
        StreamWriter _writer;
        public UploadWindow(StreamWriter writer)
        {
            InitializeComponent();
            _writer = writer;
        }

        public void AppendLine(string line, bool print = true)
        {
            Dispatcher.Invoke(() =>
            {
                if (print)
                {
                    OutputTextBox.AppendText(line + "\n");
                    OutputTextBox.ScrollToEnd();
                }
                _writer.WriteLine(line);
                _writer.Flush();
            });
        }

        public void AppendText(string text, bool print = true)
        {
            Dispatcher.Invoke(() =>
            {
                if (print)
                {
                    OutputTextBox.AppendText(text);
                    OutputTextBox.ScrollToEnd();
                }
                _writer.WriteLine(text);
                _writer.Flush();
            });
        }
    }
}
